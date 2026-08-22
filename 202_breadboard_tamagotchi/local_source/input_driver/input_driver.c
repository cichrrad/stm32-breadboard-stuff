#include "input_driver.h"
#include "device_headers/stm32g491xx.h"
#include "../tick_engine/systick_timer.h"

ButtonState g_buttons = {false, false, false};

// History shift registers
static uint8_t hist_pet = 0;
static uint8_t hist_feed = 0;
static uint8_t hist_play = 0;
static uint32_t last_poll_tick = 0;

void Input_Init(void)
{
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
}

void Input_Update(void)
{
    if (!SysTick_IsElapsed(last_poll_tick, 10))
        return;
    last_poll_tick = GetTick();

    // Read inverted logic (0 means pressed due to pull-up)
    bool pet_raw = !(GPIOA->IDR & (1 << 1));
    bool feed_raw = !(GPIOA->IDR & (1 << 4));
    bool play_raw = !(GPIOB->IDR & (1 << 0));

    // Shift in new readings
    hist_pet = (hist_pet << 1) | pet_raw;
    hist_feed = (hist_feed << 1) | feed_raw;
    hist_play = (hist_play << 1) | play_raw;

    if (hist_pet == 0x7F)
    {
        g_buttons.pet_flag = true;
    }
    if (hist_feed == 0x7F)
    {
        g_buttons.feed_flag = true;
    }
    if (hist_play == 0x7F)
    {
        g_buttons.play_flag = true;
    }
}