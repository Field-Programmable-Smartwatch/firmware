#include <stm32wb55xx.h>
#include <stdint.h>

#include <gpio.h>
#include <lpuart.h>

void led_on()
{
    gpio_write(GPIOB, 7, 1);
}

void led_off()
{
    gpio_write(GPIOB, 7, 0);
}

void configure()
{
    lpuart_configuration_t lpuart_config;

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;

    gpio_configure_pin(GPIOB, 7,
                       GPIO_MODE_OUTPUT,
                       GPIO_OUTPUT_TYPE_PUSH_PULL,
                       GPIO_OUTPUT_SPEED_LOW,
                       GPIO_PULL_RESISTOR_NONE);

    gpio_configure_pin(GPIOA, 2,
                       GPIO_MODE_ALT_FUNC,
                       GPIO_OUTPUT_TYPE_PUSH_PULL,
                       GPIO_OUTPUT_SPEED_FAST,
                       GPIO_PULL_RESISTOR_NONE);
    GPIOA->AFR[0] |= 8 << GPIO_AFRL_AFSEL2_Pos;


    lpuart_config.clock_source = LPUART_CLOCK_SOURCE_SYSCLK;
    lpuart_config.word_length = LPUART_WORD_LENGTH_8;
    lpuart_config.baud_rate_prescaler = 0x22b8;
    lpuart_config.stop_bits = LPUART_STOP_BITS_1;
    lpuart_init(lpuart_config);
}

void main()
{
    configure();
    
    while (1) {
        led_on();
        lpuart_send_bytes("Hello, world!\r\n", 15);
        for (uint32_t i = 0; i < 400000; i++);
        led_off();
        for (uint32_t i = 0; i < 400000; i++);
    }
}
