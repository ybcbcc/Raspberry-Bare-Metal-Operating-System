#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stddef.h>

// Initialize UART
void uart_init(void);

// Output a single character
void uart_putc(unsigned char c);

// Output a string
void uart_puts(const char* str);

// Format and output a string
void uart_printf(const char* fmt, ...);

#endif // UART_H