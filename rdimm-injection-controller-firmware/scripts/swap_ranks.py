from interposer import *

print("Swapping CS lines")
set_CS_switch(CSState.SWAPPED)

input("Press Enter to continue...")

set_CS_switch(CSState.PASSTHROUGH)
print("CS lines restored")
