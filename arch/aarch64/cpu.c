#include "serial.h"
#include "kernel/log.h"

// AArch64 (Cortex-A53) CPU Initialization and Exception-Level Transition
// Exact Execution State & EL2 to EL1 Transition Sequence:
// 1. CurrentEL Detection: Read CurrentEL system register. RPi 64-bit firmware (e.g. start.elf / armstub8.bin) typically enters kernel at EL2.
// 2. If CurrentEL == 2, configure HCR_EL2 (set RW bit 31 for AArch64 EL1, disable traps).
// 3. Set up SPSR_EL2 for return to EL1h (masking DAIF: 0x3c5 for EL1h with interrupts disabled).
// 4. Set ELR_EL2 to return point (label or function address).
// 5. Initialize SP_EL1 to a dedicated stack region.
// 6. Execute 'eret' to transition to EL1.
// 7. Configure VBAR_EL1 to point to 2048-byte aligned exception vector table in EL1.

extern void aarch64_vector_table(void);

void aarch64_cpu_init(void) {
    serial_print("[AArch64] Initializing Cortex-A53 CPU state & Exception Level...\n");

    uint64_t current_el = 0;
    __asm__ volatile ("mrs %0, CurrentEL" : "=r" (current_el));
    current_el >>= 2;

    if (current_el == 3) {
        serial_print("[AArch64] ERROR: Entered at EL3. Halting.\n");
        for (;;) { __asm__ volatile ("wfi"); }
    } else if (current_el == 2) {
        serial_print("[AArch64] Entered at EL2. Transitioning to EL1...\n");
        
        // Configure HCR_EL2: RW bit 31 = 1 (EL1 is AArch64)
        __asm__ volatile ("msr hcr_el2, %0" : : "r" ((uint64_t)(1UL << 31)));

        // Setup SPSR_EL2: EL1h mode with DAIF masked (0x3c5)
        __asm__ volatile ("msr spsr_el2, %0" : : "r" ((uint64_t)0x3c5));

        // Setup ELR_EL2 to point to our EL1 continuation label
        __asm__ volatile (
            "adr x0, aarch64_el1_entry\n\t"
            "msr elr_el2, x0\n\t"
            "eret\n\t"
            "aarch64_el1_entry:\n\t"
            ::: "x0", "memory"
        );
    } else {
        serial_print("[AArch64] Entered directly at EL1.\n");
    }

    // Now executing at EL1: set up stack pointer SP_EL1 and VBAR_EL1
    // Register 2048-byte aligned vector table to VBAR_EL1
    uint64_t vbar_addr = (uint64_t)&aarch64_vector_table;
    __asm__ volatile ("msr vbar_el1, %0" : : "r" (vbar_addr));

    // Mask interrupts during setup via DAIF set
    __asm__ volatile ("msr daifset, #0xf");
    serial_print("[AArch64] EL1 VBAR_EL1 registered and exception environment ready.\n");
}

void aarch64_exception_handler(uint64_t esr, uint64_t elr) {
    (void)esr;
    (void)elr;
    serial_print("\n[AArch64 PANIC] Fatal Synchronous Exception / Abort at EL1!\n");
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
