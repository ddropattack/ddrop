from interposer import *

print("Disconnecting CS0")
set_CS_switch(CSState.SWAPPED)

input("Press Enter to continue...")

set_CS_switch(CSState.PASSTHROUGH)
print("CS0 restored")
