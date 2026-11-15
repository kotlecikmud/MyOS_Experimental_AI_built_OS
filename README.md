# MyOS – Custom Operating System (Milestones 1 to 7)

## Overview
MyOS is a custom, minimal, independent operating system developed entirely from scratch. It features its own freestanding kernel, hardware drivers, memory management, exception handling, virtual filesystem (VFS), security/authentication subsystem, and interactive command-line shell. It is not an application running on Linux, nor is it a Linux distribution.

---

## Architectural Milestones Completed

1. **Milestone 1: Project Foundation & Bootable Kernel**
   - Modern **Limine Boot Protocol** (Base Revision 3) with higher-half kernel mapping (`0xffffffff80000000`).
   - Serial console driver (COM1 at 38,400 baud) for headless output.
   - Global Descriptor Table (GDT) and Interrupt Descriptor Table (IDT) configuration.
   - Physical Memory Manager (PMM) parsing Limine memory map.

2. **Milestone 2 & 3: Multi-Architecture Support & Hardware Drivers**
   - Architectures supported: x86-64 (PC/QEMU), ARMv6 (Raspberry Pi Zero W), AArch64 (Raspberry Pi Zero 2 W).
   - UART / serial drivers across architectures.

3. **Milestone 4: Persistent Storage & Virtual Filesystem (VFS)**
   - In-memory VFS with directory and file management (`/`, `/home`, `/home/admin`, `/etc`, `/bin`, `/tmp`).
   - Robust path resolution supporting absolute paths, relative paths, parent navigation (`..`), and root navigation (`/`).

4. **Milestone 5: Interactive Command-Line Shell**
   - Console I/O abstraction.
   - Interactive shell with prompt (`MyOS:/home/admin> `).
   - Supported commands:
     - `list`: Enumerates contents of the current working directory.
     - `cd <path>`: Changes the current working directory.
     - `help`: Displays available commands.

5. **Milestone 6: Administrator Account & Salted Authentication**
   - First-boot detection: Prompts the user to create an administrator account on first boot if no credentials exist.
   - Salted secure hash: Derives an iterative cryptographically secure salted hash for the administrator password.
   - Subsequent boots: Enforces login authentication (username & password) before granting access to the shell.

6. **Milestone 7: Integration, Robust Boot Flow & Release**
   - Complete integrated boot sequence from hardware boot to authenticated shell environment.

---

## Project Structure
```text
MyOS/
├── arch/
│   ├── aarch64/          # AArch64 (Raspberry Pi Zero 2 W)
│   └── armv6/            # ARMv6 (Raspberry Pi Zero W)
├── kernel/
│   ├── include/          # kernel.h, serial.h, gdt.h, idt.h, memory.h, pmm.h, console.h, vfs.h, shell.h, auth.h
│   └── src/              # kernel.c, serial.c, gdt.c, idt.c, memory.c, pmm.c, log.c, console.c, vfs.c, shell.c, auth.c
├── limine.conf           # Limine 12.x bootloader configuration
├── linker.ld             # x86-64 linker script
├── limine-binary.tar.gz  # Limine 12.x bootloader binaries
├── Makefile              # Multi-architecture build system
└── README.md
```

---

## Build & Run Instructions

### Prerequisites
- GCC and GNU binutils for the native x86-64 target
- `xorriso` (for ISO image generation)
- `Limine` bootloader files
- QEMU (`qemu-system-x86_64`) for testing

### Building and Running (Ubuntu / WSL2)
Install the host build dependencies if they are not already present:

```bash
sudo apt update
sudo apt install build-essential xorriso qemu-system-x86 ovmf
```

Build the x86-64 kernel and hybrid BIOS/UEFI ISO:

```bash
make ARCH=x86_64
make iso ARCH=x86_64
```

Boot the ISO in QEMU using BIOS firmware and view kernel serial output:

```bash
make run ARCH=x86_64
```

The ISO includes the Limine 12.x BIOS boot image, UEFI boot image, `BOOTX64.EFI`,
`limine-bios.sys`, and `limine.conf`. The Makefile runs `limine bios-install`
after creating the hybrid ISO. UEFI boot is included in the ISO; testing it in
QEMU additionally requires an OVMF firmware image (paths vary by distribution).

The ARMv6 and AArch64 build targets remain separate and continue to use their
respective `arm-none-eabi-*` and `aarch64-none-elf-*` cross-compilers. The x86-64
native compiler change does not provide those cross-compilers.
