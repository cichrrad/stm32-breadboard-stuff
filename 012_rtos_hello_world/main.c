#include <stdint.h>
#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"

// The task function that will run forever
void vBlinkyTask(void *pvParameters)
{
    while (1)
    {
        // Toggle LED
        if (GPIOA->ODR & (1 << 5))
        {
            GPIOA->BSRR = (1 << (5 + 16));
        }
        else
        {
            GPIOA->BSRR = (1 << (5));
        }
        // Yield CPU for 500 ticks (500 ms)
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    // Clock init for the LED pin
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

    // LED2 pin (A5) init to output (mode 1), Push-pull (0), fast (2), no pull (0)
    GPIO_Init(GPIOA, 5, 1, 0, 2, 0);

    // Allow debug in sleep mode so flashing can be done without
    // holding the button
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;

    // Parameters: function pointer, name, stack size (words), arguments, priority, task handle
    xTaskCreate(vBlinkyTask, "Blinky", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    // spin up FreeRTOS
    vTaskStartScheduler();

    // We should never reach here
    while (1)
        ;

    return 0;
}