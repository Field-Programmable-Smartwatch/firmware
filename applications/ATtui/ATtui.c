#include <ATtui.h>
#include <task_manager.h>
#include <debug.h>
#include <bluefruit.h>
#include <string.h>
#include <sdep.h>

#define ATtui_COMMAND_MAX 128
#define ATtui_RESPONSE_MAX 256

static int32_t ATtui_send_uart(uint8_t *message, uint32_t size)
{
    uint8_t command[16] = "at+bleuarttx= ";
    for (uint8_t i = 0; i < size; i++) {
        command[13] = message[i];
        bluefruit_write(command, 14);
    }
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
    task_t *task = task_manager_get_task_by_name("ATtui");

    memset(command, 0, ATtui_COMMAND_MAX);
    memset(response, 0, ATtui_RESPONSE_MAX);
    
    debug_print("> ");
    while (task->status == TASK_STATUS_RUNNING) {
        uint8_t c = debug_wait_for_input();
        if (c == 8) { // Backspace key pressed
            if (command_length == 0) {
                continue;
            }
            command[--command_length] = 0;
            // Clear character on tty
            debug_print("\b \b");

        } else if (c == '\r') { // Enter key pressed
            if (memcmp(command, "exit", 4) == 0) { // Exit command
                task->status = TASK_STATUS_STOP;
                task_manager_start_task_by_name("Menu");
            } else if (memcmp(command, "send", 4) == 0) { // Send command
                ATtui_send_uart(command+5, command_length-5);
            } else {
                ATtui_send_command(command, command_length);
            }
            command_length = 0;
            ATtui_get_response(response, ATtui_RESPONSE_MAX);
            debug_print("\r\n%s\r\n> ", response);

        } else if (command_length < ATtui_COMMAND_MAX - 1 &&
                   (c >= 32 && c <= 126)) { // Character key pressed
            command[command_length++] = c;
            debug_print("%c", c);
        }
    }
}
