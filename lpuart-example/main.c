#include <stm32wb55xx.h>
#include <stdint.h>

#include <gpio.h>
#include <lpuart.h>
#include <debug.h>

void configure()
{
    debug_init();
}

void main()
{
    configure();

    uint32_t u = 1231;
    int32_t i1 = -1231;
    char s[6] = "hello";
    char c = 'c';
    debug_print("Start of program!\r\n");
    debug_print("u: %u, i: %i, s: %s, c: %c\r\n", u, i1, s, c);
    while (1) {
        debug_print("Hello, world!\r\n");
        for (uint32_t i = 0; i < 800000; i++);
    }
}
