#ifndef KERNEL_LOG_H
#define KERNEL_LOG_H

void log_info(const char* message);
void log_warning(const char* message);
void log_error(const char* message);
void kernel_panic(const char* message);

#endif /* KERNEL_LOG_H */
