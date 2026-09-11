#ifndef MOVEPAGE_H
#define MOVEPAGE_H

#include <linux/kvm_types.h>
#include <linux/kvm_host.h>
#include <linux/bits.h>

#include <asm/svm.h>
#include <asm/sev-common.h>
#include <linux/kvm_host.h>


struct kvm_sev_info {
	bool active;		/* SEV enabled guest */
	bool es_active;		/* SEV-ES enabled guest */
	bool need_init;		/* waiting for SEV_INIT2 */
	unsigned int asid;	/* ASID used for this guest */
	unsigned int handle;	/* SEV firmware handle */
	int fd;			/* SEV device fd */
	unsigned long policy;
	unsigned long pages_locked; /* Number of pages locked */
	struct list_head regions_list;  /* List of registered regions */
	u64 ap_jump_table;	/* SEV-ES AP Jump Table address */
	u64 vmsa_features;
	u16 ghcb_version;	/* Highest guest GHCB protocol version allowed */
	struct kvm *enc_context_owner; /* Owner of copied encryption context */
	struct list_head mirror_vms; /* List of VMs mirroring */
	struct list_head mirror_entry; /* Use as a list entry of mirrors */
	struct misc_cg *misc_cg; /* For misc cgroup accounting */
	atomic_t migration_in_progress;
	void *snp_context;      /* SNP guest context page */
	void *guest_req_buf;    /* Bounce buffer for SNP Guest Request input */
	void *guest_resp_buf;   /* Bounce buffer for SNP Guest Request output */
	struct mutex guest_req_mutex; /* Must acquire before using bounce buffers */
};

struct kvm_svm {
	struct kvm kvm;

	/* Struct members for AVIC */
	u32 avic_vm_id;
	struct page *avic_logical_id_table_page;
	struct page *avic_physical_id_table_page;
	struct hlist_node hnode;

	struct kvm_sev_info sev_info;
};

struct kvm_vcpu;

static __always_inline struct kvm_svm *to_kvm_svm(struct kvm *kvm)
{
	return container_of(kvm, struct kvm_svm, kvm);
}


#endif // MOVEPAGE_H
