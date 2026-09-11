#include <linux/module.h>
#include <linux/cdev.h>   // device_create, ...
#include <linux/version.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/kprobes.h>
#include <linux/rcupdate.h>
#include <linux/xarray.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/psp-sev.h>
#include <linux/psp.h>
#include <asm/sev.h>

#include "movepage.h"
#include "movepage_ioctls.h"

#define NAME "movepage"

static int major = -1;
static struct cdev mycdev;
static struct class *myclass = NULL;

int kvm_update_spte(struct kvm *kvm, gfn_t gfn, hpa_t new_hpa);
u64* (*get_sptep) (struct kvm_vcpu *vcpu, gfn_t gfn, u64 *spte);

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

kallsyms_lookup_name_t kallsyms_lookup_name_func;
#define kallsyms_lookup_name kallsyms_lookup_name_func

static struct kprobe kp = {
  .symbol_name = "kallsyms_lookup_name"
};

struct list_head* _vm_list;

static u64 __maybe_unused rmpupdate_pre_guest(u64 paddr, u32 asid){
  struct rmp_state state = {0};
  u64 ret;

  state.assigned = 1;
  state.pagesize = 0;
  state.immutable = 1;
  state.asid = asid;
  
  do {
    /* Binutils version 2.36 supports the RMPUPDATE mnemonic. */
    asm volatile(".byte 0xF2, 0x0F, 0x01, 0xFE"
           : "=a" (ret)
           : "a" (paddr), "c" ((unsigned long)&state)
           : "memory", "cc");
  } while (ret == RMPUPDATE_FAIL_OVERLAP);

  return ret;
}

static u64 __maybe_unused rmpupdate_pre_swap(u64 paddr, u64 gpa, u32 asid){
    struct rmp_state state = {0};
    u64 ret;

    state.assigned = 1;
    state.pagesize = 0;
    state.immutable = 1;
    state.gpa = gpa;
    state.asid = asid;
    
    do {
        /* Binutils version 2.36 supports the RMPUPDATE mnemonic. */
        asm volatile(".byte 0xF2, 0x0F, 0x01, 0xFE"
                 : "=a" (ret)
                 : "a" (paddr), "c" ((unsigned long)&state)
                 : "memory", "cc");
    } while (ret == RMPUPDATE_FAIL_OVERLAP);

    return ret;
}
 

static u64 hpa_for_gpa(u64 gpa, struct kvm_vcpu* vcpu) {
    u64 spte;
    u64 *sptep;
  rcu_read_lock();
    sptep = get_sptep(vcpu, gpa >> PAGE_SHIFT, &spte);
  rcu_read_unlock();

    return spte & 0x0000FFFFFFFFF000ULL;
}

struct psp_sev_cmd_move {
    u64 guest_physical_addr;
    u32 page_size:1;
    u32 reserved:31;
    u32 reserved1;
    u64 src_paddr;
    u64 dst_paddr;
} __packed;

static int psp_move_page(struct kvm *kvm, u64 hpa_src, u64 gpa_src, u64 hpa_dst){    
  struct kvm_sev_info *sev = &to_kvm_svm(kvm)->sev_info;
  struct psp_sev_cmd_move data = {0};
  u32 error = 0;
  int ret;
  u64 rmpret = 0;

  rmpret = rmpupdate_pre_guest(hpa_dst, sev->asid);
  if (rmpret) {
    pr_info("psp_move_page: Unable to set HPA 0x%llx to PRE_GUEST\n", hpa_dst);
    pr_info("psp_move_page: Error value: 0x%llx\n", rmpret);
    return -1;
  }

  rmpret = rmpupdate_pre_swap(hpa_src, gpa_src, sev->asid);
  if (rmpret) {
    pr_info("psp_move_page: Unable to set 0x%llx to PRE_SWAP\n", hpa_src);
    pr_info("psp_move_page: Error value: 0x%llx\n", rmpret);
    return -1;
  }

  data.guest_physical_addr = __psp_pa(sev->snp_context);
  data.page_size = 0; // 4 KB page
  data.src_paddr = (hpa_src);
  data.dst_paddr = (hpa_dst);

  ret = sev_do_cmd(SEV_CMD_SNP_PAGE_MOVE, &data, &error);

  if (ret != 0) {
    pr_info("page move error value 0x%x\n", error);
    pr_info("page move ret value 0x%x\n",ret);
  }

  return ret;
}

