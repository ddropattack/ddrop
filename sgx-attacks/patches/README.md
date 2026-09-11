# Patches for the Scalable SGX attacks

## Linux SGX driver patch

This patch modifies the EPC allocation in the legacy SGX driver to ensure all
pages are allocated in regions that are not affected by the interposer, allowing
enclaves to run with the interposer active. Secondly, it adds an interface to
request a certain EPC page at a given physical address. This is used, for
instance, to allocate an attacker EPC page that aliases with a victim EPC page.
This patch is based on the [driver patches for Battering RAM](https://github.com/batteringramattack/batteringram/tree/main/scalable-sgx-attacks/patches).

## SGX-Step patch

A small patch that enables the sgx_step device to be opened multiple times.
