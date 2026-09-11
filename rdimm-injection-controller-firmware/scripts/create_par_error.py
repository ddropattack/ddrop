from interposer import *
from time import sleep

print("Disconnecting ALERTn")
set_CA_switch(Pin.PIN_ALERT_DISC, CAState.DISCONNECT, CAValue.HIGH)

sleep(1)

print("Connecting CA2 to GND")
set_CA_switch(Pin.PIN_CA2_DISC, CAState.DISCONNECT, CAValue.LOW)

input("Press Enter to continue...")

set_CA_switch(Pin.PIN_CA2_DISC, CAState.PASSTHROUGH, CAValue.LOW)
print("CA 2 restored")

sleep(1)

set_CA_switch(Pin.PIN_ALERT_DISC, CAState.PASSTHROUGH, CAValue.HIGH)
print("ALERTn restored")
