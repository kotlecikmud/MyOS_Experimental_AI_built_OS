import sys
import subprocess
import os

args = sys.argv[1:]
print(f"[Python Make Runner] Invoked with: {args}")

# Simple build orchestrator in Python that implements Makefile rules or invokes compilers
# Let's check ARCH parameter
arch = "x86_64"
for arg in args:
    if arg.startswith("ARCH="):
        arch = arg.split("=")[1]
    elif arg in ["x86_64", "armv6", "aarch64"]:
        arch = arg
    elif arg == "pi-zero-w":
        arch = "armv6"
    elif arg == "pi-zero-2w":
        arch = "aarch64"

print(f"[Python Make Runner] Target ARCH: {arch}")

os.makedirs(f"build/{arch}", exist_ok=True)

if arch == "x86_64":
    cc = os.environ.get("CC", "gcc")
    ld = os.environ.get("LD", "ld")
    cflags = ["-Wall", "-Wextra", "-O2", "-ffreestanding", "-fno-stack-protector", "-fno-pie", "-fno-pic", "-mcmodel=kernel", "-mno-red-zone", "-Ikernel/include"]
    ldflags = ["-T", "linker.ld", "-nostdlib", "-static", "-z", "max-page-size=0x1000"]
    sources = ["kernel/src/kernel.c", "kernel/src/serial.c", "kernel/src/gdt.c", "kernel/src/idt.c", "kernel/src/memory.c", "kernel/src/pmm.c", "kernel/src/log.c", "kernel/src/console.c", "kernel/src/vfs.c", "kernel/src/shell.c", "kernel/src/auth.c", "kernel/src/demo.c"]
    objs = []
    for src in sources:
        obj = f"build/x86_64/{os.path.basename(src).replace('.c', '.o')}"
        objs.append(obj)
        cmd = [cc] + cflags + ["-c", src, "-o", obj]
        print("Executing:", " ".join(cmd))
        res = subprocess.run(cmd)
        if res.returncode != 0:
            print(f"Error compiling {src}")
            sys.exit(res.returncode)
    
    target = "build/x86_64/kernel.elf"
    ld_cmd = [ld] + ldflags + objs + ["-o", target]
    print("Executing link:", " ".join(ld_cmd))
    res = subprocess.run(ld_cmd)
    if res.returncode != 0:
        print("Error linking x86_64 kernel")
        sys.exit(res.returncode)
    print("Successfully built x86_64 kernel:", target)

elif arch == "armv6":
    cc = "arm-none-eabi-gcc"
    ld = "arm-none-eabi-ld"
    cflags = ["-Wall", "-Wextra", "-O2", "-ffreestanding", "-mcpu=arm1176jzf-s", "-marm", "-Ikernel/include"]
    ldflags = ["-T", "linker/armv6.ld", "-nostdlib", "-static"]
    srrs = [
        ("arch/armv6/kernel.c", "c"),
        ("arch/armv6/cpu.c", "c"),
        ("arch/armv6/vectors.S", "S"),
        ("kernel/src/serial.c", "c"),
        ("kernel/src/pmm.c", "c"),
        ("kernel/src/console.c", "c"),
        ("kernel/src/vfs.c", "c"),
        ("kernel/src/shell.c", "c"),
        ("kernel/src/auth.c", "c"),
        ("kernel/src/log.c", "c"),
        ("kernel/src/demo.c", "c"),
    ]
    objs = []
    for src, stype in srrs:
        obj = f"build/armv6/{os.path.basename(src).replace('.c', '.o').replace('.S', '.o')}"
        objs.append(obj)
        cmd = [cc] + cflags + ["-c", src, "-o", obj]
        print("Executing:", " ".join(cmd))
        res = subprocess.run(cmd)
        if res.returncode != 0:
            print(f"Error compiling {src}")
            sys.exit(res.returncode)
            
    target = "build/armv6/kernel.img"
    ld_cmd = [ld] + ldflags + objs + ["-o", target]
    print("Executing link:", " ".join(ld_cmd))
    res = subprocess.run(ld_cmd)
    if res.returncode != 0:
        print("Error linking armv6 kernel")
        sys.exit(res.returncode)
    print("Successfully built armv6 kernel:", target)

elif arch == "aarch64":
    cc = "aarch64-none-elf-gcc"
    ld = "aarch64-none-elf-ld"
    cflags = ["-Wall", "-Wextra", "-O2", "-ffreestanding", "-mcpu=cortex-a53", "-Ikernel/include"]
    ldflags = ["-T", "linker/aarch64.ld", "-nostdlib", "-static"]
    srrs = [
        ("arch/aarch64/kernel.c", "c"),
        ("arch/aarch64/cpu.c", "c"),
        ("arch/aarch64/vectors.S", "S"),
        ("kernel/src/serial.c", "c"),
        ("kernel/src/pmm.c", "c"),
        ("kernel/src/console.c", "c"),
        ("kernel/src/vfs.c", "c"),
        ("kernel/src/shell.c", "c"),
        ("kernel/src/auth.c", "c"),
        ("kernel/src/log.c", "c"),
        ("kernel/src/demo.c", "c"),
    ]
    objs = []
    for src, stype in srrs:
        obj = f"build/aarch64/{os.path.basename(src).replace('.c', '.o').replace('.S', '.o')}"
        objs.append(obj)
        cmd = [cc] + cflags + ["-c", src, "-o", obj]
        print("Executing:", " ".join(cmd))
        res = subprocess.run(cmd)
        if res.returncode != 0:
            print(f"Error compiling {src}")
            sys.exit(res.returncode)
            
    target = "build/aarch64/kernel8.img"
    ld_cmd = [ld] + ldflags + objs + ["-o", target]
    print("Executing link:", " ".join(ld_cmd))
    res = subprocess.run(ld_cmd)
    if res.returncode != 0:
        print("Error linking aarch64 kernel")
        sys.exit(res.returncode)
    print("Successfully built aarch64 kernel:", target)
