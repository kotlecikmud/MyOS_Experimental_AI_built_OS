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

int console_check_key(void) { return serial_received(); }

void console_draw_cursor(bool visible) {
    static bool cursor_drawn = false;
    if (visible && !cursor_drawn) {
        console_putchar('_');
        cursor_drawn = true;
    } else if (!visible && cursor_drawn) {
        console_print("\b \b");
        cursor_drawn = false;
    }
}

/* Read a line without blocking the CPU indefinitely. The cursor is refreshed
 * while waiting, and every received character immediately removes it. */
int console_getline(char *buf, int max_len) {
    if (!buf || max_len < 2) return 0;

    int idx = 0;
    unsigned int blink = 0;
    bool cursor_visible = false;

    for (;;) {
        char c;
        if (!serial_try_getchar(&c)) {
            /* A bounded delay makes the blink visible on both UART and HDMI.
             * This is a polling console; no timer/interrupt dependency. */
            for (volatile unsigned int d = 0; d < 12000; ++d) {
                __asm__ volatile ("nop");
            }
            if (++blink >= 35) {
                cursor_visible = !cursor_visible;
                console_draw_cursor(cursor_visible);
                blink = 0;
            }
            continue;
        }

        console_draw_cursor(false);
        cursor_visible = false;
        blink = 0;

        if (c == '\r' || c == '\n') {
            console_putchar('\n');
            buf[idx] = '\0';
            return idx;
        }
        if (c == '\b' || c == 127) {
            if (idx > 0) {
                --idx;
                console_print("\b \b");
            }
            continue;
        }
        if (c >= 32 && c <= 126 && idx < max_len - 1) {
            buf[idx++] = c;
            console_putchar(c);
        }
    }
}
