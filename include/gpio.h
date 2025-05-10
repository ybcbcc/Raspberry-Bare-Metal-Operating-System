#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// Raspberry Pi 4 peripherals base address
#define PERIPHERAL_BASE 0xFE000000

// GPIO registers
#define GPIO_BASE (PERIPHERAL_BASE + 0x200000)

// GPIO function select registers
#define GPFSEL0 ((volatile uint32_t *)(GPIO_BASE + 0x00))
#define GPFSEL1 ((volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPFSEL2 ((volatile uint32_t *)(GPIO_BASE + 0x08))
#define GPFSEL3 ((volatile uint32_t *)(GPIO_BASE + 0x0C))
#define GPFSEL4 ((volatile uint32_t *)(GPIO_BASE + 0x10))

// GPIO pull-up/down registers
#define GPIO_PUP_PDN_CNTRL_REG0 ((volatile uint32_t *)(GPIO_BASE + 0xE4)) // GPIO 0-15
#define GPIO_PUP_PDN_CNTRL_REG1 ((volatile uint32_t *)(GPIO_BASE + 0xE8)) // GPIO 16-31
#define GPIO_PUP_PDN_CNTRL_REG2 ((volatile uint32_t *)(GPIO_BASE + 0xEC)) // GPIO 32-47
#define GPIO_PUP_PDN_CNTRL_REG3 ((volatile uint32_t *)(GPIO_BASE + 0xF0)) // GPIO 48-57
// UART0 (PL011) registers
#define UART0_BASE (PERIPHERAL_BASE + 0x201000)
#define UART0_DR ((volatile uint32_t *)(UART0_BASE + 0x00))
#define UART0_FR ((volatile uint32_t *)(UART0_BASE + 0x18))
#define UART0_IBRD ((volatile uint32_t *)(UART0_BASE + 0x24))
#define UART0_FBRD ((volatile uint32_t *)(UART0_BASE + 0x28))
#define UART0_LCRH ((volatile uint32_t *)(UART0_BASE + 0x2C))
#define UART0_CR ((volatile uint32_t *)(UART0_BASE + 0x30))
#define UART0_IMSC ((volatile uint32_t *)(UART0_BASE + 0x38))
#define UART0_ICR ((volatile uint32_t *)(UART0_BASE + 0x44))

// **新增 UART1 (Mini UART)**
#define UART1_BASE (PERIPHERAL_BASE + 0x215000)
#define UART1_DR ((volatile uint32_t *)(UART1_BASE + 0x00))
#define UART1_FR ((volatile uint32_t *)(UART1_BASE + 0x18))
#define UART1_IBRD ((volatile uint32_t *)(UART1_BASE + 0x24))
#define UART1_FBRD ((volatile uint32_t *)(UART1_BASE + 0x28))
#define UART1_LCRH ((volatile uint32_t *)(UART1_BASE + 0x2C))
#define UART1_CR ((volatile uint32_t *)(UART1_BASE + 0x30))
#define UART1_IMSC ((volatile uint32_t *)(UART1_BASE + 0x38))
#define UART1_ICR ((volatile uint32_t *)(UART1_BASE + 0x44))

// UART flags
#define UART_FR_TXFF (1 << 5) // 发送 FIFO 满
#define UART_FR_RXFE (1 << 4) // 接收 FIFO 空

// UART control bits
#define UART_CR_UARTEN (1 << 0) // 启用 UART
#define UART_CR_TXE (1 << 8)    // 启用 TX
#define UART_CR_RXE (1 << 9)    // 启用 RX

// UART line control bits
#define UART_LCRH_FEN (1 << 4)    // Enable FIFOs
#define UART_LCRH_WLEN_8 (3 << 5) // 8 bit word length

// Helper functions for GPIO configuration
void gpio_set_function(unsigned int pin, unsigned int function);
void gpio_set_pull(unsigned int pin, unsigned int pull);

#endif // GPIO_H
