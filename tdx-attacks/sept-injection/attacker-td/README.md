# Attacker TD

## `flip-debug-bit.c` - Flipping Victim TD debug bit
This is the attacker code for the debug attack. This binary will
1. Prepare a page with malicious SEPT entries to be relocated by the host.
   Since the SEPT entries are encrypted under the same memory encryption key as the other TD pages, relocating the pages will leave correctly encrypted SEPT entries at the HPA.
2. Once the host has initiated its attack by dropping writes to the TDH.MEM.SEPT.ADD page, thus injecting the previously crafted SEPT entries, the victim TDCS structure is now mapped into the attacker (this) TD. While it is encrypted under a different key (the victim's key), writing any value to it has a 50% probability of flipping the victim's debug flag.

## `setup-sept-mapping.c` - Injecting SEPT Entry

This code can be used to set up a SEPT mapping such that the attacker can modify its own mappings. The binary will, similar to above, set up a SEPT entry, which now points to another attacker SEPT page. It will also set its own debug flag. This is optional, but makes it easier to carry out the attacker, as the host now has full read/write privileges to the attacker TD, thus can carry out the attacks from the Python scripts.

## `change-mrtd.c` - Changing MRTD

This implements the attacker code to change the build-time measurement register of the TD (MRTD). The code assumes the necessary mapping have already been set up (i.e., by running `setup-sept-mapping`).

### Building

Build the scripts with
```sh
make
```

Then copy them to the attacker TD.

### Running

Run the scripts with `sudo` inside the attacker TD, and follow the instructions in the output.
