#include <stm32wb55xx.h>
#include "debug.h"
#include <lpuart.h>
#include <gpio.h>
#include <stdarg.h>
#include <string.h>


uint32_t uint_to_str(char *dest, uint32_t u, const uint32_t size)
{
    if (size == 0) {
        return 0;
    }
    
    char str[11];
    str[10] = 0;
    uint32_t str_index = 10;
    uint32_t bytes_copied = 0;
    
    do {
        uint8_t digit = u % 10;
        str[--str_index] = digit | 48; // convert raw digit to ascii representation
        u -= digit;
        u /= 10;
    } while (u);

    while (bytes_copied < size && str[str_index]) {
        dest[bytes_copied++] = str[str_index++];
    }
    
    return bytes_copied;
}

uint32_t int_to_str(char *dest, const int32_t i, const uint32_t size)
{
    if (i > 0) {
        return uint_to_str(dest, i, size);
    }

    uint32_t u = ~(i) + 1;
    
    char str[11];
    str[10] = 0;
    uint32_t str_index = 10;
    uint32_t bytes_copied = 0;
    
    do {
        uint8_t digit = u % 10;
        str[str_index--] = digit | 48; // convert raw digit to ascii representation
        u -= digit;
        u /= 10;
    } while (u);

    str[str_index] = '-';
    while (bytes_copied < size && str[str_index]) {
        dest[bytes_copied++] = str[str_index++];
    }

    return bytes_copied;
}

void debug_init()
{
    gpio_configuration_t tx_pin_config;
    lpuart_configuration_t lpuart_config;

    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    tx_pin_config.port = GPIOA;
    tx_pin_config.pin = 2;
    tx_pin_config.mode = GPIO_MODE_ALT_FUNC;
    tx_pin_config.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    tx_pin_config.output_speed = GPIO_OUTPUT_SPEED_FAST;
    tx_pin_config.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    tx_pin_config.alternative_function = 8;
    gpio_configure_pin(tx_pin_config);

    lpuart_config.clock_source = LPUART_CLOCK_SOURCE_SYSCLK;
    lpuart_config.word_length = LPUART_WORD_LENGTH_8;
    lpuart_config.baud_rate_prescaler = 0x22b8;
    lpuart_config.stop_bits = LPUART_STOP_BITS_1;
    lpuart_init(lpuart_config);
}

void debug_print(char *format, ...)
{
    va_list ap;
    uint32_t u;
    int32_t i;
    char *s;
    int c;
    char debug_msg[DEBUG_MSG_MAX_LENGTH];
    uint32_t debug_msg_size = 0;
    uint32_t format_size = strlen(format);
    
    va_start(ap, format);
    for (uint32_t format_index = 0; format_index < format_size; format_index++) {
        char ch = format[format_index];
        if (ch == '%') {
            
            if (format[format_index+1] == 'u') {
                u = va_arg(ap, uint32_t);
                debug_msg_size += uint_to_str(&debug_msg[debug_msg_size], u, DEBUG_MSG_MAX_LENGTH - debug_msg_size);
            }
            
            if (format[format_index+1] == 'i' ||
                format[format_index+1] == 'd') {
                i = va_arg(ap, int32_t);
                debug_msg_size += int_to_str(&debug_msg[debug_msg_size], i, DEBUG_MSG_MAX_LENGTH - debug_msg_size);
            }
            
            if (format[format_index+1] == 's') {
                s = va_arg(ap, char *);
                uint32_t copy_size = strlen(s);
                if (copy_size > DEBUG_MSG_MAX_LENGTH - debug_msg_size) {
                    copy_size = DEBUG_MSG_MAX_LENGTH - debug_msg_size;
                }
                strncpy(&debug_msg[debug_msg_size], s, copy_size);
                debug_msg_size += copy_size;
            }
            
            if (format[format_index+1] == 'c') {
                c = va_arg(ap, int);
                if (DEBUG_MSG_MAX_LENGTH - debug_msg_size > 0) {
                    debug_msg[debug_msg_size++] = (char)c;
                }
            }
            
            format_index++;
            
        } else {
            debug_msg[debug_msg_size++] = ch;
        }
    }
    va_end(ap);
    debug_msg[debug_msg_size] = 0;

    lpuart_send_bytes(debug_msg, debug_msg_size);
}
