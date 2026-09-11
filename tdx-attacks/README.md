# DDRop attacks on Intel TDX

This folder contains the DDRop proof-of-concept implementations for Intel TDX.

## Kernel patches

We provide a small kernel patch to enable TDX to use memory mapped to the second socket while it is reserved through `memmap` in `./patches`.

## TDXplore patches

These attacks rely on a modified version of [TDXplore](https://github.com/google/security-research/tree/master/pocs/cpus/tdxplore).
The patch file can be found [here](./patches/tdxplore.patch).
Tested with the latest version of TDXplore (commit `47c71cd9146606fce459d7af0d5ef0ad427a3699`).

## Running the attacks

The different attacks can be found in the following subfolders:
 * [Dropping victim writes](./drop-write/README.md)
 * [Exploiting TDH.MEM.PAGE.RELOCATE](./page-relocate/README.md)
 * [Injecting SEPT entries & Case Studies](./sept-injection/README.md)