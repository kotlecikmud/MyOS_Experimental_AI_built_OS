#include "console.h"
#include "serial.h"
#if defined(__arm__)
#include "hdmi.h"
static int hdmi_ready;
#endif

void console_init(void) {
    serial_print("[Console] Initializing console...\r\n");
#if defined(__arm__)
    hdmi_ready = rpi_hdmi_init(640, 480, 32);
    if (hdmi_ready) rpi_hdmi_print("MYOS - RASPBERRY PI ZERO W\nHDMI FRAMEBUFFER ONLINE\n\n");
    else serial_print("[Console] HDMI framebuffer unavailable; using UART only\r\n");
#endif
}

void console_putchar(char c) {
    serial_putchar(c);
#if defined(__arm__)
    if (hdmi_ready) { char s[2] = { c, 0 }; rpi_hdmi_print(s); }
#endif
}
void console_print(const char *s) {
    serial_print(s);
#if defined(__arm__)
    if (hdmi_ready) rpi_hdmi_print(s);
#endif
}
char console_getchar(void) { return serial_getchar(); }
int console_getline(char *buf, int max_len) {
    int idx = 0;
    while (idx < max_len - 1) {
        char c = console_getchar();
        if (c == '\r' || c == '\n') {
            console_putchar('\n'); buf[idx] = '\0'; return idx;
        } else if (c == '\b' || c == 127) {
            if (idx > 0) { --idx; console_putchar('\b'); console_putchar(' '); console_putchar('\b'); }
        } else if (c) { buf[idx++] = c; console_putchar(c); }
    }
    buf[max_len - 1] = '\0';
    return idx;
}
