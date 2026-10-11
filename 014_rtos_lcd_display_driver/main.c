#include <stdint.h>
#include <stdbool.h>

#include "ili9341_hw.h"
#include "ili9341_gfx.h"

#include "FreeRTOS.h"
#include "task.h"

#define UI_REFRESH_RATE_MS 17

// Standard RGB565 Color Definitions
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F

void vRenderTask(void *rgs)
{
    ILI9341_HW_Init();

    // bg color
    ILI9341_Clear(COLOR_BLACK);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(UI_REFRESH_RATE_MS);

    int box_x = 10;
    int direction = 2;
    bool blink_state = false;

    while (1)
    {
        // --- Heartbeat 1: Blinking status indicator in top right ---
        ILI9341_FillRect(300, 10, 10, 10, blink_state ? COLOR_GREEN : COLOR_BLACK);

        // --- Heartbeat 2: Bouncing box to test DMA streaming speed ---
        ILI9341_FillRect(box_x, 100, 30, 30, COLOR_BLACK);

        // Update physics
        box_x += direction;
        if (box_x >= (320 - 30) || box_x <= 0)
        {
            direction = -direction;
        }

        // Draw new position
        ILI9341_FillRect(box_x, 100, 30, 30, COLOR_RED);

        // Toggle blink state every ~30 frames
        static int frame_count = 0;
        if (++frame_count >= 30)
        {
            blink_state = !blink_state;
            frame_count = 0;
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

int main(void)
{
    xTaskCreate(vRenderTask, "render", configMINIMAL_STACK_SIZE + 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    }
    return 0;
}