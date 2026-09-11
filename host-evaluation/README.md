# DDRop Primitives Evaluation on Host

This folder contains the programs to evaluate the interposer's functionality.
They should be used in combination with the [script](../rdimm-injection-controller-firmware/scripts/) to control the interposer.

## Physical Memory Access

This evaluation relies on BadRAM's [read_alias](https://github.com/badramattack/badram/tree/main/alias-reversing/modules/read_alias) kernel module, which provides direct, uncached access to physical addresses.
By default, the Makefiles point to the included `badram` submodule (`../../badram`). If you place BadRAM in a custom directory, modify `LIBCOMMON` and `LIBKRA` in the Makefiles or pass them as environment variables.

## Evaluations

### `drop-write` - Dropping Writes

This binary will allocate a buffer at the given location.
It will then ask to enable the interposer, after which it will update the value at that location.
After disabling the interposer, the binary will read back the result, and print it to the console.

This binary can be used to test the CS disconnection and parity primitives.

### `drop-read` - Dropping Reads

This binary will function similar to above, but instead of writing while the interposer is active, it will read from the target location.

### `swap-ranks` - Detecting Rank Aliasing

This binary functions similar to `drop-write`, but instead of writing a fixed pattern to the target location, it writes the cache line's address.
After disabling the interposer again, it iterates back over the page, and prints any discrepancies in the read-back address.

This binary can be used to test the CS swapping primitive.

## Building and Running

The binaries can be built using `make`, and run with `sudo`.
They all take a single command-line argument: the target HPA.
Follow the instructions printed to the console to enable and disable the interposer.
