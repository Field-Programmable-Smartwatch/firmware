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

extern uint32_t _sidata[];
extern uint32_t _sdata[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];
extern uint32_t interrupt_vector_table[];

uint8_t button_state = 0;

void jump_to_bootloader()
{
    void (*system_memory_jump)(void);

    volatile uint32_t addr = 0x1FFF0000;

    // Set MSI Clock as the System Clock
    RCC->CR |= RCC_CR_MSION;

    // Reset Clock Configuration
    RCC->CFGR = 0x00070000;
    RCC->CR &= 0xFAF6FEFB;
    RCC->CSR &= 0xFFFFFFFA;
    RCC->CRRCR &= 0xFFFFFFFE;
    RCC->PLLCFGR = 0x22041000;
    RCC->PLLSAI1CFGR = 0x22041000;

    // Disable Interrupts
    RCC->CIER = 0x00000000;

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    SYSCFG->MEMRMP = 0x01;
    system_memory_jump = (void (*)(void)) (*((uint32_t *)(addr + 4)));
    __set_MSP(*(uint32_t *)addr);

    system_memory_jump();
}

void configure()
{
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
}

void main()
{
    if (*((unsigned long *)0x20003FF0) == 0xDEADBEEF) {
        *((unsigned long *)0x20003FF0) = 0xDEADB000;
        jump_to_bootloader();
    }

    configure();
    display_clear();
    terminal_print_at(6, 3, "Monday");
    terminal_print_at(5, 4, "January 2");
    terminal_print_at(6, 5, "12:00AM");
    terminal_set_cursor(0, 0);
    display_render();

    event_queue_t event_queue;

    while (1) {
        asm("wfi");
        event_queue = event_handler_poll();
        if (!event_queue.length) {
            continue;
        }
        for (uint8_t i = 0; i < event_queue.length; i++) {
            event_t event = event_queue.events[i];
            if (event.type == EVENT_TYPE_POS_EDGE) {
                if (event.id == ID_BUTTON_UP) {
                    debug_print("UP!\r\n");
                    terminal_print_at(0, 0, "UP    ");
                }

                if (event.id == ID_BUTTON_SELECT) {
                    debug_print("SELECT!\r\n");
                    terminal_print_at(0, 0, "SELECT");
                }

                if (event.id == ID_BUTTON_DOWN) {
                    debug_print("DOWN!\r\n");
                    terminal_print_at(0, 0, "DOWN  ");
                }

                if (event.id == ID_BUTTON_LOAD_BOOTLOADER) {
                    debug_print("LOADING BOOTLOADER!\r\n");
                    display_clear();
                    terminal_print_at(0, 4, "LOADING BOOTLOADER");
                    display_render();
                    *((unsigned long *)0x20003FF0) = 0xDEADBEEF;
                    NVIC_SystemReset();
                }
            }
        }
        display_render();
    }
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

    // Set MSI Clock as the System Clock
    RCC->CR |= RCC_CR_MSION;

    // Reset Clock Configuration
    RCC->CFGR = 0x00070000;
    RCC->CR &= 0xFAF6FEFB;
    RCC->CSR &= 0xFFFFFFFA;
    RCC->CRRCR &= 0xFFFFFFFE;
    RCC->PLLCFGR = 0x22041000;
    RCC->PLLSAI1CFGR = 0x22041000;

    // Disable Interrupts
    RCC->CIER = 0x00000000;
    main();
}
