#include <stm32wb55xx.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <gpio.h>
#include <lpuart.h>
#include <log.h>
#include <spi.h>
#include <rcc.h>
#include <sdcard.h>
#include <elf32.h>
#include <error.h>

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

    memory_set(block , 0, 512);
    uint8_t *addr = (uint8_t *)(KERNEL_START_ADDR);
    for (uint32_t i = 0; i < KERNEL_SIZE_IN_BLOCKS; i++) {
        sdcard_read_block(i, block);
        memory_copy(addr, block, 512);
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
    
    __enable_irq();

    lpuart_init();
    log_init();
    log_debug("Bootloader Start");

    error = spi_init();
    if (error) {
        log_error(error, "Failed to initialize spi\r\n");
        while(1);
    }
    log_debug("Bootloader initialized SPI");

    error = sdcard_init();
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
    memory_copy(_sdata, _sidata, data_section_size*4);

    // Zero out bss
    uint32_t bss_section_size = _ebss - _sbss;
    memory_set(_sbss, 0, bss_section_size*4);

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
