#include <ota_update.h>
#include <task_manager.h>
#include <systick_timer.h>
#include <bluefruit.h>
#include <sdcard.h>
#include <string.h>
#include <stdbool.h>
#include <error.h>
#include <log.h>

#define OTA_UPDATE_RESPONSE_SIZE 256

static error_t ota_update_read(uint8_t *data, uint32_t data_size)
{
    error_t error; 
    if (!data) {
        log_error(ERROR_INVALID, "data pointer points to NULL");
        return ERROR_INVALID;
    }
    uint8_t response[OTA_UPDATE_RESPONSE_SIZE];
    memory_set(response, 0, OTA_UPDATE_RESPONSE_SIZE);
    error = bluefruit_write("at+bleuartrx", 12);
    if (error) {
        log_error(error, "Failed to send at+bleuartrx command");
        return error;
    }

    systick_timer_wait_ms(100);
    error = bluefruit_read(response, OTA_UPDATE_RESPONSE_SIZE);
    if (error) {
        log_error(error, "Failed to receive response from at+bleuartrx command");
        return error;
    }
    memory_copy(data, &response[0], data_size);

    // Send back ACK
    error = bluefruit_write("at+bleuarttx=0", 14);
    if (error) {
        log_error(error, "Failed to send at+bleuarttx=0 command");
        return error;
    }
    systick_timer_wait_ms(50);

    error = bluefruit_read(response, OTA_UPDATE_RESPONSE_SIZE);
    if (error) {
        log_error(error, "Failed to receive response from at+bleuarttx=0 command");
        return error;
    }

    return SUCCESS;
}

// TODO: right now this can only handle 4GiB firmware image
//       Do something to increase this
static error_t ota_update_receive_and_write_firmware(uint32_t firmware_size)
{
    error_t error;
    uint8_t block[512];
    uint32_t block_index = 0;
    uint32_t block_addr = 0;

    while (firmware_size > 0) {
        uint32_t size = (firmware_size < 16) ? firmware_size : 16;
        error = ota_update_read(&block[block_index], size);
        if (error) {
            log_error(error, "Failed to read firmware data");
            return error;
        }

        block_index += size;
        firmware_size -= size;

        if (block_index == 512 || firmware_size == 0) {
            sdcard_write_block(block_addr, block);
            block_addr += 1;
            block_index = 0;
        }
    }

    return SUCCESS;
}

static error_t ota_update_wait_for_start_signal(uint32_t *firmware_size)
{
    error_t error;
    *firmware_size = 0;
    // TODO: Add some sort of timeout
    error = ota_update_read((uint8_t *) firmware_size, sizeof(uint32_t));
    if (error) {
        log_error(error, "Failed to read start signal from ota update transaction");
    }

    return SUCCESS;
}

void ota_update_application_start()
{
    error_t error;
    uint32_t firmware_size;
    task_t *task = task_manager_get_task_by_name(string("ota update"));
    if (!task) {
        log_error(ERROR_INVALID, "Failed get task '%s'");
        task_manager_start_task_by_name(string("Menu"));
        return;
    }

    while (task->status == TASK_STATUS_RUNNING) {
        error = ota_update_wait_for_start_signal(&firmware_size);
        log_debug("firmware size: %x", firmware_size);
        if (error == ERROR_TIMEOUT) {
            log_error(error, "Did not receive start signal from OTA transaction");
            task->status = TASK_STATUS_STOP;
            task_manager_start_task_by_name(string("Menu"));
            return;
        }

        error = ota_update_receive_and_write_firmware(firmware_size);

        task->status = TASK_STATUS_STOP;
        task_manager_start_task_by_name(string("Menu"));
    }
}
