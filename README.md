# DDRop: Active Memory Interposer Attacks on Confidential VMs by Dropping DDR5 Writes

This repository contains the hardware designs, firmware, host evaluation tools, and attack proof-of-concepts for [**DDRop**](https://ddropattack.eu/ddrop.pdf).

* **Authors:** Jesse De Meulemeester (KU Leuven), Stefan Gloor (ETH Zurich), Patrick Jattke (ETH Zurich), Daniel Moghimi (Google), David Oswald (Durham University), Martin Thompson (Durham University & ZF Automotive UK Ltd), Kaveh Razavi (ETH Zurich), Ingrid Verbauwhede (KU Leuven), and Jo Van Bulck (KU Leuven)

---

## Overview

Modern Trusted Execution Environments (TEEs) such as Intel TDX, Intel Scalable SGX, and AMD SEV-SNP rely on memory encryption to protect enclaves and confidential VMs from an untrusted hypervisor or cloud operator. However, to maintain memory performance, scalable cloud TEEs omit cryptographic freshness guarantees.

DDRop demonstrates active physical interposition attacks on DDR5 at native bus speeds using a custom low-cost DDR5 RDIMM interposer. By injecting parity errors on DDR5 command/address lines, the interposer silently discards cache line writebacks (dropping writes) or swaps chip selects. This repository contains the full materials to inspect, build, and evaluate the interposer hardware, controller firmware, host primitives, and confidential VM attacks.

---

## Repository Structure & Paper Mapping

The contents of this repository correspond directly to the sections of the paper:

| Directory | Paper Section | Description |
|:---|:---|:---|
| [`rdimm-interposer/`](./rdimm-interposer/) | §4.1 | DDR5 RDIMM interposer PCB designs |
| [`rdimm-injection-controller/`](./rdimm-injection-controller/) | §4.1 | Microcontroller injection board designs |
| [`rdimm-injection-controller-firmware/`](./rdimm-injection-controller-firmware/) | §4.1 | Teensy 4.1 firmware & Python control scripts |
| [`host-evaluation/`](./host-evaluation/) | §4.2 | Bare-metal host evaluation of DDRop primitives | 
| [`sev-attacks/`](./sev-attacks/) | §5.1, §5.2.1 | Attacks on AMD SEV-SNP |
| [`sgx-attacks/`](./sgx-attacks/) | §5.1 | Attacks on Intel Scalable SGX | 
| [`tdx-attacks/`](./tdx-attacks/) | §5.1, §5.2.1, §5.2.2, §6 | Attacks on Intel TDX | 

Demonstration videos recorded during physical attack execution are available in the respective directories:
* AMD SEV-SNP write drop: [`sev-attacks/drop-write/sev-drop-write.png`](./sev-attacks/drop-write/sev-drop-write.png)
* AMD SEV-SNP `SNP_PAGE_MOVE` exploit: [`sev-attacks/page-move/sev-page-move.webm`](./sev-attacks/page-move/sev-page-move.webm)
* Intel Scalable SGX write drop: [`sgx-attacks/drop-write/sgx-drop-write.mp4`](./sgx-attacks/drop-write/sgx-drop-write.mp4)
* Intel TDX page relocation: [`tdx-attacks/page-relocate/tdx-page-relocate.webm`](./tdx-attacks/page-relocate/tdx-page-relocate.webm)
* Intel TDX debug bit flip: [`tdx-attacks/sept-injection/tdcs-poc.mp4`](./tdx-attacks/sept-injection/tdcs-poc.mp4)

---

## Getting Started

### Hardware and Software Requirements

#### Software & Build Environment
* Linux x86_64 system
* GCC (supporting C11 / C++17) and Make
* Python 3.8+ with `requirements.txt` packages installed
* [KiCad](https://www.kicad.org/) (v9.0 or later) to view PCB schematics and layouts
* [Arduino IDE](https://www.arduino.cc/) or `teensy_loader_cli` with Teensyduino support (for compiling and flashing the controller firmware)

#### Physical Experiment Requirements
End-to-end physical replication of the attacks requires:
1. The manufactured **DDR5 RDIMM Interposer** and **Injection Controller** boards (design files in `rdimm-interposer/` and `rdimm-injection-controller/`).
2. A **Teensy 4.1** microcontroller board connected to the injection controller.
3. Server hardware with DDR5 RDIMM slots supporting the target TEE:
   * **Intel TDX**: 5th/6th Gen Intel Xeon Scalable processor with TDX enabled.
   * **AMD SEV-SNP**: 4th/5th/6th Gen AMD EPYC processor with SEV-SNP enabled.
   * **Intel Scalable SGX**: 4th/5th/6th Gen Intel Xeon Scalable processor with SGX enabled.

When physical interposer hardware is not available, the software components and attack binaries can still be compiled locally, and their execution flow can be cross-referenced with the provided screencasts.

### Building the Artifacts

You can verify and build all host evaluation tools and attack PoCs from the repository root:

```bash
make all
```

This command automatically builds the dependencies and produces:
* **Host evaluation primitives** (`host-evaluation/drop-write`, `swap-ranks`, `drop-read`)
* **AMD SEV-SNP attack binaries** (`sev-attacks/drop-write`, `page-move`, `movepage`)
* **Intel TDX attack binaries** (`tdx-attacks/drop-write`, `page-relocate`, `attacker-td`, `victim-td`)

To remove all built binaries and object files:
```bash
make clean
```

> [!NOTE]
> Building Intel Scalable SGX enclaves (`sgx-attacks/`) requires the Intel SGX SDK and SGX-Step with `LIBSGXSTEP_DIR` set (see [`sgx-attacks/README.md`](sgx-attacks/README.md)). The microcontroller firmware can be compiled with `make firmware` (see [`rdimm-injection-controller-firmware/README.md`](rdimm-injection-controller-firmware/README.md)).

---

## Detailed Instructions

Refer to the individual README files in each folder for specific execution steps, kernel patch instructions, and command-line parameters:
* [`rdimm-interposer/README.md`](rdimm-interposer/README.md): Interposer specifications, stackup, and modification instructions.
* [`rdimm-injection-controller/README.md`](rdimm-injection-controller/README.md): Controller board hardware and assembly.
* [`rdimm-injection-controller-firmware/README.md`](rdimm-injection-controller-firmware/README.md): Firmware flashing and control API.
* [`host-evaluation/README.md`](host-evaluation/README.md): Running host-level write drop, read drop, and rank swapping primitives.
* [`sev-attacks/README.md`](sev-attacks/README.md): AMD SEV-SNP setup and attack steps.
* [`sgx-attacks/README.md`](sgx-attacks/README.md): Intel Scalable SGX driver patches and enclave evaluation.
* [`tdx-attacks/README.md`](tdx-attacks/README.md): Intel TDX kernel/TDXplore patches, page relocation, and SEPT injection attacks.

---

## Citation

If you reference or build upon this work, please cite the paper:

```bibtex
@inproceedings{ddrop26,
  title     = {{DDRop}: Active Memory Interposer Attacks on Confidential {VMs} by Dropping {DDR5} Writes},
  author    = {De Meulemeester, Jesse and Gloor, Stefan and Jattke, Patrick and Moghimi, Daniel and Oswald, David and Thompson, Martin and Razavi, Kaveh and Verbauwhede, Ingrid and Van Bulck, Jo},
  booktitle = {Proceedings of the 2026 {ACM} {SIGSAC} Conference on Computer and Communications Security ({CCS} '26)},
  publisher = {ACM},
  year      = 2026,
}
```
