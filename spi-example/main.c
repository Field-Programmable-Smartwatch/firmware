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

void configure()
{
    terminal_configuration_t terminal;
    
    debug_init();
    display_init();

    terminal.width = 18;
    terminal.height = 10;
    terminal_init(terminal);
}

void main()
{
    configure();
    display_clear();
    while (1) {
        terminal_print("Hello World!\n");
        display_render();
        for (uint32_t i = 0; i < 800000; i++);        
    }
}
