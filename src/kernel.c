#include "uart.h"

// Kernel entry point
void kernel_main(void) {
    // Initialize UART
    uart_init();
    
    // Hello world
    uart_puts("Raspberry Pi 4 Bare Metal OS\n");
    uart_puts("UART initialized successfully\n");
    
    // Simple counter demonstration
    uart_puts("Starting counter...\n");
    
    for (int i = 0; i < 10; i++) {
        uart_printf("Counter: %d (0x%x)\n", i, i);//展示格式化
        
        // Simple delay
        for (volatile int j = 0; j < 1000000; j++) {
            asm volatile("nop"); //执行空操作来消耗时间
        }
    }
    
    uart_puts("Counter finished.\n");
    uart_puts("Welcome to your bare-metal Raspberry Pi 4!\n");
    
    // Kernel never returns, just loop
    while (1) {
        asm volatile("wfe");死循环
        //asm volatile在C语言中嵌入汇编语言
    }
}
