#include "stm32g491xx.h"
#include "ddriver.h"
#include "ui_widgets.h"
#include "systick_timer.h"
#include "miky_bitmap.h"

#include <stdint.h>

int main(void)
{

    SysTick_Init(16000000);
    DD_Init();

    UILoadBarType lbar = {
        .x = 1,
        .y = 1,
        .width = 100,
        .height = 10,
        .val = 0,
        .fill_state = true,
    };

    UITextType tbar = {
        .x = 1,
        .y = 30,
        .val = "default",
        .fill_state = true,
        .solid_bg = true
    };

    uint32_t last_tick_ui_update = GetTick();
    volatile bool flip = false;
    while (1)
    {
        if (SysTick_IsElapsed(last_tick_ui_update, 500))
        {
            last_tick_ui_update = GetTick();
            dd_clear();

            if (flip)
            {
                ui_draw_string(&tbar, "TICK");
                flip = false;
            }
            else
            {
                ui_draw_string(&tbar, "TOCK");
                flip = true;
            }

            // dd_fill_rect(0, 0, DD_WIDTH, DD_HEIGHT, true);
            lbar.val += (10 % 255);
            ui_draw_loadbar(&lbar);
            dd_fill_triangle(64,32,100,55,90,21,true);
            dd_fill_circle(64,32,5,true);
            dd_fill_circle(100,55,5,true);
            dd_fill_circle(90,21,5,true);

                    dd_draw_line(5,5,45,45,true);
        dd_draw_line(5,45,45,5,true);

            dd_update();
        }
        __WFI();
    }
    return 0;
}