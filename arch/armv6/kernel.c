#include "../../kernel/include/pmm.h"
#include "../../kernel/include/console.h"
#include "../../kernel/include/vfs.h"
#include "../../kernel/include/shell.h"
#include "../../kernel/include/auth.h"

extern void armv6_cpu_init(void);

void kernel_main(void) {
    // Raspberry Pi Zero W (ARMv6) kernel main entry
    armv6_cpu_init();
    pmm_init();
    armv6_pmm_init();
    console_init();
    vfs_init();
    auth_init();
    shell_init();
    if (auth_authenticate()) {
        shell_run();
    }
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
