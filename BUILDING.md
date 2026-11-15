# Building MyOS (x86-64 / Limine 12.x)

This project builds the x86-64 kernel with the native x86-64 GCC toolchain on
Ubuntu/WSL2. The ARMv6 and AArch64 targets remain separate cross-compiler targets.

## 1. Install host tools

```bash
sudo apt update
sudo apt install build-essential xorriso qemu-system-x86 ovmf
```

## 2. Build the kernel and hybrid BIOS/UEFI ISO

Run these commands from the MyOS project directory:

```bash
make clean
make ARCH=x86_64
make iso ARCH=x86_64
```

The first ISO build extracts the included `limine-binary.tar.gz`, builds the
Limine host utility, stages the BIOS and UEFI boot files, creates a hybrid ISO,
and runs `limine bios-install` on that ISO.

Expected output image:

```text
build/x86_64/myos.iso
```

## 3. Test BIOS boot in QEMU

```bash
make run ARCH=x86_64
```

Kernel diagnostics are sent to the QEMU serial console. On first boot, the
MyOS authentication subsystem asks you to create an administrator account.

## 4. UEFI boot

The hybrid ISO includes `EFI/BOOT/BOOTX64.EFI` and the Limine UEFI CD image.
To test UEFI in QEMU, use an OVMF firmware image installed by the `ovmf` package.
The exact firmware file paths can vary by Ubuntu package version; common paths
are under `/usr/share/OVMF/`.

## Notes

- Limine's configuration is `limine.conf` and uses the Limine 12.x syntax.
- The x86-64 kernel requests Limine base revision 3, the memory map, and the
  Higher Half Direct Map (HHDM).
- The x86-64 PMM bitmap is placed in a usable physical memory range and accessed
  through the HHDM. `pmm_alloc_block()` returns a physical address; map it before
  dereferencing it from the higher-half kernel.
- The x86-64 build was compiled and linked successfully with GCC 11.4 / GNU ld
  2.38. ISO creation and QEMU boot still need to be run on a host with xorriso
  and QEMU available.
- ARMv6 and AArch64 source files and compiler selections were retained. Those
  targets require their respective cross-compilers and were not built as part
  of this x86-64 validation.
