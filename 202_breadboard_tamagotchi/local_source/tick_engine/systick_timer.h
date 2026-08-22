#ifndef SYSTICK_TIMER_H
#define SYSTICK_TIMER_H

#include <stdint.h>
#include <stdbool.h>
#include "device_headers/stm32g491xx.h"


/**
 * @brief  Initializes the SysTick peripheral to generate an interrupt every 1 ms.
 * @param  system_clock_hz: The current core clock frequency in Hz.
 *         (e.g., 170000000 for 170 MHz on STM32G491RE)
 * @return true if initialization succeeded, false if the clock is too fast for the 24-bit timer.
 */
bool SysTick_Init(uint32_t system_clock_hz);

/**
 * @brief  Retrieves the current system tick value.
 * @note   On a 32-bit Cortex-M processor, reading a 32-bit aligned variable is naturally atomic.
 * @return The number of milliseconds elapsed since SysTick_Init was called.
 */
uint32_t GetTick(void);

/**
 * @brief  Non-blocking delay check. Safely handles 32-bit integer rollover.
 * @param  start_tick: The tick value recorded at the start of the interval.
 * @param  delay_ms: The desired delay interval in milliseconds.
 * @return true if the delay has elapsed, false otherwise.
 * 
 * @example
 *      uint32_t last_time = GetTick();
 *      if (SysTick_IsElapsed(last_time, 100)) {
 *          last_time = GetTick();
 *          // Do something every 100ms without blocking!
 *      }
 */
bool SysTick_IsElapsed(uint32_t start_tick, uint32_t delay_ms);

void delay_ms(uint32_t ms);

#endif // SYSTICK_TIMER_H