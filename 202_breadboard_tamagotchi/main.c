#include "gpio.h"
#include "ddriver.h"
#include "systick_timer.h"

#include "local_source/input_driver/input_driver.h"
#include "local_source/pet/pet.h"

#include "local_source/assets/ui_banner.h"
#include "local_source/assets/miky.h"

uint8_t get_bar_percentage(uint8_t val, uint8_t max_width)
{
    uint8_t step = max_width / 5;
    return (val / 50) * step;
}

void stat_bars_update(Pet *p)
{
    dd_fill_rect(0, 16, DD_WIDTH, DD_HEIGHT - 16, true);
    dd_draw_bitmap(0, 0, UI_WIDTH, UI_HEIGHT, ui_banner, true);

    dd_fill_rect(UI_FOOD_BAR_X, UI_FOOD_BAR_Y, get_bar_percentage(p->food, UI_FOOD_BAR_WIDTH), UI_FOOD_BAR_HEIGHT, false);
    dd_fill_rect(UI_BORED_BAR_X, UI_BORED_BAR_Y, get_bar_percentage(p->bored, UI_BORED_BAR_WIDTH), UI_BORED_BAR_HEIGHT, false);
    dd_fill_rect(UI_ALONE_BAR_X, UI_ALONE_BAR_Y, get_bar_percentage(p->alone, UI_ALONE_BAR_WIDTH), UI_ALONE_BAR_HEIGHT, false);
}

int main(void)
{
    // Initialize the 1ms timekeeping system
    SysTick_Init(16000000);

    // Initialize all hardware (Clocks, GPIOs, SPI, DMA, OLED) completely behind the scenes
    DD_Init();
    Input_Init();

    Pet Miky = {
        .food = PET_MAX_STAT_VALUE,
        .bored = PET_MIN_STAT_VALUE,
        .alone = PET_MIN_STAT_VALUE,

        .food_status_severity = 0,
        .bored_status_severity = 0,
        .alone_status_severity = 0,

        .food_change_factor = 50,
        .bored_change_factor = 50,
        .alone_change_factor = 50,

        .food_change_time_ms = 270000,
        .bored_change_time_ms = 900000,
        .alone_change_time_ms = 1800000,

        .last_time_fed = 0,
        .last_time_played_with = 0,
        .last_time_pet = 0,

        .currentActivity = ACTIVITY_IDLE,

        .emotion_array = miky_emotions,
        .currentEmotion = EMOTION_HAPPY,
        .alive = true};

    while (1)
    {
        if (Miky.alive)
        {

            Input_Update();
            // Check buttons and tweak stats based on it
            if (g_buttons.feed_flag)
            {
                g_buttons.feed_flag = false;
                Pet_Eat(&Miky);
            }
            if (g_buttons.pet_flag)
            {
                g_buttons.pet_flag = false;
                Pet_Pet(&Miky);
            }
            if (g_buttons.play_flag)
            {
                g_buttons.play_flag = false;
                Pet_Play(&Miky);
            }
            
            Pet_UpdateStats(&Miky);
            Pet_Transition(&Miky);
            stat_bars_update(&Miky);
            dd_draw_bitmap(28, 16, MIKY_WIDTH, MIKY_HEIGHT, Miky.emotion_array[Miky.currentEmotion], true);
            dd_update();
        }
        __WFI();
    }
}