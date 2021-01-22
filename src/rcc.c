#include <stm32wb55xx.h>
#include <stdint.h>
#include <rcc.h>

void rcc_reset()
{
    RCC->CFGR = 0x00070000;
    RCC->CR &= 0xFAF6FEFB;
    RCC->CSR &= 0xFFFFFFFA;
    RCC->CRRCR &= 0xFFFFFFFE;
    RCC->PLLCFGR = 0x22041000;
    RCC->PLLSAI1CFGR = 0x22041000;
    RCC->CIER = 0x00000000;
    RCC->SMPSCR = 0x00000301;
    RCC->AHB1ENR = 0x00000000;
    RCC->AHB2ENR = 0x00000000;
    RCC->AHB3ENR = 0x02080000;
    RCC->APB1ENR1 = 0x00000400;
    RCC->APB1ENR2 = 0x00000000;
    RCC->APB2ENR = 0x00000000;
    RCC->AHB1SMENR = 0x00011207;
    RCC->AHB2SMENR = 0x0001209F;
    RCC->AHB3SMENR = 0x03070100;
    RCC->APB1SMENR1 = 0x85A04E01;
    RCC->APB1SMENR2 = 0x00000021;
    RCC->APB2SMENR = 0x00265800;
    RCC->CCIPR = 0x00000000;
}

void rcc_set_msi_clock_speed(rcc_msi_clock_speed_t msi_clock_speed)
{
    rcc_disable_msi_clock();
    RCC->CR &= ~(RCC_CR_MSIRANGE);
    RCC->CR |= msi_clock_speed << RCC_CR_MSIRANGE_Pos;
}

void rcc_set_system_clock_source(rcc_system_clock_source_t system_clock_source)
{
    RCC->CFGR &= ~(RCC_CFGR_SW);
    RCC->CFGR |= (system_clock_source << RCC_CFGR_SW_Pos);
}

void rcc_set_lpuart1_clock_source(rcc_lpuart_clock_source_t lpuart_clock_source)
{
    RCC->CCIPR &= ~(RCC_CCIPR_LPUART1SEL);
    RCC->CCIPR |= lpuart_clock_source << RCC_CCIPR_LPUART1SEL_Pos;
}

uint32_t g_msi_clock_speeds[] =
    {100000, 200000, 400000, 800000,
     1000000, 2000000, 4000000, 8000000,
     16000000, 24000000, 32000000, 48000000};

uint32_t rcc_get_system_clock_speed()
{
    uint32_t system_clock_speed = 0;
    if ((RCC->CFGR & RCC_CFGR_SW) == RCC_SYSTEM_CLOCK_SOURCE_MSI) {
        system_clock_speed = g_msi_clock_speeds[(RCC->CR & RCC_CR_MSIRANGE) >> RCC_CR_MSIRANGE_Pos];
        
    } else if ((RCC->CFGR & RCC_CFGR_SW) == RCC_SYSTEM_CLOCK_SOURCE_HSI16) {
        system_clock_speed = 16000000;
        
    } else if ((RCC->CFGR & RCC_CFGR_SW) == RCC_SYSTEM_CLOCK_SOURCE_HSE) {
        system_clock_speed = 32000000;
        
    } else if ((RCC->CFGR & RCC_CFGR_SW) == RCC_SYSTEM_CLOCK_SOURCE_PLL) {
        // TODO
        system_clock_speed = 0;
    }
    return system_clock_speed;
}

void rcc_enable_msi_clock()
{
    RCC->CR |= RCC_CR_MSION;
}

void rcc_disable_msi_clock()
{
    RCC->CR &= ~RCC_CR_MSION;
}

void rcc_enable_gpioa_clock()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
}

void rcc_enable_gpiob_clock()
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
}

void rcc_enable_lpuart1_clock()
{
    RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;
}

void rcc_enable_spi1_clock()
{
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
}

void rcc_disable_interrupts()
{
    RCC->CIER = 0x00000000;
}
