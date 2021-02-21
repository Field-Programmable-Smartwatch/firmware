#include <stm32wb55xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <terminal.h>
#include <gpio.h>
#include <lpuart.h>
#include <debug.h>
#include <spi.h>
#include <display.h>
#include <systick_timer.h>
#include <event_handler.h>
#include <time.h>
#include <rcc.h>
#include <task_manager.h>
#include <sdcard.h>
#include <rtc.h>

extern uint32_t _bootloader_magic[];
extern uint32_t _sidata[];
extern uint32_t _sdata[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];
extern uint32_t interrupt_vector_table[];

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
    if (*((unsigned long *)_bootloader_magic) == 0x10ADB007) {
        *((unsigned long *)_bootloader_magic) = 0x00000000;
        jump_to_bootloader();
    }

    terminal_configuration_t terminal;

    __enable_irq();
    NVIC_EnableIRQ(SysTick_IRQn);
    RCC->CIER |= 1 << 2;

    lpuart_init();
    debug_init();
    
    spi_init();

    systick_timer_init();
    systick_timer_start();
    
    display_init();
    sdcard_init();
    event_handler_init();

    rtc_init();

    terminal.width = 18;
    terminal.height = 10;
    terminal_init(terminal);
    
    task_manager_init();
    task_manager_start();
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
    rcc_reset();
    rcc_set_msi_clock_speed(RCC_MSI_CLOCK_SPEED_16MHz);
    rcc_set_system_clock_source(RCC_SYSTEM_CLOCK_SOURCE_MSI);
    rcc_disable_interrupts();

    main();
}
void HardFault_Handler()
{
    debug_print("KERNEL HARD FAULT\r\n");
    NVIC_SystemReset();
}

void BusFault_Handler()
{
    debug_print("KERNEL BUS FAULT\r\n");
    NVIC_SystemReset();
}

void UsageFault_Handler()
{
    debug_print("KERNEL USAGE FAULT\r\n");
    NVIC_SystemReset();    
}
