#include "hdmi.h"
#include "mailbox.h"
#include "serial.h"
#include <stddef.h>

#define FB_WIDTH  640u
#define FB_HEIGHT 480u

static volatile uint8_t *framebuffer;
static uint32_t fb_width, fb_height, fb_pitch, fb_depth;
static uint32_t cursor_x, cursor_y;

static volatile uint32_t __attribute__((aligned(16))) mbox[35];

static int mailbox_tag_ok(unsigned index) {
    return (mbox[index] & 0x80000000u) != 0;
}

bool rpi_hdmi_init(uint32_t width, uint32_t height, uint32_t bpp) {
    if (!width || !height || (bpp != 16 && bpp != 24 && bpp != 32)) return false;

    for (unsigned i = 0; i < 35; ++i) mbox[i] = 0;
    mbox[0] = sizeof(mbox);
    mbox[1] = 0;

    /* Set physical dimensions. */
    mbox[2] = 0x00048003; mbox[3] = 8; mbox[4] = 8;
    mbox[5] = width; mbox[6] = height;
    /* Set virtual dimensions. */
    mbox[7] = 0x00048004; mbox[8] = 8; mbox[9] = 8;
    mbox[10] = width; mbox[11] = height;
    /* Set virtual offset. */
    mbox[12] = 0x00048009; mbox[13] = 8; mbox[14] = 8;
    mbox[15] = 0; mbox[16] = 0;
    /* Set depth and RGB order. */
    mbox[17] = 0x00048005; mbox[18] = 4; mbox[19] = 4; mbox[20] = bpp;
    mbox[21] = 0x00048006; mbox[22] = 4; mbox[23] = 4; mbox[24] = 1;
    /* Allocate framebuffer, 4 KiB aligned. */
    mbox[25] = 0x00040001; mbox[26] = 8; mbox[27] = 8;
    mbox[28] = 4096; mbox[29] = 0;
    /* Obtain actual pitch. */
    mbox[30] = 0x00040008; mbox[31] = 4; mbox[32] = 4; mbox[33] = 0;
    mbox[34] = 0;

    if (rpi_mailbox_property((void *)mbox) != 0 || mbox[1] != 0x80000000u ||
        !mailbox_tag_ok(27) || !mailbox_tag_ok(32)) {
        serial_print("[HDMI] GPU framebuffer request failed\r\n");
        return false;
    }

    uint32_t bus_addr = mbox[28];
    uint32_t size = mbox[29];
    fb_pitch = mbox[33];
    fb_width = mbox[5];
    fb_height = mbox[6];
    fb_depth = mbox[20];

    if (!bus_addr || !size || !fb_pitch || !fb_width || !fb_height ||
        (fb_depth != 16 && fb_depth != 24 && fb_depth != 32) ||
        fb_pitch < fb_width * ((fb_depth + 7u) / 8u)) {
        serial_print("[HDMI] GPU returned invalid framebuffer parameters\r\n");
        return false;
    }

    /* VideoCore returns a bus address; Pi Zero RAM is in the low 1 GiB. */
    framebuffer = (volatile uint8_t *)(uintptr_t)(bus_addr & 0x3fffffffu);
    cursor_x = cursor_y = 0;
    rpi_hdmi_clear(0x00101828u);
    serial_print("[HDMI] Framebuffer ready\r\n");
    return true;
}

void rpi_hdmi_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    if (!framebuffer || x >= fb_width || y >= fb_height) return;
    volatile uint8_t *p = framebuffer + y * fb_pitch + x * ((fb_depth + 7u) / 8u);
    if (fb_depth == 16) {
        uint16_t c = (uint16_t)((((color >> 19) & 0x1f) << 11) |
                                (((color >> 10) & 0x3f) << 5) |
                                ((color >> 3) & 0x1f));
        *(volatile uint16_t *)p = c;
    } else {
        p[0] = (uint8_t)color;
        p[1] = (uint8_t)(color >> 8);
        p[2] = (uint8_t)(color >> 16);
        if (fb_depth == 32) p[3] = (uint8_t)(color >> 24);
    }
}

void rpi_hdmi_clear(uint32_t color) {
    if (!framebuffer) return;
    for (uint32_t y = 0; y < fb_height; ++y)
        for (uint32_t x = 0; x < fb_width; ++x)
            rpi_hdmi_draw_pixel(x, y, color);
}

