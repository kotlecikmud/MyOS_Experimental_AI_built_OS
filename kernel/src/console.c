#include "console.h"
#include "serial.h"

void console_init(void) {
    serial_print("[Console] Initializing serial console environment...\n");
}

void console_putchar(char c) {
    serial_putchar(c);
}

void console_print(const char* s) {
    serial_print(s);
}

char console_getchar(void) {
    // In headless serial mode, poll serial port or return 0 if no char
    return serial_getchar();
}

int console_getline(char* buf, int max_len) {
    int idx = 0;
    while (idx < max_len - 1) {
        char c = console_getchar();
        if (c == '\0') {
            continue;
        }
        if (c == '\r' || c == '\n') {
            console_putchar('\n');
            buf[idx] = '\0';
            return idx;
        } else if (c == '\b' || c == 127) {
            if (idx > 0) {
                idx--;
                console_putchar('\b');
                console_putchar(' ');
                console_putchar('\b');
            }
        } else {
            buf[idx++] = c;
            console_putchar(c);
        }
    }
    buf[max_len - 1] = '\0';
    return idx;
}
