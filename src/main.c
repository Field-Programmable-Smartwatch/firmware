#include <stm32wb55xx.h>
#include <stdint.h>
#include <string.h>
#include <terminal.h>
#include <gpio.h>
#include <debug.h>
#include <spi.h>
#include <display.h>

#define CHAR_WIDTH 8
#define CHAR_HEIGHT 16
#define WIDTH 144
#define HEIGHT 168

extern uint32_t _sidata[];
extern uint32_t _sdata[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];
extern uint32_t interrupt_vector_table[];

void configure()
{
    terminal_configuration_t terminal;
    
    debug_init();
    display_init();

    terminal.width = 18;
    terminal.height = 10;
    terminal_init(terminal);
}

void main()
{
    configure();
    display_clear();
    terminal_set_cursor(6, 3);
    terminal_print("Monday");
    terminal_set_cursor(5, 4);
    terminal_print("January 1");
    terminal_set_cursor(6, 5);
    terminal_print("12:00AM");
    while (1) {
        display_render();
        for (uint32_t i = 0; i < 800000; i++);        
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
