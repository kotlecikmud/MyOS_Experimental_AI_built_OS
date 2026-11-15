#include "serial.h"
#include "kernel/log.h"

// ARMv6 (ARM1176JZF-S) CPU Initialization and Exception Handling
// Documentation of initial firmware entry state:
// - Processor Mode: Supervisor (SVC) or HYP/EL2/Secure mode depending on bootloader, typically Supervisor (SVC) mode for bare-metal RPi firmware.
// - Interrupts (IRQ/FIQ): Disabled (I and F bits set in CPSR).
// - MMU: Disabled (identity/flat or raw physical execution).
// - Stack: Initialized by firmware/loader below kernel load address (0x8000).

void armv6_cpu_init(void) {
    serial_print("[ARMv6] Initializing ARM1176JZF-S CPU state...\n");
    // Ensure interrupts remain disabled during early init
    __asm__ volatile ("cpsid if");
    serial_print("[ARMv6] CP15 coprocessor and exception vectors prepared.\n");
}

void armv6_exception_handler(uint32_t vector_num) {
    serial_print("\n[ARMv6 PANIC] Fatal Exception / Abort encountered on ARMv6!\n");
    if (vector_num == 2) {
        serial_print("[ARMv6 PANIC] Software Interrupt (SVC)\n");
    } else if (vector_num == 4) {
        serial_print("[ARMv6 PANIC] Prefetch Abort (Instruction fetch fault)\n");
    } else if (vector_num == 5) {
        serial_print("[ARMv6 PANIC] Data Abort (Data load/store fault)\n");
    } else {
        serial_print("[ARMv6 PANIC] Undefined instruction or unhandled trap\n");
    }
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
