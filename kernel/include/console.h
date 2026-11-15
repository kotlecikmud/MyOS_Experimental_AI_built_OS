#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>
#include <stddef.h>

void console_init(void);
void console_putchar(char c);
void console_print(const char* s);
char console_getchar(void);
int console_getline(char* buf, int max_len);

#endif /* CONSOLE_H */
