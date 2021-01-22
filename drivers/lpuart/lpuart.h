#ifndef LPUART_H
#define LPUART_H

#include <stdint.h>
#include <stdbool.h>
#include <rcc.h>

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
    bool is_open;
    rcc_lpuart_clock_source_t clock_source;
    lpuart_word_length_t word_length;
    uint32_t baud_rate_prescaler;
    lpuart_stop_bits_t stop_bits;
} lpuart_configuration_t;

void lpuart_read(int32_t lpuart_handle, void *buffer, uint32_t length);
void lpuart_write(int32_t lpuart_handle, void *data, uint32_t length);
int32_t lpuart_open(lpuart_configuration_t config);
void lpuart_close(int32_t lpuart_handle);
void lpuart_init();
void lpuart_destroy();

#endif
