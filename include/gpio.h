#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

// Raspberry Pi 4 peripherals base address
#define PERIPHERAL_BASE 0xFE000000

// GPIO registers
#define GPIO_BASE       (PERIPHERAL_BASE + 0x200000)

// GPIO function select registers
//这些寄存器控制 GPIO 引脚的功能（输入、输出、特殊功能）
/*
每个GPIO占三位，每个寄存器占32位，所以设置IO的时候，将对应的IO设置成对应的状态，一共有三十个IO，例如我要设置18IO，就在GPFSEL1中找第24位到27位，设置成需要的模式即可。
功能模式编码一共九种不赘述*/
#define GPFSEL0         ((volatile uint32_t*)(GPIO_BASE + 0x00))
#define GPFSEL1         ((volatile uint32_t*)(GPIO_BASE + 0x04))
#define GPFSEL2         ((volatile uint32_t*)(GPIO_BASE + 0x08))

// GPIO pull-up/down registers
#define GPIO_PUP_PDN_CNTRL_REG0 ((volatile uint32_t*)(GPIO_BASE + 0xE4))
#define GPIO_PUP_PDN_CNTRL_REG1 ((volatile uint32_t*)(GPIO_BASE + 0xE8))

// UART0 (PL011) registers
#define UART0_BASE      (PERIPHERAL_BASE + 0x201000)
#define UART0_DR        ((volatile uint32_t*)(UART0_BASE + 0x00))
#define UART0_FR        ((volatile uint32_t*)(UART0_BASE + 0x18))
#define UART0_IBRD      ((volatile uint32_t*)(UART0_BASE + 0x24))
#define UART0_FBRD      ((volatile uint32_t*)(UART0_BASE + 0x28))
#define UART0_LCRH      ((volatile uint32_t*)(UART0_BASE + 0x2C))
#define UART0_CR        ((volatile uint32_t*)(UART0_BASE + 0x30))
#define UART0_IMSC      ((volatile uint32_t*)(UART0_BASE + 0x38))
#define UART0_ICR       ((volatile uint32_t*)(UART0_BASE + 0x44))

// UART flags
#define UART_FR_TXFF    (1 << 5)  // 发送 FIFO 满
#define UART_FR_RXFE    (1 << 4)  // 接收 FIFO 空

// UART control bits
#define UART_CR_UARTEN  (1 << 0)  // 启用 UART
#define UART_CR_TXE     (1 << 8)  // 启用 TX
#define UART_CR_RXE     (1 << 9)  // 启用 RX

// UART line control bits
#define UART_LCRH_FEN   (1 << 4)    // Enable FIFOs
#define UART_LCRH_WLEN_8 (3 << 5)   // 8 bit word length

// Helper functions for GPIO configuration
void gpio_set_function(unsigned int pin, unsigned int function);
void gpio_set_pull(unsigned int pin, unsigned int pull);

#endif // GPIO_H
