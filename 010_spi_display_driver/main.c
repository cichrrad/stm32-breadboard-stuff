#include "stm32g491xx.h"
#include "device_drivers/display_driver/ddriver.h"
#include "device_drivers/display_driver/ui_widgets.h"
#include "device_drivers/tick_engine/systick_timer.h"
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
            dd_update();
        }
        __WFI();
    }
    return 0;
}