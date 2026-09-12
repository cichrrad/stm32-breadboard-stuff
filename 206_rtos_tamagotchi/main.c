#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <float.h>

#include "gpio.h"
#include "ddriver.h"
#include "ui_widgets.h"

#include "FreeRTOS.h"
#include "task.h"

#include "miky.h"

#define UI_REFRESH_RATE_MS 17

void vRenderTask(void *pvParameters)
{
    DD_Init();

    bool hbeat = true;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(UI_REFRESH_RATE_MS);
    uint8_t x_offset = 0;
    while (1)
    {
        dd_clear();
        dd_set_pixel(127, 0, hbeat);
        dd_draw_bitmap(x_offset, 16, MIKY_WIDTH, MIKY_HEIGHT, miky_emotions[0], true);
        x_offset = (x_offset == 127 ? 0 : x_offset + 1);
        hbeat = !hbeat;
        dd_update();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

int main(void)
{

    xTaskCreate(vRenderTask, "RenderTask", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    }
    return 0;
}