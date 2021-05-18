#include <stdint.h>
#include <stdbool.h>
#include <stm32wb55xx.h>
#include <stm32wb55xx/gpio.h>
#include <stm32wb55xx/lpuart.h>
#include <stm32wb55xx/rcc.h>
#include <mcu/system_timer.h>
#include <stm32wb55xx/rtc.h>
#include <stm32wb55xx/spi.h>
#include <kernel/debug/log.h>
#include <kernel/event/event_handler.h>
#include <kernel/task/task_manager.h>
#include <drivers/display/ls013b7h05.h>
#include <drivers/memory/sdcard.h>
#include <drivers/ble/bluefruit.h>
#include <libraries/string.h>
#include <libraries/time.h>
#include <applications/time_app/time_app.h>
#include <applications/menu/menu.h>
#include <applications/ATtui/ATtui.h>
//#include <applications/bootloader/bootloader.h>
#include "terminal.h"
#include <key_map.h>

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

error_t initialize_display()
{
    error_t error;
    ls013b7h05_configuration_t display_device;

    error = gpio_configure_pin((gpio_configuration_t)
                               {.port = GPIOB,
                                .pin = 7,
                                .mode = GPIO_MODE_OUTPUT,
                                .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                                .output_speed = GPIO_OUTPUT_SPEED_LOW,
                                .pull_resistor = GPIO_PULL_RESISTOR_NONE},
                               &display_device.cs_pin_handle);
    if (error) {
        log_error(error, "Failed to configure cs pin for display");
        while(1);
    }

    error = spi_device_init((spi_device_configuration_t)
                            {.spi = SPI1,
                             .clock_mode = SPI_CLOCK_MODE_0,
                             .mode = SPI_MODE_MASTER,
                             .baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_4,
                             .significant_bit = SPI_SIGNIFICANT_BIT_MSB,
                             .com_mode =  SPI_COM_MODE_SIMPLEX,
                             .data_size = SPI_DATA_SIZE_8BIT},
                            &display_device.spi_handle);
    if (error) {
        log_error(error, "Failed to initialize spi device for display");
        while(1);
    }

    error = ls013b7h05_init(display_device);
    if (error) {
        log_error(error, "Failed to initialize display");
        while(1);
    }

    log_debug("Kernel initialized display");
    return SUCCESS;
}

error_t initialize_bluefruit_device()
{
    error_t error;
    bluefruit_configuration_t bluefruit_device;

    // Configure CS pin
    error = gpio_configure_pin((gpio_configuration_t)
                               {.port = GPIOA,
                                .pin = 15,
                                .mode = GPIO_MODE_OUTPUT,
                                .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                                .output_speed = GPIO_OUTPUT_SPEED_LOW,
                                .pull_resistor = GPIO_PULL_RESISTOR_NONE},
                               &bluefruit_device.cs_pin_handle);
    if (error) {
        log_error(error, "Failed to configure cs pin for bluefruit device");
        while(1);
    }

    // Configure IRQ pin
    error = gpio_configure_pin((gpio_configuration_t)
                               {.port = GPIOA,
                                .pin = 14,
                                .mode = GPIO_MODE_INPUT,
                                .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                                .output_speed = GPIO_OUTPUT_SPEED_LOW,
                                .pull_resistor = GPIO_PULL_RESISTOR_DOWN},
                               &bluefruit_device.irq_pin_handle);
    if (error) {
        log_error(error, "Failed to configure irq pin for bluefruit device");
        while(1);
    }

    // Initialize SPI device
    error = spi_device_init((spi_device_configuration_t)
                            {.spi =SPI1,
                             .clock_mode = SPI_CLOCK_MODE_0,
                             .mode = SPI_MODE_MASTER,
                             .baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_8,
                             .significant_bit = SPI_SIGNIFICANT_BIT_MSB,
                             .com_mode =  SPI_COM_MODE_FULL_DUPLEX,
                             .data_size = SPI_DATA_SIZE_8BIT},
                            &bluefruit_device.spi_handle);
    if (error) {
        log_error(error, "Failed to configure spi device for bluefruit device");
        while(1);
    }

    error = bluefruit_init(bluefruit_device);
    if (error) {
        log_error(error, "Failed to initialize bluefruit");
        while(1);
    }

    log_debug("Kernel initialized bluefruit");
    return SUCCESS;
}

