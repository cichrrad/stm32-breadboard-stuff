#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <float.h>
#include <assert.h>

#include "gpio.h"
#include "ddriver.h"
#include "ui_widgets.h"
#include "utils_rng.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "local_source/miky.h"
#include "local_source/ui_banner.h"
#include "local_source/pet/pet.h"
#include "local_source/input/input.h"
#include "local_source/mini_game/mini_game_runner.h"
#include "local_source/mini_game/rps/rps.h"

#define UI_REFRESH_RATE_MS 20
// Target 50 ticks/s
#define GAME_TICK_RATE_MS 20
#define TICKS_PER_SECOND (1000UL / GAME_TICK_RATE_MS)

#define INPUT_POLL_RATE_MS 10

// UI Offsets
#define PET_VIEWPORT_X 28
#define PET_VIEWPORT_Y 16

// Define asserts
static_assert(UI_REFRESH_RATE_MS >= GAME_TICK_RATE_MS, "Refresh Rate must be >= than Tick Rate");

QueueHandle_t xInputQueue;

static const InputMapping idle_input_mapping = {
    .inputs = {INPUT_BTN1, INPUT_BTN2, INPUT_BTN3},
    .actions = {(actionFn)Pet_Play, (actionFn)Pet_Feed, (actionFn)Pet_Pet}};

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

    .food_change_time_ticks = TICKS_PER_SECOND * 360,
    .bored_change_time_ticks = TICKS_PER_SECOND * 360,
    .alone_change_time_ticks = TICKS_PER_SECOND * 360,

    .last_time_fed = 0,
    .last_time_played_with = 0,
    .last_time_pet = 0,

    .currentActivity = ACTIVITY_IDLE,

    .emotion_array = miky_emotions,
    .currentEmotion = EMOTION_HAPPY,
    .alive = false};

