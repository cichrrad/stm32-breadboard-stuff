#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <float.h>

#include "gpio.h"
#include "ddriver.h"
#include "ui_widgets.h"

#include "FreeRTOS.h"
#include "task.h"

#include "local_source/miky.h"
#include "local_source/ui_banner.h"

// Target ~24FPS
#define UI_REFRESH_RATE_MS 42

// NOTE: This task cannot be notified via basic
// task notify, because display driver reservers this
// so that DMA can notify its task
// (because I am an idiot)
void vRenderTask(void *pvParameters)
{
    DD_Init();

    bool hbeat = true;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(UI_REFRESH_RATE_MS);

    UILoadBarType foodBar = {
        .fill_state = false,
        .height = UI_FOOD_BAR_HEIGHT,
        .width = UI_FOOD_BAR_WIDTH,
        .x = UI_FOOD_BAR_X,
        .y = UI_FOOD_BAR_Y,
        .val = 0};

    UILoadBarType boredBar = {
        .fill_state = false,
        .height = UI_BORED_BAR_HEIGHT,
        .width = UI_BORED_BAR_WIDTH,
        .x = UI_BORED_BAR_X,
        .y = UI_BORED_BAR_Y,
        .val = 0};

    UILoadBarType aloneBar = {
        .fill_state = false,
        .height = UI_ALONE_BAR_HEIGHT,
        .width = UI_ALONE_BAR_WIDTH,
        .x = UI_ALONE_BAR_X,
        .y = UI_ALONE_BAR_Y,
        .val = 0};

    while (1)
    {
        dd_clear();
        hbeat = !hbeat;

        // draw top UI
        dd_draw_bitmap(0, 0, UI_BANNER_WIDTH, UI_BANNER_HEIGHT, ui_banner, true);
        // update values for ui
    
        ui_draw_loadbar(&foodBar);
        ui_draw_loadbar(&aloneBar);
        ui_draw_loadbar(&boredBar);

        dd_set_pixel(127, 63, hbeat);
        dd_update();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

void vGameUpdateTask(void *pvParameters) {

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