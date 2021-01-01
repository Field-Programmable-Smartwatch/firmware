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
    gpio_configuration_t gpio_pb6;
    gpio_configuration_t gpio_pb5;
    gpio_configuration_t gpio_pb4;
    gpio_configuration_t gpio_pb3;

    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);

    RCC->CIER |= 1 << 2;

    debug_init();
    display_init();

    terminal.width = 18;
    terminal.height = 10;
    terminal_init(terminal);

    gpio_pb6.port = GPIOB;
    gpio_pb6.pin = 6;
    gpio_pb6.mode = GPIO_MODE_INPUT;
    gpio_pb6.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_pb6.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpio_pb6.pull_resistor = GPIO_PULL_RESISTOR_UP;
    gpio_configure_pin(gpio_pb6);

    gpio_pb5.port = GPIOB;
    gpio_pb5.pin = 5;
    gpio_pb5.mode = GPIO_MODE_INPUT;
    gpio_pb5.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_pb5.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpio_pb5.pull_resistor = GPIO_PULL_RESISTOR_UP;
    gpio_configure_pin(gpio_pb5);

    gpio_pb4.port = GPIOB;
    gpio_pb4.pin = 4;
    gpio_pb4.mode = GPIO_MODE_INPUT;
    gpio_pb4.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_pb4.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpio_pb4.pull_resistor = GPIO_PULL_RESISTOR_UP;
    gpio_configure_pin(gpio_pb4);

    gpio_pb3.port = GPIOB;
    gpio_pb3.pin = 3;
    gpio_pb3.mode = GPIO_MODE_INPUT;
    gpio_pb3.output_type = GPIO_OUTPUT_TYPE_PUSH_PULL;
    gpio_pb3.output_speed = GPIO_OUTPUT_SPEED_LOW;
    gpio_pb3.pull_resistor = GPIO_PULL_RESISTOR_UP;
    gpio_configure_pin(gpio_pb3);

    // Set EXTI sources to port B
    SYSCFG->EXTICR[0] |= (1 << SYSCFG_EXTICR1_EXTI3_Pos);
    SYSCFG->EXTICR[1] |= (1 << SYSCFG_EXTICR2_EXTI6_Pos) | (1 << SYSCFG_EXTICR2_EXTI5_Pos) | (1 << SYSCFG_EXTICR2_EXTI4_Pos);

    // Set rising edge event trigger for the interrupt
    EXTI->RTSR1 |= (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3);

    // Enable interrupts
    EXTI->IMR1 |= (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3);

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

    while (1) {
        asm("wfi");
        if (button_state == 1) {
            terminal_print_at(0, 0, "UP    ");
        }
        if (button_state == 2) {
            terminal_print_at(0, 0, "SELECT");
        }
        if (button_state == 3) {
            terminal_print_at(0, 0, "DOWN  ");
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

void delay()
{
    for (uint32_t i = 0; i < 40000; i++);
}

void EXTI9_5_IRQHandler()
{
    if (EXTI->PR1 & (1 << 6)) {
        uint8_t i1 = gpio_read(GPIOB, 6);
        delay();
        uint8_t i2 = gpio_read(GPIOB, 6);
        if (i1 == 0 && i2 == 0) {
            EXTI->PR1 = 1 << 6;
            return;
        }
        debug_print("Up Button Pressed\r\n");
        button_state = 1;
        EXTI->PR1 = 1 << 6;
    }

    if (EXTI->PR1 & (1 << 5)) {
        uint8_t i1 = gpio_read(GPIOB, 5);
        delay();
        uint8_t i2 = gpio_read(GPIOB, 5);
        if (i1 == 0 && i2 == 0) {
            EXTI->PR1 = 1 << 5;
            return;
        }
        debug_print("Select Button Pressed\r\n");
        button_state = 2;
        EXTI->PR1 = 1 << 5;
    }
}

void EXTI4_IRQHandler()
{
    if (EXTI->PR1 & (1 << 4)) {
        uint8_t i1 = gpio_read(GPIOB, 4);
        delay();
        uint8_t i2 = gpio_read(GPIOB, 4);
        if (i1 == 0 && i2 == 0) {
            EXTI->PR1 = 1 << 4;
            return;
        }
        debug_print("Down Button Pressed\r\n");
        button_state = 3;
        EXTI->PR1 = 1 << 4;
    }
}

void EXTI3_IRQHandler()
{
    if (EXTI->PR1 & (1 << 3)) {
        uint8_t i1 = gpio_read(GPIOB, 3);
        delay();
        uint8_t i2 = gpio_read(GPIOB, 3);
        if (i1 == 0 && i2 == 0) {
            EXTI->PR1 = 1 << 3;
            return;
        }
        debug_print("Load Bootloader\r\n");
        *((unsigned long *)0x20003FF0) = 0xDEADBEEF;
        display_clear();
        terminal_print_at(0, 4, "LOADING BOOTLOADER");
        display_render();
        EXTI->PR1 = 1 << 3;
        NVIC_SystemReset();
    }
}
