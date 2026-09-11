# Injecting SEPT Entries

This attack injects attacker-controlled entries into an attacker TD's SEPT tree.
Using this primitive, we show how an attacker can capture and replay victim ciphertext, flip the debug flag, and spoof MRTD.

## Running

### Accessing Victim Ciphertext & Changing MRTD

The script `setup-sept-pages.py` performs the operations for injecting an attacker SEPT entry pointing at another SEPT entry, enabling the attacker to overwrite its own SEPT entries, and thus access any HPA.
Using this primitive, the attacker can capture and replay victim ciphertext, and write to its own TDCS, e.g., modifying the MRTD value.
1. Set up TDXplore by inserting the kernel module and building the library.
2. Build and copy `./attacker-td/setup-sept-mapping.c` to the attacker TD.
3. Run `setup-sept-mapping` inside the attacker TD and follow the instructions printed to the console.
4. Run `setup-sept-pages.py`, the command-line arguments are: 
   * gpa: Attacker TD GPA which will map to victim TDCS. Must not be mapped yet.
   * init-gpa: Attacker TD GPA of page which will contain malicious SEPT entries. Enter the GPA of the buffer printed by `setup-sept-mapping` inside the attacker TD.
   * safe-hpa: Free HPA that is not affected by the interposer. This will be used for the level 2 and 3 SEPT entries.
   * target-hpa: Free HPA that is affected by the interposer and will hold the malicious SEPT entries.
5. The Python script will set the attacker in debug mode. With this the host can also perform the necessary read/write operations using the debug API. See `read-write-memory.py`. This can for instance be used to capture and replay ciphertext belonging to a victim TD.
6. For modifying the MRTD, build and copy `./attacker-td/change-mrtd.c` and copy the binaries to the attacker TD.

### Flipping Victim Debug Bit

A video demonstrating this attack end-to-end is available in [tdcs-poc.mp4](tdcs-poc.mp4).

The attack on the debug bit can be executed by running `set_debug_flag.py`.
1. Set up TDXplore by inserting the kernel module and building the library.
2. Build and copy `./attacker-td/flip-debug-bit.c` to the attacker TD.
3. Build and copy `./victim-td/ddrop-victim.c` to the victim TD.
4. Run `set_debug_flag.py`. The command-line arguments are similar to above.
5. Follow the instructions in the Python script on the host and binary in the attacker TD. The script will step through setting up the SEPT mappings, flipping the victim's debug flag, and reading victim plaintext.
6. If successful, the attacker should read "Hello World from victim TD!"
