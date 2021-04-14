#include <stm32wb55xx.h>
#include <log.h>
#include <lpuart.h>
#include <gpio.h>
#include <stdarg.h>
#include <string.h>
#include <rcc.h>
#include <error.h>

#ifdef LOG_LEVEL
log_level_t g_log_level = LOG_LEVEL;
#else
log_level_t g_log_level = LOG_LEVEL_INFO;
#endif

int32_t g_lpuart_handle = -1;

static void log_print(char *format, va_list ap)
{
    if (!format) {
        return;
    }
    
    char msg_data[LOG_MSG_MAX_LENGTH];
    string_t msg = string_init(msg_data, 0, LOG_MSG_MAX_LENGTH);
    string_t format_string = string(format);

    if (msg.error || format_string.error) {
        return;
    }

    string_format(&msg, format_string, ap);

    lpuart_write(g_lpuart_handle, string_data(msg), string_size(msg));
}

void log(log_level_t log_level, char *format, ...)
{
    if (g_log_level < log_level || !format) {
        return;
    }
    va_list ap;
    va_start(ap, format);
    log_print(format, ap);
    va_end(ap);
}

void log_error(error_t error, char *format, ...)
{
    if (g_log_level < LOG_LEVEL_ERROR || !format) {
        return;
    }

    va_list ap;
    
    log(LOG_LEVEL_ERROR, "ERROR %s [%s]: ", string((char *)__func__), string((char *)error_get_message(error)));
    va_start(ap, format);
    log_print(format, ap);
    va_end(ap);
    log(LOG_LEVEL_ERROR, "\r\n");
}

void log_info(char *format, ...)
{
    if (g_log_level < LOG_LEVEL_INFO || !format) {
        return;
    }

    va_list ap;
    
    log(LOG_LEVEL_INFO, "INFO %s: ", __func__);
    va_start(ap, format);
    log_print(format, ap);
    va_end(ap);
    log(LOG_LEVEL_INFO, "\r\n");
}

void log_debug(char *format, ...)
{
    if (g_log_level < LOG_LEVEL_DEBUG || !format) {
        return;
    }

    va_list ap;

    log(LOG_LEVEL_DEBUG, "DEBUG %s: ", __func__);
    va_start(ap, format);
    log_print(format, ap);
    va_end(ap);
    log(LOG_LEVEL_DEBUG, "\r\n");
}

uint8_t log_wait_for_input()
{
    while (lpuart_rx_empty());
    return lpuart_read(g_lpuart_handle);
}

void log_init()
{
    gpio_configuration_t tx_pin_config;
    gpio_configuration_t rx_pin_config;
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

    rx_pin_config.port = GPIOA;
    rx_pin_config.pin = 3;
    rx_pin_config.mode = GPIO_MODE_ALT_FUNC;
    rx_pin_config.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    rx_pin_config.output_speed = GPIO_OUTPUT_SPEED_FAST;
    rx_pin_config.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    rx_pin_config.alternative_function = 8;
    gpio_configure_pin(rx_pin_config);

    lpuart_config.clock_source = RCC_LPUART_CLOCK_SOURCE_SYSCLK;
    lpuart_config.word_length = LPUART_WORD_LENGTH_8;
    lpuart_config.baud_rate_prescaler = 0x8ae3;
    lpuart_config.stop_bits = LPUART_STOP_BITS_1;
    g_lpuart_handle = lpuart_open(lpuart_config);
}
