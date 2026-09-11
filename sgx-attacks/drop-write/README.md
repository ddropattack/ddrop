# Simple drop write attack on Scalable SGX

This is a PoC for a drop write attack inside the EPC. It assumes a cooperative
debug scenario where a victim and attack are manually synchronized through a
CLI interface.

A demonstration video is available in [sgx-drop-write.mp4](sgx-drop-write.mp4).

The victim enclave will:
* Initialize its buffer with all zeros.
* Overwrite the buffer with all ones.
* Print the final buffer.

The attacker aims to drop the victim's write, resulting in a buffer of all zeros.

## Build

### Linux SGX driver
First, build and install the modified out-of-tree SGX driver by applying the
patches in `../patches/linux-sgx-driver.patch`. These patches ensure the EPC
pages are allocated at physical addresses that are not affected by the
interposer, thus preventing crashes when enabling the interposer. Additionally,
they provide an interface through which EPC pages can be allocated at
chosen physical addresses. Note that this patch contains hardcoded addresses,
specific to the target system's memory configuration.
Make sure the in-tree SGX driver is disabled by adding `nosgx` to the kernel
command line arguments. (While similar modifications can also be made to the
in-tree driver, we use the legacy SGX driver).

### SGX-Step

Build and install the modified SGX-Step library. We currently only use this
library to translate virtual to physical addresses.
Set `LIBSGXSTEP_DIR` to point to your SGX-Step directory (e.g., `export LIBSGXSTEP_DIR=/path/to/sgx-step`).

### Victim enclave

The victim enclave can be built using `make all` inside `victim-enclave`. Note that the victim enclave
contains hardcoded addresses, which depend on the target system's memory
configuration.

```bash
cd victim-enclave
make clean
make all
```

## Use
  
Launch the victim enclave, which will allocate a buffer. During this step, the physical address of the victim's buffer and its alias pa will be printed.

```
sudo ./victim-enclave/build/binaries/app
```

When prompted, enable the interposer to drop the victim write.


## Example output

If the attack was successful, certain cache lines within the victim buffer should be zero. Note that not all cache lines within the EPC page are affected due to the interleaving between different memory (sub)channels. 

### Victim enclave

```
$ sudo ./victim-enclave/build/binaries/app

--------------------------------------------------------------------------------
[main.c] Creating enclave...
--------------------------------------------------------------------------------

[pt.c] /dev/sgx-step opened!

Enclave buffer allocated at va=0x7463e4e189000 -- pa=0x4001cb3900

Enclave buffer is initialized to all zero
hexdump buffer (64 bytes):
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 

Enable interposer and press enter

Setting enclave buffer to 0xff...

Disable interposer and press enter

Reading back enclave buffer
hexdump buffer (64 bytes):
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 


--------------------------------------------------------------------------------
[main.c] Done.
--------------------------------------------------------------------------------
```
