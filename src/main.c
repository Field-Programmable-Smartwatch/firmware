#include <stm32wb55xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <terminal.h>
#include <gpio.h>
#include <debug.h>
#include <spi.h>
#include <display.h>
#include <systick_timer.h>
#include <event_handler.h>
#include <time.h>

extern uint32_t _sidata[];
extern uint32_t _sdata[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];
extern uint32_t interrupt_vector_table[];
bool msion = false;
bool msirdy = false;
uint32_t hz;

void jump_to_bootloader()
{
    void (*system_memory_jump)(void);

    volatile uint32_t addr = 0x1FFF0000;

    SYSCFG->MEMRMP = 0x01;
    system_memory_jump = (void (*)(void)) (*((uint32_t *)(addr + 4)));
    __set_MSP(*(uint32_t *)addr);

    system_memory_jump();
}

void main()
{
    if (*((unsigned long *)0x20003FF0) == 0x10ADB007) {
        *((unsigned long *)0x20003FF0) = 0x00000000;
        jump_to_bootloader();
    }

    terminal_configuration_t terminal;

    __enable_irq();
    NVIC_EnableIRQ(SysTick_IRQn);
    RCC->CIER |= 1 << 2;

    debug_init();
    display_init();
    systick_timer_init();
    systick_timer_start();
    event_handler_init();

    terminal.width = 18;
    terminal.height = 10;
    terminal_init(terminal);

    time_application_start();
}

void __attribute__((naked)) Reset_Handler()
{
    __asm__("ldr r0, =_estack\n\t"
            "mov sp, r0");

    // Copy data section from flash memory to ram
    uint32_t data_section_size = _edata - _sdata;
    memcpy(_sdata, _sidata, data_section_size*4);

    // Zero out bss
    uint32_t bss_section_size = _ebss - _sbss;
    memset(_sbss, 0, bss_section_size*4);

    // Set Interrupt Vector Table Offset
    SCB->VTOR = (uint32_t)interrupt_vector_table;

    // Reset Clock Configuration
    RCC->CFGR = 0x00070000;
    RCC->CR &= 0xFAF6FEFB;
    RCC->CSR &= 0xFFFFFFFA;
    RCC->CRRCR &= 0xFFFFFFFE;
    RCC->PLLCFGR = 0x22041000;
    RCC->PLLSAI1CFGR = 0x22041000;
    
    // Set MSI clock speed to 16Mhz
    RCC->CR &= ~RCC_CR_MSION;
    RCC->CR &= ~(0xF << 4);
    RCC->CR |= 8 << 4;
    
    // Set MSI Clock as the System Clock
    RCC->CR |= RCC_CR_MSION;

    // Disable Interrupts
    RCC->CIER = 0x00000000;
    main();
}
