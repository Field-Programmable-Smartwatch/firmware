#include "lpuart.h"
#include <stm32wb55xx.h>

void lpuart_send_byte(uint8_t data)
{
    LPUART1->TDR = (uint32_t)data;
    while (!(LPUART1->ISR & USART_ISR_TC)) {}
}

void lpuart_send_bytes(void *data, uint32_t length)
{
    uint8_t *d = data;
    while (length--) {
        lpuart_send_byte(*d++);
    }
}

void lpuart_init(lpuart_configuration_t config)
{
    // Set clock source TODO: move this out of here...
    RCC->CCIPR &= ~(1 << RCC_CCIPR_LPUART1SEL_Pos);
    RCC->CCIPR |= config.clock_source << RCC_CCIPR_LPUART1SEL_Pos;
    RCC->APB1ENR2 |= 1;

    // Set word length
    LPUART1->CR1 |= config.word_length << USART_CR1_M1_Pos;

    // Set baud rate prescaler
    if (config.baud_rate_prescaler < 0x300) {
        config.baud_rate_prescaler = 0x300;
    }
    // Ensure prescaler isnt greater than 20 bits
    config.baud_rate_prescaler &= ~(0xfff00000);
    LPUART1->BRR = config.baud_rate_prescaler;

    // Enable LPUART
    LPUART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