error_t initialize_input_events()
{
    error_t error;
    gpio_handle_t handle;

    error = gpio_configure_pin((gpio_configuration_t)
                               {.port = GPIOB,
                                .pin = 5,
                                .mode = GPIO_MODE_INPUT,
                                .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                                .output_speed = GPIO_OUTPUT_SPEED_LOW,
                                .pull_resistor = GPIO_PULL_RESISTOR_UP},
                               &handle);
    if (error) {
        log_error(error, "Failed to configure UP BUTTON pin");
        return error;
    }
    event_handler_add_event_source(ID_BUTTON_UP, handle);

    error = gpio_configure_pin((gpio_configuration_t)
                               {.port = GPIOB,
                                .pin = 4,
                                .mode = GPIO_MODE_INPUT,
                                .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                                .output_speed = GPIO_OUTPUT_SPEED_LOW,
                                .pull_resistor = GPIO_PULL_RESISTOR_UP},
                               &handle);
    if (error) {
        log_error(error, "Failed to configure UP BUTTON pin");
        return error;
    }
    event_handler_add_event_source(ID_BUTTON_SELECT, handle);

    error = gpio_configure_pin((gpio_configuration_t)
                               {.port = GPIOB,
                                .pin = 3,
                                .mode = GPIO_MODE_INPUT,
                                .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                                .output_speed = GPIO_OUTPUT_SPEED_LOW,
                                .pull_resistor = GPIO_PULL_RESISTOR_UP},
                               &handle);
    if (error) {
        log_error(error, "Failed to configure UP BUTTON pin");
        return error;
    }
    event_handler_add_event_source(ID_BUTTON_DOWN, handle);

    return SUCCESS;
}

void main()
{
    if (*((unsigned long *)_bootloader_magic) == 0x10ADB007) {
        *((unsigned long *)_bootloader_magic) = 0x00000000;
        jump_to_bootloader();
    }

    error_t error;
    lpuart_handle_t lpuart_handle;

    __enable_irq();

    NVIC_EnableIRQ(SysTick_IRQn);
    RCC->CIER |= 1 << 2;

    rcc_enable_lpuart1_clock();
    rcc_enable_spi1_clock();
    rcc_enable_gpioa_clock();
    rcc_enable_gpiob_clock();

    // Initialize LPUART interface
    lpuart_init((lpuart_configuration_t)
                {.lpuart = LPUART1,
                 .rx_port = GPIOA,
                 .rx_pin = 3,
                 .tx_port = GPIOA,
                 .tx_pin = 2,
                 .clock_source = RCC_LPUART_CLOCK_SOURCE_SYSCLK,
                 .word_length = LPUART_WORD_LENGTH_8,
                 .baud_rate_prescaler = 0x8ae3,
                 .stop_bits = LPUART_STOP_BITS_1},
                &lpuart_handle);

    // Initialize log
    log_init((log_configuration_t)
             {.lpuart_handle = lpuart_handle});

    log_debug("Kernel Start");

    // Initialize SPI interface
    error = spi_interface_init((spi_interface_configuration_t)
                               {.sck_port = GPIOA,
                                .sck_pin = 5,
                                .miso_port = GPIOA,
                                .miso_pin = 6,
                                .mosi_port = GPIOA,
                                .mosi_pin = 7});
    if (error) {
        log_error(error, "Failed to initialize spi interface");
        while(1);
    }

    log_debug("Kernel initialized SPI");

    // Initialize RTC interface
    error = rtc_init();
    if (error) {
        log_error(error, "Failed to initialize RTC");
        while(1);
    }

    log_debug("Kernel initialized RTC interface");


    // Initialize SysTick Timer
    system_timer_init();
    system_timer_start();
    log_debug("Kernel initialized system timer");

    initialize_display();
    initialize_bluefruit_device();

    // Initialize Event Handler
    event_handler_init();
    log_debug("Kernel initialized event handler");
    initialize_input_events();

    // Initialize Terminal
    terminal_init((terminal_configuration_t)
                  {.width = 18,
                   .height = 10});
    log_debug("Kernel initialized terminal");

    // Initialize Task Manager
    time_app_data_t time_app_data;
    menu_app_data_t menu_app_data;
    ATtui_app_data_t ATtui_app_data;

    task_manager_init();
    log_debug("Kernel initialized task manager");
    task_manager_add_task(string("Time"), &time_application_start, &time_app_data);
    task_manager_add_task(string("Set Time"), &set_time_application_start, &time_app_data);
    task_manager_add_task(string("Menu"), &menu_application_start, &menu_app_data);
    task_manager_add_task(string("ATtui"), &ATtui_application_start, &ATtui_app_data);
    //task_manager_add_task(string("Bootloader"), &bootloader_application_start, 0);

    // Run task
    task_manager_start_task_by_name(string("Time"));
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
