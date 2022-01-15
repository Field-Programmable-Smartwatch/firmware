#include <libraries/log.h>
#include <libraries/input.h>
#include <libraries/string.h>

extern uint32_t _sidata[];
extern uint32_t _sdata[];
extern uint32_t _edata[];
extern uint32_t _sbss[];
extern uint32_t _ebss[];

void main()
{
    // Copy data section from flash memory to ram
    uint32_t data_section_size = _edata - _sdata;
    memcpy(_sdata, _sidata, data_section_size*4);

    // Zero out bss
    uint32_t bss_section_size = _ebss - _sbss;
    memset(_sbss, 0, bss_section_size*4);

    error_t error;
    input_event_t event = {0};
    while(1) {
        error = input_poll(&event);
        if (error || !event.valid) {
            continue;
        }
        log_info("Event received: type:%u x:%u y:%u", event.type, event.x, event.y);
    }
}
