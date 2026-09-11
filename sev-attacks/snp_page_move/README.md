# SNP_PAGE_MOVE patches and kernel module

This is a small patch that adds support for the `SNP_PAGE_MOVE` command.

Tested with the official AMD SEV-SNP kernel branch `snp-host-latest` at commit `68799c0277b24ae63dc6e2b40d6bacd2e2d9a3bb`.

To expose this command to userspace, we rely on a small kernel module in `./movepage`.
This kernel module is adapted from [Relocate+Vote](https://zenodo.org/records/16351848).

## Build
This assumes that your kernel sources are already configured etc.
This is e.g., the case if you follow the manual in the AMD repo to build the kernel.

1) Apply the patch
2) Copy the buildscript to the kernel source directory. Run it with `./rebuild-kvm.sh `.
3) Reload the KVM and CCP modules with `sudo modprobe -r kvm_amd kvm ccp && sudo modprobe kvm_amd`
4) Build the kernel module and userspace program with `make`
5) Insert the movepage kernel module with `sudo insmod kmod_movepage.ko`

## Use

Use the build binary `movepage` to move SEV pages to a new HPA:
`sudo ./movepage <PID> <VICTIM GPA> <NEW HPA>`
