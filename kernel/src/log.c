#include "kernel/log.h"
#include "serial.h"
#include "kernel.h"

void log_info(const char* message) {
    serial_print("[INFO] ");
    serial_print(message);
    serial_print("\n");
}

void log_warning(const char* message) {
    serial_print("[WARNING] ");
    serial_print(message);
    serial_print("\n");
}

void log_error(const char* message) {
    serial_print("[ERROR] ");
    serial_print(message);
    serial_print("\n");
}

void kernel_panic(const char* message) {
    serial_print("\n==================================================\n");
    serial_print("                  KERNEL PANIC                    \n");
    serial_print("==================================================\n");
    serial_print("[PANIC] ");
    serial_print(message);
    serial_print("\n");
    serial_print("System halted safely.\n");
    serial_print("==================================================\n");

    // Halt CPU
    for (;;) {
#ifdef __x86_64__
        __asm__ volatile ("cli; hlt");
#else
        __asm__ volatile ("");
#endif
    }
}