# Injection Controller Firmware
This is the main firmware repository for the Teensy 4.1 microcontroller board,
used as the fault injection controller.

## Dependencies/Toolchain
The project uses code and compilers from the [Arduino](arduino.cc) project.

1. Download and install the [Arduino IDE](https://www.arduino.cc/en/software/)
2. Open the IDE, which should automatically download additional resources.
3. Go to `File -> Preferences` and enter `https://www.pjrc.com/teensy/package_teensy_index.json` to the "Additional Board Manager URLs",
as described by the [Teensy website](https://www.pjrc.com/teensy/td_download.html).
4. Download the Teensy resources (compilers etc.) when prompted.
5. Now everything should be installed and you can close the IDE.

The makefile expects the necessary compilers to be installed in the default 
`$HOME/.arduino15` location. You can override it by setting `ARDUINOPATH=`.

Drivers and libraries from Arduino were copied to this repo using
```
rm -rf cores
mkdir -p cores/libraries
cp -r ~/.arduino15/packages/teensy/hardware/avr/*/libraries/{FNET,NativeEthernet,SPI,TeensyThreads} cores/libraries/
cp -r ~/.arduino15/packages/teensy/hardware/avr/*/cores/teensy4/* cores/
```
It is theoretically possible to use your system's
`arm-none-eabi` toolchain and Teensy-loader utilities without having to install 
the IDE. This is untested, however.

Tested with Arduino IDE `v2.3.6`, Teensy toolchain `v5.4.1` and tools `v1.56.2` (board package `v1.56.2`).

## Build
To initiate the build process, run
```
make -j
```

> [!NOTE]
> Build with `FLAGS=-DDISABLE_ETHERNET` when not connecting the Ethernet port.

## Upload
To program the firmware into the microcontroller, run
```
teensy_loader_cli -w --mcu=TEENSY41 build/main.hex
```

## Interfacing with the Controller

You can interface with and control the interposer via USB using the Python scripts in [`scripts/`](./scripts/):

First, ensure the required Python packages are installed:
```bash
pip install -r scripts/requirements.txt
```

The directory provides three scripts to trigger the different DDRop primitives:
* [`scripts/create_par_error.py`](./scripts/create_par_error.py): Disconnects `ALERT_n` and drives `CA2` to GND to inject parity errors on the DDR5 command/address bus, silently dropping commands.
* [`scripts/swap_ranks.py`](./scripts/swap_ranks.py): Swaps the `CS0` and `CS1` lines to test rank aliasing.
* [`scripts/disconnect_CS.py`](./scripts/disconnect_CS.py): Disconnects `CS0` to drop writes or reads to Rank 0. Note: requires slight modifications to the interposer.

Each script prompts to press <kbd>Enter</kbd> to restore the original bus signals once the fault injection window concludes.
