#include <stdint.h>
#include <stdbool.h>
#include <stm32wb55xx.h>
#include <stm32wb55xx/gpio.h>
#include <stm32wb55xx/lpuart.h>
#include <stm32wb55xx/spi.h>
#include <stm32wb55xx/rcc.h>
#include <drivers/memory/sdcard.h>
#include <kernel/debug/log.h>
#include <libraries/error.h>
#include <libraries/string.h>
#include "elf32.h"

#define KERNEL_START_ADDR 0x20002000
#define KERNEL_SIZE_IN_BLOCKS 35

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

void jump_to_kernel_main()
{
    void (*kernel_main)(void);

    volatile uint32_t addr = KERNEL_START_ADDR;
    kernel_main = (void (*)(void)) (*(uint32_t *)(KERNEL_START_ADDR+4));
    __set_MSP(*(uint32_t *)addr);
    SCB->VTOR = KERNEL_START_ADDR;
    kernel_main();
}

void load_kernel()
{
    uint8_t block[512];

    memset(block , 0, 512);
    uint8_t *addr = (uint8_t *)(KERNEL_START_ADDR);
    for (uint32_t i = 0; i < KERNEL_SIZE_IN_BLOCKS; i++) {
        sdcard_read_block(i, block);
        memcpy(addr, block, 512);
        addr += 512;
    }
}

void bootloader_main()
{
    if (*((unsigned long *)_bootloader_magic) == 0x10ADB007) {
        *((unsigned long *)_bootloader_magic) = 0x00000000;
        jump_to_bootloader();
    }

    if (*((unsigned long *)_bootloader_magic) == 0x10ADBEEF) {
        *((unsigned long *)_bootloader_magic) = 0x00000000;
        jump_to_kernel_main();
    }

    error_t error;
    lpuart_handle_t lpuart_handle;
    sdcard_configuration_t sdcard_device;

    __enable_irq();

    rcc_enable_lpuart1_clock();
    rcc_enable_spi1_clock();
    rcc_enable_gpioa_clock();
    rcc_enable_gpiob_clock();

    // Initialize lpuart interface
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

    log_debug("Bootloader Start");

    // Initialize spi interface
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

    log_debug("Bootloader initialized SPI");

    // Initialize SDcard device
    error = gpio_configure_pin((gpio_configuration_t)
                               {.port = GPIOB,
                                .pin = 6,
                                .mode = GPIO_MODE_OUTPUT,
                                .output_type = GPIO_OUTPUT_TYPE_PUSH_PULL,
                                .output_speed = GPIO_OUTPUT_SPEED_LOW,
                                .pull_resistor = GPIO_PULL_RESISTOR_NONE},
                               &sdcard_device.cs_pin_handle);
    if (error) {
        log_error(error, "Failed to configure CS pin for SDcard");
        while(1);
    }
    error = spi_device_init((spi_device_configuration_t)
                            {.spi = SPI1,
                             .clock_mode = SPI_CLOCK_MODE_0,
                             .mode = SPI_MODE_MASTER,
                             .baud_rate_prescaler = SPI_BAUD_RATE_PRESCALER_64,
                             .significant_bit = SPI_SIGNIFICANT_BIT_MSB,
                             .com_mode =  SPI_COM_MODE_FULL_DUPLEX,
                             .data_size = SPI_DATA_SIZE_8BIT},
                            &sdcard_device.spi_handle);
    if (error) {
        log_error(error, "Failed to initialize SPI device for SDcard");
        while(1);
    }

    error = sdcard_init(sdcard_device);

    if (error) {
        log_error(error, "Failed to initialize sdcard\r\n");
        while(1);
    }

    log_debug("Bootloader initialized SDcard");

    elf_load();
    log_debug("Bootloader loaded kernel into RAM");
    log_debug("Bootloader jumping to kernel main");
    jump_to_kernel_main();
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

    bootloader_main();
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
