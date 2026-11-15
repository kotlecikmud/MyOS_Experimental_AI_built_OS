#ifndef RPI_HDMI_H
#define RPI_HDMI_H

#include <stdint.h>
#include <stdbool.h>

bool rpi_hdmi_init(uint32_t width, uint32_t height, uint32_t bpp);
void rpi_hdmi_clear(uint32_t color);
void rpi_hdmi_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void rpi_hdmi_draw_char(uint32_t x, uint32_t y, char c, uint32_t fg, uint32_t bg);
void rpi_hdmi_print(const char* s);
void rpi_hdmi_scroll(void);

#endif
