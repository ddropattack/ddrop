# DDRop attacks on AMD SEV-SNP

This folder contains the DDRop proof-of-concept implementations for AMD SEV-SNP.
All experiments were performed with the official AMD SEV-SNP kernel branch `snp-host-latest` at commit `68799c0277b24ae63dc6e2b40d6bacd2e2d9a3bb`.

## Support for SNP_PAGE_MOVE

The kernel patch in `./snp_page_move` implements support for `SNP_PAGE_MOVE`.
See [here](./snp_page_move/README.md) for more instructions on how to build the custom kernel and the corresponding kernel module.

## Running the attacks

The different attacks can be found in the following subfolders:
 * [Dropping victim writes](./drop-write/README.md)
 * [Exploiting SNP_PAGE_MOVE](./page-move/README.md)