MiniGameRunner_t MiniGameRunner = {
    .running = false,
    .currentMinigame = MINI_GAME_COUNT,
    .games = {
        // Rock-Paper-Scissors
        {.initFn = rps_initFn,
         .renderFn = rps_renderFn,
         .updateFn = rps_updateFn,
         .im =
             {.inputs =
                  {INPUT_BTN1,
                   INPUT_BTN2,
                   INPUT_BTN3},
              .actions =
                  {rps_select_paper,
                   rps_select_rock,
                   rps_select_scissors}},
         .exit_flag = false
        }
    }
};

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
        .val = "..."};

    while (1)
    {

        foodBar.val = Miky.food;
        aloneBar.val = Miky.alone;
        boredBar.val = Miky.bored;

        dd_clear();
        hbeat = !hbeat;
        if (Miky.alive)
        {
            switch (Miky.currentActivity)
            {
            case ACTIVITY_IDLE:
                // draw top UI
                dd_draw_bitmap(0, 0, UI_BANNER_WIDTH, UI_BANNER_HEIGHT, ui_banner, true);
                // update values for ui
                ui_draw_loadbar(&foodBar);
                ui_draw_loadbar(&aloneBar);
                ui_draw_loadbar(&boredBar);
                dd_draw_bitmap(PET_VIEWPORT_X, PET_VIEWPORT_Y, MIKY_WIDTH, MIKY_HEIGHT, Miky.emotion_array[Miky.currentEmotion], true);
                break;
            case ACTIVITY_IN_GAME:
                if (mgr_is_running(&MiniGameRunner))
                {
                    mgr_call_renderFn(&MiniGameRunner);
                }
                break;
            }
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

// TODO -- sync with render/input task
void vGameUpdateTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(GAME_TICK_RATE_MS);
    GameInput currentInput;

    while (1)
    {
        if (Miky.alive)
        {
            // handle inputs (if there are some)
            while (xQueueReceive(xInputQueue, &currentInput, 0) == pdTRUE)
            {
                switch (Miky.currentActivity)
                {
                case ACTIVITY_IDLE:
                    ((void (*)(Pet *))idle_input_mapping.actions[currentInput])(&Miky);
                    break;
                case ACTIVITY_IN_GAME:
                    mgr_get_current_game_instance(&MiniGameRunner)->im.actions[currentInput]();
                    break;
                default:
                    // HUH
                    break;
                }
            }

            switch (Miky.currentActivity)
            {
            case ACTIVITY_IDLE:
                Pet_Update_Stats(&Miky);
                Pet_Calculate_Emotion(&Miky);
                break;
            case ACTIVITY_IN_GAME:
                if (MiniGameRunner.running == false)
                {
                    // TODO -- here we would randomly select a game
                    MiniGameRunner.currentMinigame = MINI_GAME_RPS;
                    mgr_call_initFn(&MiniGameRunner);
                    // confirm it is false so we can catch end signal after
                    // game
                    mgr_get_current_game_instance(&MiniGameRunner)->exit_flag = false;
                    MiniGameRunner.running = true;
                    break;
                }

                if (MiniGameRunner.running && mgr_get_current_game_instance(&MiniGameRunner)->exit_flag == true)
                {
                    Miky.currentActivity = ACTIVITY_IDLE;
                    // to be sure
                    mgr_call_initFn(&MiniGameRunner);
                    MiniGameRunner.running = false;
                    MiniGameRunner.currentMinigame = MINI_GAME_COUNT;
                    break;
                }

                mgr_call_updateFn(&MiniGameRunner);
                break;
            default:
                break;
            }
        }
        else
        {
            // EXIT
            vTaskDelete(NULL);
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

void vInputHandlerTask(void *pvParameters)
{
    // setup buttons
    // Enable GPIOA and GPIOB clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN;

    // PA1 and PA4 to Input Mode (00)
    GPIOA->MODER &= ~(GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE4_Msk);
    // PB0 to Input Mode (00)
    GPIOB->MODER &= ~GPIO_MODER_MODE0_Msk;

    // Enable Pull-up resistors (01)
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD1_Msk | GPIO_PUPDR_PUPD4_Msk);
    GPIOA->PUPDR |= (1 << GPIO_PUPDR_PUPD1_Pos) | (1 << GPIO_PUPDR_PUPD4_Pos);

    GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD0_Msk;
    GPIOB->PUPDR |= (1 << GPIO_PUPDR_PUPD0_Pos);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(INPUT_POLL_RATE_MS);

    // last 8 polls
    uint8_t button1_history = 0xFF;
    uint8_t button2_history = 0xFF;
    uint8_t button3_history = 0xFF;

    while (1)
    {
        // shift by 1 to make space for latest read, then read
        // PA1
        uint8_t pa1_val = (GPIOA->IDR & (1 << 1)) ? 1 : 0;
        button1_history = (button1_history << 1) | pa1_val;
        // PA4
        uint8_t pa4_val = (GPIOA->IDR & (1 << 4)) ? 1 : 0;
        button2_history = (button2_history << 1) | pa4_val;
        // PB0
        uint8_t pb0_val = (GPIOB->IDR & (1 << 0)) ? 1 : 0;
        button3_history = (button3_history << 1) | pb0_val;

        // look for 0x80 (1000 0000)
        // -> means it was pressed and was
        // held for last 7 polls + it filters
        // bouncing, unless it takes 7 polls...
        // (7*INPUT_POLL_RATE_MS ~ 70ms is never gonna
        // happen for bounces)
        GameInput input = INPUTS_COUNT;
        if (button1_history == 0x80)
        {
            input = INPUT_BTN1;
        }

        if (button2_history == 0x80)
        {
            input = INPUT_BTN2;
        }

        if (button3_history == 0x80)
        {
            input = INPUT_BTN3;
        }

        if (input != INPUTS_COUNT)
        {
            xQueueSend(xInputQueue, &input, 0);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

int main(void)
{

    Miky.alive = true;
    Miky.currentActivity = ACTIVITY_IDLE;
    xInputQueue = xQueueCreate(5, sizeof(GameInput));

    RNG_Init();

    xTaskCreate(vRenderTask, "RenderTask", 256, NULL, 2, NULL);
    xTaskCreate(vGameUpdateTask, "GameUpdateTask", 256, NULL, 1, NULL);
    xTaskCreate(vInputHandlerTask, "InputHandlerTask", 256, NULL, 3, NULL);

    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    }
    return 0;
}