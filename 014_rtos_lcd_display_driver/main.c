#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ili9341.h"
#include "hw_spi.h"

#include "stm32g491xx.h"

#include "FreeRTOS.h"
#include "task.h"

#define UI_REFRESH_RATE_MS 17 // ~60 FPS
#define GB_WIDTH 160
#define GB_HEIGHT 144

#define MOVE_FACTOR 1

// Dummy 8-bit framebuffer to simulate the Game Boy emulator output
static uint8_t dummy_gb_frame[GB_WIDTH * GB_HEIGHT];

// 16-bit buffer for DMA transfer. 46,080 bytes fits easily in the 112KB RAM.
static uint16_t gb_dma_buffer[GB_WIDTH * GB_HEIGHT];

// Swapped palette for Little-Endian DMA to Big-Endian ILI9341
static const uint16_t GB_PALETTE_SWAPPED[4] = {
    0x318E, // 0: Lightest green (Background)
    0x4E6D, // 1: Light green
    0x2934, // 2: Dark green
    0x8311  // 3: Darkest green (Moving object)
};

void Test_RenderFrame(void)
{
    // 1. Translate 8-bit dummy frame to 16-bit DMA buffer
    for (uint32_t i = 0; i < (GB_WIDTH * GB_HEIGHT); i++)
    {
        gb_dma_buffer[i] = GB_PALETTE_SWAPPED[dummy_gb_frame[i] & 0x03];
    }

    // 2. Set the drawing window centered on the 240x320 screen
    uint16_t x_offset = (ILI9341_WIDTH - GB_WIDTH) / 2;
    uint16_t y_offset = (ILI9341_HEIGHT - GB_HEIGHT) / 2;
    ILI9341_SetWindow(x_offset, y_offset, x_offset + GB_WIDTH - 1, y_offset + GB_HEIGHT - 1);

    // 3. Blast the frame over DMA.
    // The task will block here using the FreeRTOS semaphore until the transfer finishes.
    HW_SPI_SendFrame_DMA((uint8_t *)gb_dma_buffer, sizeof(gb_dma_buffer));
}

void vRenderTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(UI_REFRESH_RATE_MS);

    ILI9341_Init();

    // Clear the physical screen to black once so the background outside the 160x144 window is clean
    ILI9341_FillRect(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, COLOR_BLACK);

    uint16_t y_pos = 0;
    int curr_color = 0;
    bool moving_down = true;

    while (1)
    {
        // 1. Clear the dummy 8-bit framebuffer to Color 0 (Background)
        memset(dummy_gb_frame, 0, sizeof(dummy_gb_frame));

        // 2. Draw a moving 40x40 square in the dummy buffer using Color 3
        for (int r = 0; r < 40; r++)
        {
            for (int c = 0; c < 40; c++)
            {
                int draw_y = y_pos + r;
                int draw_x = 60 + c; // Fixed X position for testing
                if (draw_y < GB_HEIGHT && draw_x < GB_WIDTH)
                {
                    dummy_gb_frame[draw_y * GB_WIDTH + draw_x] = curr_color++;
                    if (curr_color > 3)
                    {
                        curr_color = 0;
                    }
                }
            }
        }

        // Update position logic for the next frame
        if (moving_down)
        {
            y_pos += MOVE_FACTOR;
            if (y_pos >= (GB_HEIGHT - 40))
                moving_down = false;
        }
        else
        {
            y_pos -= MOVE_FACTOR;
            if (y_pos == 0)
                moving_down = true;
        }

        // 3. Render the entire frame via DMA
        Test_RenderFrame();

        // 4. Yield CPU until next 17ms interval is hit
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void SystemClock_Config(void)
{
    // 1. Enable Flash prefetch and set 5 Wait States for 170MHz
    FLASH->ACR |= FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_5WS;

    // 2. Enable High Speed Internal (HSI) 16MHz clock
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY))
        ;

    // 3. Configure PLL: Source = HSI (16MHz)
    // Formula: (HSI / M) * N / R = (16 / 4) * 85 / 2 = 170 MHz
    RCC->PLLCFGR = (2U << RCC_PLLCFGR_PLLSRC_Pos) | // HSI source
                   (3U << RCC_PLLCFGR_PLLM_Pos) |   // M = 4 (Value is M-1)
                   (85U << RCC_PLLCFGR_PLLN_Pos) |  // N = 85
                   RCC_PLLCFGR_PLLREN;              // Enable R output (default R=2)

    // 4. Enable PLL and wait for it to lock
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY))
        ;

    // 5. Switch system clock source to PLL
    RCC->CFGR |= (3U << RCC_CFGR_SW_Pos);
    while ((RCC->CFGR & RCC_CFGR_SWS) != (3U << RCC_CFGR_SWS_Pos))
        ;
}

int main(void)
{
    SystemClock_Config();
    // configMINIMAL_STACK_SIZE + 256 provides enough space for task tracking and standard variables
    xTaskCreate(vRenderTask, "render", configMINIMAL_STACK_SIZE + 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    }
    return 0;
}