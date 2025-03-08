//两个函数实现了IO口功能选择和上/下拉控制
#include "gpio.h"

// Set GPIO pin function
void gpio_set_function(unsigned int pin, unsigned int function) {
    volatile uint32_t* reg = GPFSEL0 + (pin / 10);
    unsigned int shift = (pin % 10) * 3;
    
    uint32_t value = *reg;
    value &= ~(7 << shift);              // Clear the 3 bits for this pin
    value |= (function << shift);        // Set the new function
    *reg = value;
}

// Set GPIO pin pull-up/down state
void gpio_set_pull(unsigned int pin, unsigned int pull) {
    volatile uint32_t* reg = GPIO_PUP_PDN_CNTRL_REG0 + (pin / 16);
    unsigned int shift = (pin % 16) * 2;
    
    uint32_t value = *reg;
    value &= ~(3 << shift);              // Clear the 2 bits for this pin
    value |= (pull << shift);            // Set the new pull state
    *reg = value;
}
