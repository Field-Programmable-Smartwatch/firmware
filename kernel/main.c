#include <config.h>
#include <mcu/stm32wb55xx/stm32wb55xx.h>
#include <mcu/stm32wb55xx/gpio.h>
#include <mcu/stm32wb55xx/spi.h>
#include <mcu/stm32wb55xx/lpuart.h>
#include <mcu/stm32wb55xx/i2c.h>
#include <mcu/system_timer.h>
#include <kernel/task/task_manager.h>
#include <kernel/debug/log.h>
#include <libraries/string.h>
#include <kernel/tasks/app_task.h>
#include <kernel/tasks/input_task.h>

extern uint32_t _estack[];
extern uint32_t _sidata[];
extern uint32_t _sdata[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];
extern uint32_t interrupt_vector_table[];

static lpuart_handle_t g_lpuart_handle;
static spi_interface_handle_t g_spi_interface_handle;

static error_t init_mcu()
{
    error_t error;

    // Enable necessary peripheral clocks
    rcc_enable_lpuart1_clock();
    rcc_enable_spi1_clock();
    rcc_enable_i2c1_clock();
    rcc_enable_gpioa_clock();
    rcc_enable_gpiob_clock();
    rcc_enable_dma1_clock();
    rcc_enable_dmamux1_clock();

    // Initialize LPUART interface
    lpuart_init((lpuart_configuration_t)
                {.lpuart = LPUART1,
                 .rx_port = GPIOA,
                 .rx_pin = 3,
                 .tx_port = GPIOA,
                 .tx_pin = 2,
                 .clock_source = RCC_LPUART_CLOCK_SOURCE_SYSCLK,
                 .word_length = LPUART_WORD_LENGTH_8,
                 .baud_rate_prescaler = 0x22b8e,
                 .stop_bits = LPUART_STOP_BITS_1},
                &g_lpuart_handle);

    // Initialize I2C interface
    error = i2c_interface_init((i2c_interface_configuration_t)
                               {.i2c = I2C1,
                                .scl_port = GPIOB,
                                .scl_pin = 6,
                                .sda_port = GPIOB,
                                .sda_pin = 7,
                                .timing = 1234});
    if (error) {
        return error;
    }

    // Initialize SPI interface
    error = spi_interface_init((spi_interface_configuration_t)
                               {.sck_port = GPIOA,
                                .sck_pin = 5,
                                .miso_port = GPIOA,
                                .miso_pin = 6,
                                .mosi_port = GPIOA,
                                .mosi_pin = 7},
                               &g_spi_interface_handle);
    if (error) {
        return error;
    }

    return SUCCESS;
}

static error_t init_log()
{
    log_init((log_configuration_t)
             {.lpuart_handle = g_lpuart_handle});
    return SUCCESS;
}

static error_t init_tasks()
{
    error_t error;

    error = task_manager_init();
    if (error) {
        return error;
    }

    error = init_log();
    if (error) {
        return error;
    }

    error = input_task_init();
    if (error) {
        return error;
    }

    error = app_task_init();
    if (error) {
        return error;
    }

    return SUCCESS;
}

void main()
{
    error_t error;

    error = init_mcu();
    if (error) {
        while(1);
    }

    error = init_tasks();
    if (error) {
        while(1);
    }

    log_info("TEST %u", 1);

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
    rcc_set_pll_clock_speed(CONFIG_CLOCK_FREQ);
    rcc_set_system_clock_source(RCC_SYSTEM_CLOCK_SOURCE_PLL);
    rcc_disable_interrupts();

    main();
}

void HardFault_Handler()
{
    log_error(ERROR_FAULT, "HARD FAULT");
    while(1);
}

void BusFault_Handler()
{
    log_error(ERROR_FAULT, "BUS FAULT");
    while(1);
}

void UsageFault_Handler()
{
    log_error(ERROR_FAULT, "USEAGE FAULT");
    while(1);
}
