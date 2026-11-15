#include "idt.h"
#include "serial.h"
#include "kernel.h"
#include "kernel/log.h"

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

struct cpu_context {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t interrupt_number, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

struct idt_entry idt[256];
struct idt_ptr ip;

void idt_set_gate(uint8_t vector, void* handler, uint16_t selector, uint8_t flags) {
    uint64_t addr = (uint64_t)handler;
    idt[vector].offset_low = addr & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].ist = 0;
    idt[vector].type_attr = flags;
    idt[vector].offset_middle = (addr >> 16) & 0xFFFF;
    idt[vector].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

void exception_handler_common(struct cpu_context* ctx) {
    (void)ctx;
    __asm__ volatile ("cli");
    serial_print("\n==================== X86-64 CPU EXCEPTION ====================\n");
    serial_print("[EXCEPTION] Vector / Interrupt Number: ");
    // print vector number via serial or simple format
    serial_print(" (Fatal)\n");
    serial_print("[EXCEPTION] Error Code: 0x");
    serial_print("[EXCEPTION] RIP: 0x");
    serial_print("[EXCEPTION] RSP: 0x");
    serial_print("[EXCEPTION] RFLAGS: 0x");
    serial_print("============================================================\n");
    kernel_panic("x86-64 Fatal Exception");
}

void default_exception_handler(void) {
    __asm__ volatile ("cli");
    serial_print("\n[EXCEPTION] Fatal CPU Exception / Interrupt triggered!\n");
    serial_print("[EXCEPTION] Halting system safely (CLI + HLT).\n");
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

void idt_init(void) {
    serial_print("[IDT] Initializing Interrupt Descriptor Table...\n");
    ip.limit = sizeof(idt) - 1;
    ip.base = (uint64_t)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, default_exception_handler, 0x08, 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m"(ip));
    serial_print("[IDT] IDT initialized and loaded successfully (256 gates populated).\n");
}
