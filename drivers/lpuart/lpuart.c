#include "lpuart.h"
#include <stm32wb55xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define LPUART_DEVICE_MAX 8

lpuart_configuration_t g_lpuart_devices[LPUART_DEVICE_MAX];
int32_t g_current_lpuart_config = -1;

static void lpuart_send_byte(uint8_t data)
{
    LPUART1->TDR = (uint32_t)data;
    while (!(LPUART1->ISR & USART_ISR_TC)) {}
}

static void lpuart_configure(int32_t lpuart_handle)
{
    if (lpuart_handle < 0 ||
        lpuart_handle >= LPUART_DEVICE_MAX ||
        !g_lpuart_devices[lpuart_handle].is_open) {
        return;
    }

    lpuart_configuration_t *lpuart_dev = &g_lpuart_devices[lpuart_handle];
    
    g_current_lpuart_config = lpuart_handle;
    RCC->CCIPR &= ~(1 << RCC_CCIPR_LPUART1SEL_Pos);
    RCC->CCIPR |= lpuart_dev->clock_source << RCC_CCIPR_LPUART1SEL_Pos;
    LPUART1->CR1 |= lpuart_dev->word_length << USART_CR1_M1_Pos;
    LPUART1->BRR = lpuart_dev->baud_rate_prescaler;
}

static int32_t lpuart_find_free_device()
{
    bool found_free_device = false;
    int32_t lpuart_handle;

    for (lpuart_handle = 0; lpuart_handle < LPUART_DEVICE_MAX; lpuart_handle++) {
        if (g_lpuart_devices[lpuart_handle].is_open) {
            continue;
        }

        found_free_device = true;
        break;
    }

    if (found_free_device) {
        return lpuart_handle;
    }

    return -1;
}

void lpuart_read(int32_t lpuart_handle, void *buffer, uint32_t length)
{
    // TODO
    return;
}

void lpuart_write(int32_t lpuart_handle, void *data, uint32_t length)
{
    if (lpuart_handle >= LPUART_DEVICE_MAX ||
        lpuart_handle < 0 ||
        !g_lpuart_devices[lpuart_handle].is_open) {
        return;
    }
    
    if (g_current_lpuart_config != lpuart_handle) {
        lpuart_configure(lpuart_handle);
    }
    
    uint8_t *d = data;
    while (length--) {
        lpuart_send_byte(*d++);
    }
}

int32_t lpuart_open(lpuart_configuration_t config)
{
    int32_t lpuart_handle = lpuart_find_free_device();
    if (lpuart_handle < 0) {
        return -1;
    }

    if (config.baud_rate_prescaler < 0x300 ||
        config.baud_rate_prescaler > 0xfffff) {
        return -2;
    }

    memcpy(&g_lpuart_devices[lpuart_handle], &config, sizeof(lpuart_configuration_t));    
    g_lpuart_devices[lpuart_handle].is_open = true;
    lpuart_configure(lpuart_handle);
    return lpuart_handle;
}

void lpuart_close(int32_t lpuart_handle)
{
    if (lpuart_handle < 0 ||
        lpuart_handle >= LPUART_DEVICE_MAX) {
        return;
    }

    g_lpuart_devices[lpuart_handle].is_open = false;
}

void lpuart_init()
{
    RCC->APB1ENR2 |= 1;

    // Enable LPUART
    LPUART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void lpuart_destroy()
{
    // TODO
    return;
}
