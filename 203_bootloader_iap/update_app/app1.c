#include "stm32g491xx.h"
#include "ddriver.h"
#include "ui_widgets.h"
#include "systick_timer.h"
#include <stdint.h>

int main(void)
{
    __enable_irq();
    SysTick_Init(16000000);
    DD_Init();

    UITextType toast = {
        .solid_bg = true,
        .fill_state = true,
        .x = 10,
        .y = 20,
        .val = "toast"};

    UITextType hbeat = {
        .solid_bg = true,
        .fill_state = true,
        .x = 1,
        .y = 1,
        .val = "tick-tock"
    };

    uint32_t last_tick = GetTick();
    bool tick_tock = true;
    while (1)
    {
        if(SysTick_IsElapsed(last_tick,1000)){
            last_tick = GetTick();
            tick_tock = !tick_tock;

            dd_clear();
            ui_draw_string(&hbeat, tick_tock ? "TICK" : "TOCK");
            ui_draw_string(&toast, "APP 1 HELLO");
            dd_update();
        }
        __WFI();
    }

    return 0;
}