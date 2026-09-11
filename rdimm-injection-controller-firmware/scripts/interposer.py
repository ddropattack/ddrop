import hid
import struct

from enum import Enum

# Teensy
VENDOR_ID = 0x16c0
PRODUCT_ID = 0x0486

class Pin(Enum):
    PIN_LED_ETH_RIGHT = 3
    PIN_LED_STATUS = 2
    PIN_PWR_BTN = 32

    PIN_CS_SWAP = 23
    PIN_ALERT_DISC = 41
    PIN_CA6_DISC = 21
    PIN_CA5_DISC = 19
    PIN_CA4_DISC = 18
    PIN_CA3_DISC = 22
    PIN_CA2_DISC = 39
    PIN_CA1_DISC = 20
    PIN_CA0_DISC = 40
    PIN_CA6_STATIC = 36
    PIN_CA5_STATIC = 17
    PIN_CA3_STATIC = 35
    PIN_CA2_STATIC = 34
    PIN_CA4_STATIC = 15
    PIN_CA1_STATIC = 33
    PIN_CA0_STATIC = 14

class CAState(Enum):
    PASSTHROUGH = 0x11
    DISCONNECT = 0x00

class CAValue(Enum):
    LOW = 0x0
    HIGH = 0x1

class CSState(Enum):
    PASSTHROUGH = 0x00
    SWAPPED = 0x11

def send_interposer_command(command):
    h = hid.device()
    h.open(VENDOR_ID, PRODUCT_ID)
    h.set_nonblocking(1)

    h.write(command)

    h.close()

def toggle_CA_switch(pin: Pin, value: CAValue, fault_duration: int, repeat: int=1):
    """Toggle the CA switch for the given fault duration to the given state"""
    command = [0xAA, 0xBB, pin.value, *list(struct.pack(">H", fault_duration)), value.value, repeat] + [0x00] * (64 - 7)
    send_interposer_command(command)

def set_CA_switch(pin: Pin, state: CAState, value: CAValue):
    """Set the CA switch at the given pin to the given state"""
    command = [0xAA, state.value, pin.value, 0x00, 0x00, value.value] + [0x00] * (64 - 6)
    send_interposer_command(command)

def set_CS_switch(state: CSState):
    """Swap or unswap the CS pins"""
    command = [0xAA, state.value, Pin.PIN_CS_SWAP.value] + [0x00] * (64 - 3)
    send_interposer_command(command)

def reset_teensy():
    """Reset the teensy to enter bootloader mode"""
    command = [0xCC, 0x22] + [0x00] * (64 - 2)
    send_interposer_command(command)