/* Compact 5x7 uppercase font: A-Z, 0-9 and common punctuation. */
static const uint8_t glyphs[][5] = {
 {0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},
 {0x3e,0x41,0x41,0x41,0x22},{0x7f,0x41,0x41,0x22,0x1c},
 {0x7f,0x49,0x49,0x49,0x41},{0x7f,0x09,0x09,0x09,0x01},
 {0x3e,0x41,0x49,0x49,0x7a},{0x7f,0x08,0x08,0x08,0x7f},
 {0x00,0x41,0x7f,0x41,0x00},{0x20,0x40,0x41,0x3f,0x01},
 {0x7f,0x08,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
 {0x7f,0x02,0x0c,0x02,0x7f},{0x7f,0x04,0x08,0x10,0x7f},
 {0x3e,0x41,0x41,0x41,0x3e},{0x7f,0x09,0x09,0x09,0x06},
 {0x3e,0x41,0x51,0x21,0x5e},{0x7f,0x09,0x19,0x29,0x46},
 {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7f,0x01,0x01},
 {0x3f,0x40,0x40,0x40,0x3f},{0x1f,0x20,0x40,0x20,0x1f},
 {0x3f,0x40,0x38,0x40,0x3f},{0x63,0x14,0x08,0x14,0x63},
 {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},
 {0x3e,0x51,0x49,0x45,0x3e},{0x00,0x42,0x7f,0x40,0x00},
 {0x62,0x51,0x49,0x49,0x46},{0x22,0x49,0x49,0x49,0x36},
 {0x18,0x14,0x12,0x7f,0x10},{0x2f,0x49,0x49,0x49,0x31},
 {0x3e,0x49,0x49,0x49,0x32},{0x01,0x71,0x09,0x05,0x03},
 {0x36,0x49,0x49,0x49,0x36},{0x26,0x49,0x49,0x49,0x3e}
};

static int glyph_index(char c) {
    if (c >= 'a' && c <= 'z') c = (char)(c - 'a' + 'A');
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return 26 + c - '0';
    return -1;
}

void rpi_hdmi_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg) {
    int gi = glyph_index(c);
    for (uint32_t row = 0; row < 8; ++row) {
        for (uint32_t col = 0; col < 6; ++col) {
            int on = 0;
            if (gi >= 0 && row < 7 && col < 5)
                on = (glyphs[gi][col] >> row) & 1;
            else if (c == '.' && row == 6 && col == 2) on = 1;
            else if (c == ':' && (row == 2 || row == 5) && col == 2) on = 1;
            else if (c == '-' && row == 3 && col < 5) on = 1;
            else if (c == '>' && col == row / 2 && row < 7) on = 1;
            rpi_hdmi_draw_pixel(x + col, y + row, on ? fg : bg);
        }
    }
}

static void scroll_if_needed(void) {
    if (!framebuffer || cursor_y + 9 < fb_height) return;
    for (uint32_t y = 0; y + 9 < fb_height; ++y)
        for (uint32_t x = 0; x < fb_width; ++x) {
            volatile uint8_t *d = framebuffer + y * fb_pitch + x * ((fb_depth + 7u) / 8u);
            volatile uint8_t *s = framebuffer + (y + 9) * fb_pitch + x * ((fb_depth + 7u) / 8u);
            for (uint32_t b = 0; b < (fb_depth + 7u) / 8u; ++b) d[b] = s[b];
        }
    cursor_y = fb_height > 9 ? fb_height - 9 : 0;
}

void rpi_hdmi_print(const char *s) {
    if (!s || !framebuffer) return;
    while (*s) {
        if (*s == '\r') { ++s; continue; }
        if (*s == '\n') { cursor_x = 0; cursor_y += 10; scroll_if_needed(); ++s; continue; }
        rpi_hdmi_draw_char(cursor_x, cursor_y, *s++, 0x00ffffffu, 0x00101828u);
        cursor_x += 7;
        if (cursor_x + 6 >= fb_width) { cursor_x = 0; cursor_y += 10; scroll_if_needed(); }
    }
}
void rpi_hdmi_scroll(void) { scroll_if_needed(); }
