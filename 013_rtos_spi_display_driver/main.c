#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"
#include "ddriver.h"
#include "ui_widgets.h"

#include "FreeRTOS.h"
#include "task.h"

void vRenderTask(void *rgs)
{
    DD_Init();

    UITextType text = {
        .fill_state = true,
        .solid_bg = true,
        .x = 0,
        .y = 0,
        .val = "..."};

    while (1)
    {
        dd_clear();
        ui_draw_string(&text, "Hello");
        text.x = (text.x + 1) % 128;
        text.y = (text.y + 1) % 64;
        dd_draw_line(5,5,45,45,true);
        dd_draw_line(5,45,45,5,true);
        dd_update();
        // 17 -> 60FPS
        vTaskDelay(pdTICKS_TO_MS(17));
    }
}

int main(void)
{

    xTaskCreate(vRenderTask, "render", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    };
    return 0;
}