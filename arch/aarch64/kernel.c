#include "../../kernel/include/demo.h"
#include "../../kernel/include/pmm.h"
#include "../../kernel/include/console.h"
#include "../../kernel/include/vfs.h"
#include "../../kernel/include/shell.h"
#include "../../kernel/include/auth.h"

extern void aarch64_cpu_init(void);

void kernel_main(void) {
    // Raspberry Pi Zero 2 W (AArch64) kernel main entry
    aarch64_cpu_init();
    pmm_init();
    aarch64_pmm_init();
    console_init();
    vfs_init();
    auth_init();
    shell_init();

    /* Demo-first boot: current VFS cannot read the physical FAT boot partition.
     * Start the demo directly. Press a key to enter the shell. */
    demo_run();
    shell_run();
    for (;;) { __asm__ volatile ("wfi"); }
}
