#include <stm32wb55xx.h>
#include <i2c.h>
#include <error.h>
#include <stdint.h>
#include <stdbool.h>
#include <gpio.h>
#include <log.h>
#include <string.h>
#include <rcc.h>

#define I2C_DEVICE_MAX 8

#define I2C_WRITE 0
#define I2C_READ 1

#define I2C I2C1
#define I2C_SCL_PORT GPIOB
#define I2C_SCL_PIN 6
#define I2C_SDA_PORT GPIOB
#define I2C_SDA_PIN 7

static bool g_i2c_initialized = false;
i2c_configuration_t g_i2c_devices[I2C_DEVICE_MAX];
i2c_handle_t g_current_i2c_config = -1;

static bool i2c_invalid_handle(i2c_handle_t handle)
{
    return (handle < 0 || handle >= I2C_DEVICE_MAX || !g_i2c_devices[handle].is_open);
}

static error_t i2c_find_free_device(i2c_handle_t *handle)
{
    if (!handle) {
        log_debug("Passed null handle pointer as function argument");
        return ERROR_INVALID;
    }

    i2c_handle_t i2c_handle;

    for (i2c_handle = 0; i2c_handle < I2C_DEVICE_MAX; i2c_handle++) {
        if (!g_i2c_devices[i2c_handle].is_open) {
            *handle = i2c_handle;
            return SUCCESS;
        }
    }

    return ERROR_NO_MEMORY;
}

static bool i2c_data_received()
{
    return I2C->ISR & I2C_ISR_RXNE;
}

static bool i2c_ready_to_transmit()
{
    return I2C->ISR & I2C_ISR_TXE;
}

error_t i2c_read(i2c_handle_t handle, uint8_t *data, uint32_t size)
{
    if (i2c_invalid_handle(handle)) {
        log_error(ERROR_INVALID, "Failed to read from i2c device. Invalid handle");
        return ERROR_INVALID;
    }

    if (!data) {
        log_error(ERROR_INVALID, "Passed null data pointer as function argument");
        return ERROR_INVALID;
    }

    i2c_configuration_t i2c_device = g_i2c_devices[handle];
    uint32_t index = 0;

    // Set CR2
    //    - Addressing mode ADD10
    //    - Slave address
    //    - Transfer direction
    //    - Number of bytes to be transfered
    // Set start bit

    I2C->CR2 |= i2c_device.address_mode << I2C_CR2_ADD10_Pos;
    if (i2c_device.address_mode == I2C_ADDRESS_MODE_7BIT) {
        I2C->CR2 |= i2c_device.address << (I2C_CR2_SADD_Pos+1);
    } else {
        I2C->CR2 |= i2c_device.address << I2C_CR2_SADD_Pos;
    }
    I2C->CR2 |= I2C_READ << I2C_CR2_RD_WRN_Pos;
    // TODO: Add capability to have over 256 bytes
    //I2C->CR2 |= I2C_CR2_AUTOEND;
    I2C->CR2 |= (size & 0xFF) << I2C_CR2_NBYTES_Pos;
    I2C->CR2 |= I2C_CR2_START;

    // TODO: we definitely need a timeout here
    while (size) {
        if (!i2c_data_received()) {
            //log_debug("No data receieved");
            continue;
        }

        data[index++] = I2C->RXDR;
        size--;
    }

    return SUCCESS;
}

error_t i2c_write(i2c_handle_t handle, uint8_t *data, uint32_t size)
{
    if (i2c_invalid_handle(handle)) {
        log_error(ERROR_INVALID, "Failed to write to i2c device. Invalid handle");
        return ERROR_INVALID;
    }

    if (!data) {
        log_error(ERROR_INVALID, "Passed null data pointer as function argument");
        return ERROR_INVALID;
    }

    i2c_configuration_t i2c_device = g_i2c_devices[handle];
    uint32_t index = 0;

    // Set CR2
    //    - Addressing mode ADD10
    //    - Slave address
    //    - Transfer direction
    //    - Number of bytes to be transfered
    // Set start bit

    I2C->CR2 |= i2c_device.address_mode << I2C_CR2_ADD10_Pos;
    if (i2c_device.address_mode == I2C_ADDRESS_MODE_7BIT) {
        I2C->CR2 |= i2c_device.address << (I2C_CR2_SADD_Pos+1);
    } else {
        I2C->CR2 |= i2c_device.address << I2C_CR2_SADD_Pos;
    }
    I2C->CR2 |= I2C_WRITE << I2C_CR2_RD_WRN_Pos;
    // TODO: Add capability to have over 256 bytes
    //I2C->CR2 |= I2C_CR2_AUTOEND;
    I2C->CR2 |= size << I2C_CR2_NBYTES_Pos;
    I2C->CR2 |= I2C_CR2_START;
    log_debug("CR2: %x", I2C->CR2);
    // TODO: we definitely need a timeout here
    while (index < size) {
        if (!i2c_ready_to_transmit()) {
            continue;
        }

        I2C->TXDR = data[index++];
    }

    return SUCCESS;
}

