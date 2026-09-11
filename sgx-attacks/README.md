# DDRop attacks on Intel Scalable SGX

This folder contains the DDRop proof-of-concept implementations for Intel Scalable SGX.

## isgx driver patches

The driver patch in `./patches` implement support for custom memory allocation.
See [here](./patches/README.md) for more instructions.

## Running the attack

The attack can be found in the following subfolder:
 * [Dropping victim writes](./drop-write/README.md)
