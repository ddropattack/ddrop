.PHONY: all host-eval sev-attacks tdx-attacks sgx-attacks firmware clean help

all: host-eval sev-attacks tdx-attacks

host-eval:
	@echo "=== Building host-evaluation binaries ==="
	$(MAKE) -C host-evaluation/drop-write
	$(MAKE) -C host-evaluation/swap-ranks
	$(MAKE) -C host-evaluation/drop-read

sev-attacks:
	@echo "=== Building SEV-SNP attack binaries ==="
	$(MAKE) -C sev-attacks/drop-write
	$(MAKE) -C sev-attacks/page-move
	$(MAKE) -C sev-attacks/snp_page_move/movepage movepage

tdx-attacks:
	@echo "=== Building TDX attack binaries ==="
	$(MAKE) -C tdx-attacks/drop-write
	$(MAKE) -C tdx-attacks/page-relocate
	$(MAKE) -C tdx-attacks/sept-injection/attacker-td
	$(MAKE) -C tdx-attacks/sept-injection/victim-td

sgx-attacks:
ifdef LIBSGXSTEP_DIR
	@echo "=== Building SGX attack binaries ==="
	$(MAKE) -C sgx-attacks/drop-write/victim-enclave
else
	@echo "Notice: LIBSGXSTEP_DIR is not set. To build the SGX victim enclave, set LIBSGXSTEP_DIR and install Intel SGX SDK (see sgx-attacks/README.md)."
endif

firmware:
	@echo "=== Building controller firmware ==="
	$(MAKE) -C rdimm-injection-controller-firmware

clean:
	-$(MAKE) -C host-evaluation/drop-write clean
	-$(MAKE) -C host-evaluation/swap-ranks clean
	-$(MAKE) -C host-evaluation/drop-read clean
	-$(MAKE) -C sev-attacks/drop-write clean
	-$(MAKE) -C sev-attacks/page-move clean
	-$(MAKE) -C sev-attacks/snp_page_move/movepage clean
	-$(MAKE) -C tdx-attacks/drop-write clean
	-$(MAKE) -C tdx-attacks/page-relocate clean
	-$(MAKE) -C tdx-attacks/sept-injection/attacker-td clean
	-$(MAKE) -C tdx-attacks/sept-injection/victim-td clean
	-$(MAKE) -C rdimm-injection-controller-firmware clean

