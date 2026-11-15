#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void console_init(void);
void console_putchar(char c);
void console_print(const char* s);
char console_getchar(void);
int console_getline(char* buf, int max_len);
int console_check_key(void);
void console_draw_cursor(bool visible);

#endif /* CONSOLE_H */
