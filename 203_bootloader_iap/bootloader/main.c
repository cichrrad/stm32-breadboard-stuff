#include "stm32g491xx.h"
#include "device_drivers/display_driver/ddriver.h"
#include "device_drivers/display_driver/ui_widgets.h"
#include "device_drivers/tick_engine/systick_timer.h"
#include <stdint.h>

#define APP_ADDRESS 0x08008000

void jump_to_app(void)
{
    uint32_t app_reset_handler_address = *(volatile uint32_t *)(APP_ADDRESS + 4);
    void (*app_reset_handler)(void) = (void (*)(void))app_reset_handler_address;
    uint32_t app_stack_pointer = *(volatile uint32_t *)APP_ADDRESS;

    // Disable interrupts before jumping to be safe!
    __disable_irq();

    __asm volatile("MSR msp, %0" : : "r"(app_stack_pointer) :);
    app_reset_handler();
}

int main(void)
{
    SysTick_Init(16000000);
    DD_Init();
    UITextType text1 = {
        .x = 20,
        .y = 0,
        .fill_state = true,
        .solid_bg = true,
        .val = "text1"};
    UITextType text2 = {
        .x = 5,
        .y = 20,
        .fill_state = true,
        .solid_bg = true,
        .val = "text2"};

    UILoadBarType lbar = {
        .x = 8,
        .y = 48,
        .width = 112,
        .height = 8,
        .fill_state = true,
        .val = 255,
    };
    ui_draw_loadbar(&lbar);

    // 6 second window, where we wait for button press
    // to initiate flash of new app
    uint32_t flash_grace_period = GetTick();
    uint32_t last_second = flash_grace_period;
    while (!SysTick_IsElapsed(flash_grace_period, 6000))
    {
        if (SysTick_IsElapsed(last_second, 1000))
        {
            last_second = GetTick();
            dd_clear();
            lbar.val -= ((255 / 5));
            ui_draw_loadbar(&lbar);
            ui_draw_string(&text1, "HOLD BUTTON");
            ui_draw_string(&text2, "TO FLASH NEW APP");
            dd_update();
        }

        // if gpio pin shows us holding the button
        // jump into blocking loop waiting for binary
        // to arrive over usart

        __WFI();
    };
    lbar.val = 0;

    // No app flashed and no default app from previous flash
    while (1)
    {
        dd_clear();
        dd_fill_rect(0, 0, DD_WIDTH, DD_HEIGHT, text1.fill_state);
        text1.fill_state = !text1.fill_state;
        text2.fill_state = !text2.fill_state;
        ui_draw_string(&text1, "NO DEF. APP");
        ui_draw_string(&text2, "RESET AND FLASH");
        dd_update();
        delay_ms(1000);
    }
}