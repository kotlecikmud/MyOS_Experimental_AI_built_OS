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
    if (auth_authenticate()) {
        shell_run();
    }
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
