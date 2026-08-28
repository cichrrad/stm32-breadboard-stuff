#include "stm32g491xx.h"
#include "device_drivers/display_driver/ddriver.h"
#include "device_drivers/tick_engine/systick_timer.h"
#include "miky_bitmap.h"

#include <stdint.h>

void demo1()
{
    // dd_draw_bitmap(0,16,MIKY_WIDTH,MIKY_HEIGHT,miky,true);
    // dd_draw_circle(80,32,12,false);
    // dd_fill_circle(80,32,8,false);
    // dd_write_string(30,56,"Hello World!",false,true);
    // dd_draw_triangle(72,17,96,3,125,60,false);
    // dd_fill_triangle(10,15,20,3,64,15,false);
    // dd_draw_rect(112,4,10,20,false);
}

int main(void)
{

    SysTick_Init(16000000);
    DD_Init();
    dd_fill_rect(0, 0, DD_WIDTH, DD_HEIGHT, true);

    DDLoadBarType lbar = {
        .x = 1,
        .y = 1,
        .width = 100,
        .height = 10,
        .steps = 10,
        .val = 0,
        .active_equals_on = false,
    };
    dd_draw_loadbar(&lbar);
    dd_write_string(1, 20, "TICK", false, true);

    dd_update();

    dd_fill_rect(0, 0, DD_WIDTH, DD_HEIGHT, true);
    dd_draw_loadbar(&lbar);
    dd_write_string(1, 20, "TOCK", false, true);

    dd_update();

    uint32_t last_tick_ui_update = GetTick();
    while (1)
    {
        if (SysTick_IsElapsed(last_tick_ui_update, 1000))
        {
            last_tick_ui_update = GetTick();
            dd_loabdar_set_value(&lbar, (lbar.val + 10) % 250);
            dd_draw_loadbar(&lbar);
            dd_update();
        }
        __WFI();
    }
    return 0;
}