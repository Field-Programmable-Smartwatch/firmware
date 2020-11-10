#ifndef LPUART_H
#define LPUART_H

#include <stdint.h>

// TODO: this should most definitely be in an RCC driver
typedef enum lpuart_clock_source {
    LPUART_CLOCK_SOURCE_PCLK,
    LPUART_CLOCK_SOURCE_SYSCLK,
    LPUART_CLOCK_SOURCE_HSI16,
    LPUART_CLOCK_SOURCE_LSE
} lpuart_clock_source_t;

typedef enum lpuart_word_length {
    LPUART_WORD_LENGTH_8,
    LPUART_WORD_LENGTH_9,
    LPUART_WORD_LENGTH_7    
} lpuart_word_length_t;

typedef enum lpuart_stop_bits {
    LPUART_STOP_BITS_1 = 0,
    LPUART_STOP_BITS_2 = 2
} lpuart_stop_bits_t;

typedef struct lpuart_configuration {
    lpuart_clock_source_t clock_source;
    lpuart_word_length_t word_length;
    uint32_t baud_rate_prescaler;
    lpuart_stop_bits_t stop_bits;
} lpuart_configuration_t;

void lpuart_send_byte(uint8_t data);
void lpuart_send_bytes(void *data, uint32_t length);
void lpuart_init(lpuart_configuration_t config);

#endif
