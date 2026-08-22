#include "systick_timer.h"

/* Global volatile tick counter incremented in the ISR */
static volatile uint32_t g_system_ticks = 0;

bool SysTick_Init(uint32_t system_clock_hz)
{
    // Calculate the reload value for 1ms interrupts
    uint32_t reload_value = (system_clock_hz / 1000) - 1;

    // The Cortex-M SysTick is a 24-bit down-counter. 
    // SysTick_LOAD_RELOAD_Msk is provided by CMSIS (0xFFFFFFUL)
    if (reload_value > SysTick_LOAD_RELOAD_Msk)
    {
        return false; 
    }

    // Disable SysTick before configuration
    SysTick->CTRL = 0;

    // Set the reload value
    SysTick->LOAD = reload_value;

    // Reset the current counter value
    SysTick->VAL = 0;

    // Start SysTick: Use processor clock, enable interrupts, enable the counter
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk | 
                     SysTick_CTRL_TICKINT_Msk | 
                     SysTick_CTRL_ENABLE_Msk);

    /* 
     * NOTE: CMSIS core also provides an inline helper function that handles 
     * everything above (and sets the NVIC priority to the lowest level). 
     * 
     * Everything above could alternatively be replaced with:
     * 
     * if (SysTick_Config(system_clock_hz / 1000)) { return false; }
     * return true;
     */

    return true;
}

uint32_t GetTick(void)
{
    // Reading a 32-bit aligned variable takes a single 
    // memory cycle, so it is inherently atomic.
    return g_system_ticks;
}

bool SysTick_IsElapsed(uint32_t start_tick, uint32_t delay_ms)
{
    // Unsigned 32-bit integer arithmetic safely handles the tick counter rollover
    return ((GetTick() - start_tick) >= delay_ms);
}

/**
 * @brief  SysTick Interrupt Service Routine
 */
void SysTick_Handler(void)
{
    g_system_ticks++;
}

void delay_ms(uint32_t ms){
    uint32_t start = GetTick();
    while(!SysTick_IsElapsed(start,ms)){
        __NOP();
    }
}