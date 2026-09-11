# Kernel and TDXplore patches for TDX attacks

This folder contains the patches to enable `TDH.MEM.PAGE.RELOCATE` in the kernel, and support this and related SEAMCALLs in the TDXplore framework.

## Linux Kernel Patch (`linux-intel-6.8.0.patch`)

Applies to the Intel TDX host kernel (`linux-intel-6.8.0`).
This patch enables TDX memory on the second socket while reserved through `memmap`, and disables hugepages for `guest_memfd` (since `TDH.MEM.PAGE.RELOCATE` operates on 4KB pages).

To apply:
```bash
cd /path/to/linux-intel-6.8.0
patch -p1 < /path/to/linux-intel-6.8.0.patch
```

## TDXplore Patch (`tdxplore.patch`)

Applies to [TDXplore](https://github.com/google/security-research/tree/master/pocs/cpus/tdxplore) at commit `47c71cd9146606fce459d7af0d5ef0ad427a3699`.
It adds support for `TDH.MEM.PAGE.RELOCATE` and related SEAMCALLs.

To apply:
```bash
cd /path/to/tdxplore
git checkout 47c71cd9146606fce459d7af0d5ef0ad427a3699
git apply /path/to/tdxplore.patch
```
