#include "uart.h"
#include "gpio.h"

// Simple delay function
static void delay(uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
        asm volatile("nop");
    }
}

// Initialize UART0,初始化串口，设置IO口的功能，设置波特率，启用FIFO
void uart_init(void) {
    // Disable UART0
    *UART0_CR = 0;
    
    // Wait for end of transmission
    delay(150);
    
    // Setup GPIO pins 14 and 15 for UART0 function
    // GPIO 14 (TX) - Function 4 (ALT0)
    // GPIO 15 (RX) - Function 4 (ALT0)
    
    // Clear GPIO 14 and 15 settings
    uint32_t selector = *GPFSEL1;
    selector &= ~((7 << 12) | (7 << 15)); // Clear bits 12-14 (GPIO14) and 15-17 (GPIO15)
    selector |= (4 << 12) | (4 << 15);    // Set ALT0 (function 4) for both pins
    *GPFSEL1 = selector;
    
    // Disable pull-up/down for GPIO 14 and 15
    *GPIO_PUP_PDN_CNTRL_REG0 &= ~((3 << 28) | (3 << 30)); // Clear bits for GPIO 14 and 15
    
    // Clear all pending interrupts
    *UART0_ICR = 0x7FF;
    
    // Set baud rate - 115200 baud
    // UART clock is 48MHz on Raspberry Pi 4
    // Divisor = UART clock / (16 * Baud rate)
    // Divisor = 48000000 / (16 * 115200) = 26.04
    // Integer part = 26
    // Fractional part = 0.04 * 64 = 3 (rounded)
    *UART0_IBRD = 26;    // Integer part
    *UART0_FBRD = 3;     // Fractional part
    
    // Wait for end of transmission
    delay(150);
    
    // Enable FIFO & 8-bit data transmission (1 stop bit, no parity)
    *UART0_LCRH = UART_LCRH_FEN | UART_LCRH_WLEN_8;
    
    // Enable UART0, receive and transmit
    *UART0_CR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
    
    // Allow UART to settle
    delay(150);
}

// Send a single character，发送单个字符
void uart_putc(unsigned char c) {
    // Wait until transmit FIFO is not full
    while (*UART0_FR & UART_FR_TXFF);
    
    // Write character to data register
    *UART0_DR = c;
    
    // If we sent a newline, also send a carriage return
    if (c == '\n') {
        uart_putc('\r');
    }
}

// Send a string字符串
void uart_puts(const char* str) {
    while (*str) {
        uart_putc(*str++);
    }
}

// Very simple implementation of printf (supports %s, %d, %x only)格式化输出
void uart_printf(const char* fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    
    char c;
    while ((c = *fmt++)) {
        if (c != '%') {
            uart_putc(c);
            continue;
        }
        
        c = *fmt++;
        if (!c) break;
        
        switch (c) {
            case 's': {
                const char* s = __builtin_va_arg(args, const char*);
                uart_puts(s ? s : "(null)");
                break;
            }
            case 'd': {
                int num = __builtin_va_arg(args, int);
                if (num < 0) {
                    uart_putc('-');
                    num = -num;
                }
                
                // Simple way to print a number: convert to string
                char buffer[12]; // Max 10 digits for 32-bit int, sign, and null terminator
                char* p = buffer + sizeof(buffer) - 1;
                *p = '\0';
                
                do {
                    *(--p) = '0' + (num % 10);
                    num /= 10;
                } while (num > 0);
                
                uart_puts(p);
                break;
            }
            case 'x': {
                unsigned int num = __builtin_va_arg(args, unsigned int);
                char buffer[9]; // 8 hex digits + null terminator
                char* p = buffer + sizeof(buffer) - 1;
                *p = '\0';
                
                static const char hex_digits[] = "0123456789abcdef";
                do {
                    *(--p) = hex_digits[num & 0xF];
                    num >>= 4;
                } while (num > 0);
                
                uart_putc('0');
                uart_putc('x');
                uart_puts(p);
                break;
            }
            default:
                uart_putc('%');
                uart_putc(c);
                break;
        }
    }
    
    __builtin_va_end(args);
}
