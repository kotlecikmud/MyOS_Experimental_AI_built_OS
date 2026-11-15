# MyOS Makefile supporting multiple architectures (x86-64, armv6, aarch64) with organized build directories

ARCH ?= x86_64

ifeq ($(ARCH),x86_64)
    # Native Ubuntu x86-64 toolchain is sufficient for this freestanding target.
    CC = gcc
    LD = ld
    CFLAGS = -Wall -Wextra -O2 -std=gnu11 -ffreestanding -fno-stack-protector -fno-pie -fno-pic -fno-asynchronous-unwind-tables -fno-unwind-tables -mcmodel=kernel -mno-red-zone -Ikernel/include -Ikernel/core/include
    LDFLAGS = -T linker.ld -nostdlib -static -z max-page-size=0x1000
    SRCS = kernel/src/kernel.c kernel/src/serial.c kernel/src/gdt.c kernel/src/idt.c kernel/src/memory.c kernel/src/pmm.c kernel/src/log.c kernel/src/console.c kernel/src/vfs.c kernel/src/shell.c kernel/src/auth.c
    OBJS = $(addprefix build/x86_64/, $(notdir $(SRCS:.c=.o)))
    TARGET = build/x86_64/kernel.elf
    ISO = build/x86_64/myos.iso

else ifeq ($(ARCH),armv6)
    CC = arm-none-eabi-gcc
    LD = arm-none-eabi-ld
    CFLAGS = -Wall -Wextra -O2 -ffreestanding -mcpu=arm1176jzf-s -marm -Ikernel/include
    LDFLAGS = -T linker/armv6.ld -nostdlib -static
    SRCS = arch/armv6/kernel.c arch/armv6/cpu.c arch/armv6/vectors.S kernel/src/serial.c kernel/src/mailbox.c kernel/src/hdmi.c kernel/src/pmm.c kernel/src/console.c kernel/src/vfs.c kernel/src/shell.c kernel/src/auth.c kernel/src/log.c
    OBJS = build/armv6/kernel.o build/armv6/cpu.o build/armv6/vectors.o build/armv6/serial.o build/armv6/mailbox.o build/armv6/hdmi.o build/armv6/pmm.o build/armv6/console.o build/armv6/vfs.o build/armv6/shell.o build/armv6/auth.o build/armv6/log.o
    TARGET = build/armv6/kernel.img

else ifeq ($(ARCH),aarch64)
    CC = aarch64-none-elf-gcc
    LD = aarch64-none-elf-ld
    CFLAGS = -Wall -Wextra -O2 -ffreestanding -mcpu=cortex-a53 -Ikernel/include
    LDFLAGS = -T linker/aarch64.ld -nostdlib -static
    SRCS = arch/aarch64/kernel.c arch/aarch64/cpu.c arch/aarch64/vectors.S kernel/src/serial.c kernel/src/pmm.c kernel/src/console.c kernel/src/vfs.c kernel/src/shell.c kernel/src/auth.c kernel/src/log.c
    OBJS = $(addprefix build/aarch64/, $(notdir $(SRCS:.c=.o) $(SRCS:.S=.o)))
    TARGET = build/aarch64/kernel8.img
endif

all: $(TARGET)

ifeq ($(ARCH),x86_64)
build/x86_64/%.o: kernel/src/%.c
	@mkdir -p build/x86_64
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) linker.ld
	@mkdir -p build/x86_64
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

# (Limine binaries are pre-provided in limine-binary/)

$(ISO): $(TARGET) limine.conf
	rm -rf iso_root
	mkdir -p iso_root/boot/limine iso_root/EFI/BOOT
	cp $(TARGET) iso_root/boot/kernel.elf
	cp limine.conf iso_root/boot/limine/limine.conf
	cp limine-binary/limine-bios.sys iso_root/boot/limine/
	cp limine-binary/limine-bios-cd.bin iso_root/boot/
	cp limine-binary/limine-uefi-cd.bin iso_root/boot/
	cp limine-binary/BOOTX64.EFI iso_root/EFI/BOOT/BOOTX64.EFI
	xorriso -as mkisofs -R -r -J -b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
		-apm-block-size 2048 --efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(ISO)
	limine-binary/limine bios-install $(ISO)

iso: $(ISO)
run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio -no-reboot -no-hpet

else ifeq ($(ARCH),armv6)
build/armv6/%.o: arch/armv6/%.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/%.o: arch/armv6/%.S
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/%.o: kernel/src/%.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/kernel.o: arch/armv6/kernel.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/cpu.o: arch/armv6/cpu.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/vectors.o: arch/armv6/vectors.S
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/serial.o: kernel/src/serial.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/mailbox.o: kernel/src/mailbox.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/hdmi.o: kernel/src/hdmi.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/pmm.o: kernel/src/pmm.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/console.o: kernel/src/console.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/vfs.o: kernel/src/vfs.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/shell.o: kernel/src/shell.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/auth.o: kernel/src/auth.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

build/armv6/log.o: kernel/src/log.c
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) linker/armv6.ld
	@mkdir -p build/armv6
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -lgcc -o $(TARGET)
	arm-none-eabi-objcopy $(TARGET) -O binary build/armv6/kernel7.img
	cp build/armv6/kernel7.img kernel7.img

else ifeq ($(ARCH),aarch64)
build/aarch64/%.o: arch/aarch64/%.c
	@mkdir -p build/aarch64
	$(CC) $(CFLAGS) -c $< -o $@

build/aarch64/%.o: arch/aarch64/%.S
	@mkdir -p build/aarch64
	$(CC) $(CFLAGS) -c $< -o $@

build/aarch64/%.o: kernel/src/%.c
	@mkdir -p build/aarch64
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS) linker/aarch64.ld
	@mkdir -p build/aarch64
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)
endif

clean:
	rm -rf build iso_root