error_t i2c_open( i2c_handle_t *handle, i2c_configuration_t device)
{
    if (!g_i2c_initialized) {
        log_error(ERROR_INVALID, "I2C bus has not been initialized. Unable to open device");
        return ERROR_INVALID;
    }

    if (!handle) {
        log_error(ERROR_INVALID, "Passed null handle pointer as function argument");
        return ERROR_INVALID;
    }
    error_t error;

    error = i2c_find_free_device(handle);
    if (error) {
        log_error(error, "Failed to open device. No free devices left");
        return error;
    }

    memory_copy(&g_i2c_devices[*handle], &device, sizeof(i2c_configuration_t));
    g_i2c_devices[*handle].is_open = true;

    return SUCCESS;
}

error_t i2c_close(i2c_handle_t handle)
{
    if (i2c_invalid_handle(handle)) {
        log_error(ERROR_INVALID, "Unable to close device. Invalid handle");
        return ERROR_INVALID;
    }

    g_i2c_devices[handle].is_open = false;

    return SUCCESS;
}

error_t i2c_init()
{
    error_t error;
    gpio_configuration_t scl_pin;
    gpio_configuration_t sda_pin;
    gpio_configuration_t sda_pin2;
    memory_set(&scl_pin, 0, sizeof(gpio_configuration_t));
    memory_set(&sda_pin, 0, sizeof(gpio_configuration_t));
    memory_set(&sda_pin2, 0, sizeof(gpio_configuration_t));

    rcc_enable_i2c1_clock();
    rcc_enable_gpiob_clock();

    scl_pin.port = I2C_SCL_PORT;
    scl_pin.pin = I2C_SCL_PIN;
    scl_pin.mode = GPIO_MODE_ALT_FUNC;
    scl_pin.output_type = GPIO_OUTPUT_TYPE_OPEN_DRAIN;
    scl_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    scl_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    scl_pin.alternative_function = 4;
    error = gpio_configure_pin(scl_pin);
    if (error) {
        log_error(error, "Failed to configure scl pin");
        return error;
    }

    sda_pin.port = I2C_SDA_PORT;
    sda_pin.pin = I2C_SDA_PIN;
    sda_pin.mode = GPIO_MODE_ALT_FUNC;
    sda_pin.output_type = GPIO_OUTPUT_TYPE_OPEN_DRAIN;
    sda_pin.output_speed = GPIO_OUTPUT_SPEED_LOW;
    sda_pin.pull_resistor = GPIO_PULL_RESISTOR_NONE;
    sda_pin.alternative_function = 4;
    error = gpio_configure_pin(sda_pin);
    if (error) {
        log_error(error, "Failed to configure sda pin");
        return error;
    }

    I2C->TIMINGR |= 3 << I2C_TIMINGR_PRESC_Pos;
    // Set SCLH and SCLL bits TIMINGR
    I2C->TIMINGR |= (0xF << I2C_TIMINGR_SCLH_Pos) | (0x13 << I2C_TIMINGR_SCLL_Pos);
    I2C->TIMINGR |= (2 << I2C_TIMINGR_SDADEL_Pos) | (4 << I2C_TIMINGR_SCLDEL_Pos);
    // Enable I2C peripheral
    I2C->CR1 |= I2C_CR1_PE;

    g_i2c_initialized = true;
    return SUCCESS;
}

error_t i2c_deinit()
{
    log_debug("TODO");
    return SUCCESS;
}
