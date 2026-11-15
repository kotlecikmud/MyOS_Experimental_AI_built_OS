#include "../../kernel/include/demo.h"
#include "../../kernel/include/pmm.h"
#include "../../kernel/include/console.h"
#include "../../kernel/include/vfs.h"
#include "../../kernel/include/shell.h"
#include "../../kernel/include/auth.h"
#include "../../kernel/include/serial.h"

extern void armv6_cpu_init(void);

void kernel_main(void) {
    // Raspberry Pi Zero W (ARMv6) kernel main entry
    armv6_cpu_init();
    serial_init();
    serial_print("\\r\\n[MyOS] ARMv6 kernel entered\\r\\n");
    pmm_init();
    armv6_pmm_init();
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
