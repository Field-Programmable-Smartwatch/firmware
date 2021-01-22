#include <stm32wb55xx.h>
#include "debug.h"
#include <lpuart.h>
#include <gpio.h>
#include <stdarg.h>
#include <string.h>
#include <rcc.h>

int32_t g_lpuart_handle = -1;

void debug_init()
{
    gpio_configuration_t tx_pin_config;
    lpuart_configuration_t lpuart_config;

    rcc_enable_gpioa_clock();

    tx_pin_config.port = GPIOA;
    tx_pin_config.pin = 2;
    tx_pin_config.mode = GPIO_MODE_ALT_FUNC;
    tx_pin_config.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    tx_pin_config.output_speed = GPIO_OUTPUT_SPEED_FAST;
    tx_pin_config.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    tx_pin_config.alternative_function = 8;
    gpio_configure_pin(tx_pin_config);

    lpuart_config.clock_source = RCC_LPUART_CLOCK_SOURCE_SYSCLK;
    lpuart_config.word_length = LPUART_WORD_LENGTH_8;
    lpuart_config.baud_rate_prescaler = 0x8ae3;
    lpuart_config.stop_bits = LPUART_STOP_BITS_1;
    g_lpuart_handle = lpuart_open(lpuart_config);
}

void debug_print(char *format, ...)
{
    va_list ap;
    char msg[DEBUG_MSG_MAX_LENGTH];
    uint32_t msg_length = 0;
    
    va_start(ap, format);
    string_format(format, ap, msg, DEBUG_MSG_MAX_LENGTH);
    va_end(ap);

    msg_length = strlen(msg);
    lpuart_write(g_lpuart_handle, msg, msg_length);
}
