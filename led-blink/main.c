#include <stm32wb55xx.h>
#include <stdint.h>


void led_turn_on()
{
    GPIOA->BSRR |= GPIO_BSRR_BS14;
}

void led_turn_off()
{
    GPIOA->BSRR |= GPIO_BSRR_BR14;
}

void init_gpio()
{
    // Enable GPIO PORT A Clock
    RCC->AHB2RSTR |= RCC_AHB2ENR_GPIOAEN;
    
    // Set Port A Pin 14 to Output
    GPIOA->MODER |= GPIO_MODER_MODE14_0;
    GPIOA->MODER &= ~GPIO_MODER_MODE14_1;
}

void main()
{   
    init_gpio();
    
    while (1) {
        led_turn_on();
        for (uint32_t i = 0; i < 4000000; i++);

        led_turn_off();
        for (uint32_t i = 0; i < 4000000; i++);        
    }
}
