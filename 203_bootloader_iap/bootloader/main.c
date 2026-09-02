#include "stm32g491xx.h"
#include "device_drivers/display_driver/ddriver.h"
#include "device_drivers/display_driver/ui_widgets.h"
#include "device_drivers/tick_engine/systick_timer.h"

#include "local_source/button.h"
#include "local_source/usart.h"
#include "local_source/flash.h"

#include <stdint.h>

#define APP_ADDRESS 0x08008000

bool jump_to_app(void)
{
    // Read the first 4 bytes of the application vector table,
    // as that should be the stack pointer
    uint32_t app_stack_pointer = *(volatile uint32_t *)APP_ADDRESS;

    // Sanity check - SP is within the 112K RAM bounds
    if (app_stack_pointer < 0x20000000 || app_stack_pointer > 0x2001C000)
    {
        return false; // Not a valid application
    }

    uint32_t app_reset_handler_address = *(volatile uint32_t *)(APP_ADDRESS + 4);
    void (*app_reset_handler)(void) = (void (*)(void))app_reset_handler_address;

    // PREPARING TO PASS CONTROL TO APP

    // Disable SysTick timer and clear its registers
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    // Disable interrupts before jumping
    __disable_irq();
    // Clear all pending interrupts in the NVIC
    for (int i = 0; i < 8; i++)
    {
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    // Assembly to reset main stack pointer to the start of the
    // app
    __asm volatile("MSR msp, %0" : : "r"(app_stack_pointer) :);
    // Call reset handler of the app to run its routine before
    // it enters its main

    // (in this routine, in addition to zeroing of ram etc,
    //  app must itself reconfigure address of
    // the interrupt vector table to point to its own, otherwise
    // next time interrupt fires, it would be serviced with bootloaders
    // table)
    app_reset_handler();

    // this should never trigger
    return true;
}

int main(void)
{
    SysTick_Init(16000000);
    Button_Init();
    USART2_Init();
    DD_Init();

    dd_clear();

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
    bool update_requested = false;
    while (!SysTick_IsElapsed(flash_grace_period, 6000))
    {
        if (SysTick_IsElapsed(last_second, 1000))
        {
            last_second = GetTick();
            dd_clear();
            ui_draw_loadbar(&lbar);
            lbar.val -= ((255 / 5));
            ui_draw_string(&text1, "PRESS BTN");
            ui_draw_string(&text2, "TO FLASH APP");
            dd_update();
        }

        if (Button_IsPressed())
        {
            update_requested = true;
            // Exit the 6-second grace period
            break; 
        }

        __WFI();
    };

    if (update_requested)
    {
        dd_clear();
        ui_draw_string(&text1, "PROVIDE .BIN");
        ui_draw_string(&text2, "SEND VIA USART");
        dd_update();

        USART2_DMA_Start();

        while (!rx_transfer_complete)
        {
            __WFI();
        }

        dd_clear();
        ui_draw_string(&text1, "DOWNLOAD DONE!");
        ui_draw_string(&text2, "WRITING FLASH...");
        dd_update();


        extern volatile uint32_t bytes_received; 
        
        if (Flash_Write_App(rx_buffer, bytes_received)) {
            dd_clear();
            ui_draw_string(&text1, "FLASH SUCCESS!");
            ui_draw_string(&text2, "BOOTING...");
            dd_update();
        } else {
            dd_clear();
            ui_draw_string(&text1, "FLASH FAILED!");
            dd_update();
            // Halt on critical flash error
            while(1); 
        }
        
    }

    // we hope to never return from this
    jump_to_app(); 
    
    // Failed to load app
    while (1)
    {
        dd_clear();
        dd_fill_rect(0, 0, DD_WIDTH, DD_HEIGHT, text1.fill_state);
        text1.fill_state = !text1.fill_state;
        text2.fill_state = !text2.fill_state;
        ui_draw_string(&text1, "NO DEF. APP!");
        ui_draw_string(&text2, "RESET AND FLASH");
        dd_update();
        delay_ms(1000);
    }
}