static int move_page(int pid, u64 gpa_src, u64 hpa_dst) {
  struct list_head* i;
  struct kvm_vcpu* vcpu;
  struct kvm* kvm = NULL;
  int found_kvm = 0;
  int ret = 0;

  list_for_each(i, _vm_list) {
    kvm = list_entry(i, struct kvm, vm_list);

    if (kvm->userspace_pid == pid) {
      if (atomic_read(&kvm->online_vcpus) == 0) {
        printk(KERN_INFO
        "no vcpus are online %d\n", atomic_read(&kvm->online_vcpus));
        return -1;
      }
      vcpu = xa_load(&kvm->vcpu_array, 0);
      found_kvm = 1;
      break;
    }
  }
  if (!found_kvm)
  {
    pr_info("kvm object not found\n");
    return -1;
  }


  u64 hpa_victim = hpa_for_gpa(gpa_src, vcpu);

  wbinvd_on_all_cpus();

  // Issuing the SNP_PAGE_MOVE command
  ret = psp_move_page(kvm, hpa_victim, gpa_src, hpa_dst);
  if (ret != 0) {
    pr_info("Unable to move page\n");
    return ret;
  }

  // SNP_PAGE_MOVE only moves the data pages, we also need to update the page tables
  ret = kvm_update_spte(kvm, gpa_src >> PAGE_SHIFT, hpa_dst);
  
  return ret;
}

static int open(struct inode *inode, struct file *file)
{
  (void)inode; (void)file;
  printk("Opened module.\n");
  return 0;
}

static int close(struct inode *inode, struct file *file)
{
  (void)inode; (void)file;
  printk("Closed module.\n");
  return 0;
}

static long ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
  struct args args;
  (void)file;
  
  switch(cmd) {
    case PAGE_MOVE:
      if (copy_from_user(&args, (const void*)arg, sizeof(args))) return -1;
      return move_page(args.pid, args.gpa_src, args.hpa_dst);
    default:
      printk("Unknown cmd=%ud\n", cmd);
      break;
  }
  return 0;
}

static const struct file_operations fops = {
  .open = open,
  .owner = THIS_MODULE,
  .release = close,
  .unlocked_ioctl = ioctl,
};

static void cleanup(int device_created)
{
  if (device_created) {
    device_destroy(myclass, major);
    cdev_del(&mycdev);
  }
  if (myclass) class_destroy(myclass);
  if (major != -1) unregister_chrdev_region(major, 1);
}

static int __init movepage_init(void)
{
  int device_created = 0;

  register_kprobe(&kp);
  kallsyms_lookup_name = (kallsyms_lookup_name_t) kp.addr;
  unregister_kprobe(&kp);

  if (!unlikely(kallsyms_lookup_name)) {
    pr_alert("Could not retrieve kallsyms_lookup_name address\n");
    return -ENXIO;
  }

  _vm_list = (void*)kallsyms_lookup_name("vm_list");
  if (_vm_list == NULL) {
    pr_info("lookup failed vm_list\n");
    return -ENXIO;
  }

  get_sptep = (void*)kallsyms_lookup_name("kvm_tdp_mmu_fast_pf_get_last_sptep");
  if (get_sptep == NULL) {
    pr_info("lookup failed get_sptep\n");
    return -ENXIO;
  }

  /* /proc/devices */
  if (alloc_chrdev_region(&major, 0, 1, NAME "_proc") < 0)
    goto error;
  /* /sys/class */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
  if ((myclass = class_create(NAME "_sys")) == NULL)
    goto error;
#else
  if ((myclass = class_create(THIS_MODULE, NAME "_sys")) == NULL)
    goto error;
#endif
  /* /dev/ */
  if (device_create(myclass, NULL, major, NULL, NAME "_dev") == NULL)
    goto error;
  device_created = 1;
  cdev_init(&mycdev, &fops);
  if (cdev_add(&mycdev, major, 1) == -1)
    goto error;

  printk("Inserted module.\n");

  return 0;

error:
    cleanup(device_created);
    return -1;
}

static void __exit movepage_exit(void)
{
  cleanup(1);
}

module_init(movepage_init);  
module_exit(movepage_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jesse De Meulemeester");
MODULE_DESCRIPTION("Move SEV pages");
