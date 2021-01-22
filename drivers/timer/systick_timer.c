#include <stm32wb55xx.h>
#include <systick_timer.h>
#include <debug.h>

#define SYS_CLOCK 16000000L

uint32_t tick_count = 0;

void systick_timer_init()
{
    SysTick->LOAD = SYS_CLOCK / 1000;
    SysTick->VAL = 0;
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk);
}

void systick_timer_destroy()
{
    // TODO
    return;
}

void systick_timer_reset()
{
    tick_count = 0;
}

inline void systick_timer_start()
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

inline void systick_timer_stop()
{
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk);
}

uint32_t systick_timer_get_tick_count()
{
    return tick_count;
}

void systick_timer_wait_ms(uint32_t ms)
{
    systick_timer_start();
    uint32_t end_tick = tick_count + ms;
    while(tick_count != end_tick);
    systick_timer_stop();
    tick_count = 0;
}

void SysTick_Handler()
{
    tick_count++;
}
