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
#include "local_source/pet/pet.h"

// Target ~24FPS
#define UI_REFRESH_RATE_MS 42
// Target 50 ticks/s
#define GAME_TICK_RATE_MS 20
#define TICKS_PER_SECOND (1000UL / GAME_TICK_RATE_MS)

// UI Offsets
#define PET_VIEWPORT_X 28
#define PET_VIEWPORT_Y 16

static Pet Miky = {
    .food = PET_MAX_STAT_VALUE,
    .bored = PET_MIN_STAT_VALUE,
    .alone = PET_MIN_STAT_VALUE,

    .food_status_severity = 0,
    .bored_status_severity = 0,
    .alone_status_severity = 0,

    .food_change_factor = 50,
    .bored_change_factor = 50,
    .alone_change_factor = 50,

    .food_change_time_ticks = TICKS_PER_SECOND,
    .bored_change_time_ticks = TICKS_PER_SECOND,
    .alone_change_time_ticks = TICKS_PER_SECOND,

    .last_time_fed = 0,
    .last_time_played_with = 0,
    .last_time_pet = 0,

    .currentActivity = ACTIVITY_IDLE,

    .emotion_array = miky_emotions,
    .currentEmotion = EMOTION_HAPPY,
    .alive = false};

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

    UITextType ui_text = {
        .x = 10,
        .y = 4,
        .fill_state = true,
        .solid_bg = false,
        .val = "..."
    };

    while (1)
    {

        foodBar.val = Miky.food;
        aloneBar.val = Miky.alone;
        boredBar.val = Miky.bored;

        dd_clear();
        hbeat = !hbeat;
        if (Miky.alive)
        {

            // draw top UI
            dd_draw_bitmap(0, 0, UI_BANNER_WIDTH, UI_BANNER_HEIGHT, ui_banner, true);
            // update values for ui
            ui_draw_loadbar(&foodBar);
            ui_draw_loadbar(&aloneBar);
            ui_draw_loadbar(&boredBar);
            dd_draw_bitmap(PET_VIEWPORT_X, PET_VIEWPORT_Y, MIKY_WIDTH, MIKY_HEIGHT, Miky.emotion_array[Miky.currentEmotion], true);
        }
        else
        {
            ui_draw_string(&ui_text, "GAME OVER");
            dd_draw_bitmap(PET_VIEWPORT_X, PET_VIEWPORT_Y, MIKY_WIDTH, MIKY_HEIGHT, Miky.emotion_array[Miky.currentEmotion], true);
        }
        dd_set_pixel(127, 63, hbeat);
        dd_update();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

void vGameUpdateTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(GAME_TICK_RATE_MS);

    while (1)
    {
        if (Miky.alive)
        {
            Pet_Update_Stats(&Miky);
            Pet_Calculate_Emotion(&Miky);
        }
        else
        {
            // EXIT
            vTaskDelete(NULL);
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

int main(void)
{

    Miky.alive = true;
    xTaskCreate(vRenderTask, "RenderTask", 256, NULL, 2, NULL);
    xTaskCreate(vGameUpdateTask, "GameUpdateTask", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    }
    return 0;
}