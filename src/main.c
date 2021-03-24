#include <stm32wb55xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <terminal.h>
#include <gpio.h>
#include <lpuart.h>
#include <log.h>
#include <spi.h>
#include <display.h>
#include <systick_timer.h>
#include <event_handler.h>
#include <time.h>
#include <rcc.h>
#include <task_manager.h>
#include <sdcard.h>
#include <rtc.h>
#include <bluefruit.h>

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

    error_t error;
    terminal_configuration_t terminal;

    __enable_irq();
    NVIC_EnableIRQ(SysTick_IRQn);
    RCC->CIER |= 1 << 2;

    lpuart_init();
    log_init();
    log_debug("Kernel Start");
    
    error = spi_init();
    if (error) {
        log_error(error, "Failed to initialize SPI");
    }
    log_debug("Kernel initialized SPI");
    
    systick_timer_init();
    systick_timer_start();
    log_debug("Kernel initialized systick timer");
    
    error = display_init();
    if (error) {
        log_error(error, "Failed to initialize display");
    }
    log_debug("Kernel initialized display");
    
    error = sdcard_init();
    if (error) {
        log_error(error, "Failed to initialize SDcard");
    }
    log_debug("Kernel initialized SDcard");
    
    error = bluefruit_init();
    if (error) {
        log_error(error, "Failed to initialize bluefruit");
    }
    log_debug("Kernel initialized bluefruit");
    
    event_handler_init();
    log_debug("Kernel initialized event handler");
    
    error = rtc_init();
    if (error) {
        log_error(error, "Failed to initialize RTC");
    }
    log_debug("Kernel initialized RTC");

    terminal.width = 18;
    terminal.height = 10;
    terminal_init(terminal);
    log_debug("Kernel initialized terminal");
    
    task_manager_init();
    log_debug("Kernel initialized task manager");
    log_debug("Kernel starting task");
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
    log_error(ERROR_FAULT, "HARD FAULT");
    NVIC_SystemReset();
}

void BusFault_Handler()
{
    log_error(ERROR_FAULT, "BUS FAULT");
    NVIC_SystemReset();
}

void UsageFault_Handler()
{
    log_error(ERROR_FAULT, "USEAGE FAULT");
    NVIC_SystemReset();
}
