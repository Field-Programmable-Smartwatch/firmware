#include <ATtui.h>
#include <kernel/task/task_manager.h>
#include <kernel/debug/log.h>
#include <drivers/ble/bluefruit.h>
#include <libraries/string.h>
#include <mcu/system_timer.h>

#define ATtui_COMMAND_MAX 128
#define ATtui_RESPONSE_MAX 256

static int32_t ATtui_send_uart(uint8_t *message, uint32_t size)
{
    uint8_t command[16] = "at+bleuarttx= ";
    for (uint8_t i = 0; i < size; i++) {
        command[13] = message[i];
        bluefruit_write(command, 14);
        system_timer_wait_ms(50);
    }
    return 0;
}

static int32_t ATtui_recv_uart()
{
    uint8_t command[16] = "at+bleuartrx";
    bluefruit_write(command, 12);
    system_timer_wait_ms(50);
    return 0;
}

static int32_t ATtui_send_command(uint8_t *command, uint32_t size)
{
    bluefruit_write(command, size);
    return 0;
}

static int32_t ATtui_get_response(uint8_t *response, uint32_t size)
{
    memset(response, 0, size);
    bluefruit_read(response, size);
    return 0;
}

void ATtui_application_start()
{
    uint32_t command_length = 0;
    uint8_t command[ATtui_COMMAND_MAX];
    uint8_t response[ATtui_RESPONSE_MAX];
    task_t *task = task_manager_get_task_by_name(string("ATtui"));

    memset(command, 0, ATtui_COMMAND_MAX);
    memset(response, 0, ATtui_RESPONSE_MAX);

    log(LOG_LEVEL_INFO, "> ");
    while (task->status == TASK_STATUS_RUNNING) {
        uint8_t c = log_wait_for_input();
        if (c == 8) { // Backspace key pressed
            if (command_length == 0) {
                continue;
            }
            command[--command_length] = 0;
            // Clear character on tty
            log(LOG_LEVEL_INFO, "\b \b");

        } else if (c == '\r') { // Enter key pressed
            if (memcmp(command, "exit", 4) == 0) { // Exit command
                task->status = TASK_STATUS_STOP;
                task_manager_start_task_by_name(string("Menu"));
                break;
            } else if (memcmp(command, "send", 4) == 0) { // Send command
                ATtui_send_uart(command+5, command_length-5);

            } else if (memcmp(command, "recv", 4) == 0) { // recv command
                ATtui_recv_uart();

            } else {
                ATtui_send_command(command, command_length);
            }
            command_length = 0;
            system_timer_wait_ms(50);
            ATtui_get_response(response, ATtui_RESPONSE_MAX);
            log(LOG_LEVEL_INFO, "\r\n%s\r\n> ", string((char *)response));

        } else if (command_length < ATtui_COMMAND_MAX - 1 &&
                   (c >= 32 && c <= 126)) { // Character key pressed
            command[command_length++] = c;
            log(LOG_LEVEL_INFO, "%c", c);
        }
    }
}
