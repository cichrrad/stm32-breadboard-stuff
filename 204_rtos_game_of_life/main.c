#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "ddriver.h"
#include "ui_widgets.h"

#include "local_source/compute.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

static uint8_t world_grid_A[GOL_WIDTH][GOL_HEIGHT];
static uint8_t world_grid_B[GOL_WIDTH][GOL_HEIGHT];

static uint8_t (*current_grid)[GOL_HEIGHT];
static uint8_t (*next_grid)[GOL_HEIGHT];

TaskHandle_t xComputeTaskHandle = NULL;
SemaphoreHandle_t xFrameReadySemaphore = NULL;

static UITextType ui_banner = {
    .val = "G.O.L.",
    .x = 5,
    .y = 5,
    .fill_state = true,
    .solid_bg = false,
};

static UITextType ui_fps = {
    .val = "FPS:--",
    .x = 80,
    .y = 5,
    .fill_state = true,
    .solid_bg = false,
};

// helper for fps counter, works for 0-99
void update_fps_string(int fps, char *buf)
{
    buf[4] = (fps / 10) + '0';
    buf[5] = (fps % 10) + '0';
}

void vComputeTask(void *pvParameters)
{
    current_grid = world_grid_A;
    next_grid = world_grid_B;
    // generation 0
    rng_populate_grid(current_grid);

    while (1)
    {
        // Compute the next generation based on current_grid
        compute_next_gen(current_grid, next_grid);

        // Swap
        void *temp = current_grid;
        current_grid = next_grid;
        next_grid = temp;

        // Signal Render task to draw new frame
        xSemaphoreGive(xFrameReadySemaphore);

        // Zzzz until Render task signals it needs
        // this task to calculate new frame
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

void vRenderTask(void *pvParameters)
{
    DD_Init();
    bool hbeat = true;

    TickType_t last_time = xTaskGetTickCount();
    int frame_count = 0;
    char fps_buffer[7] = "FPS:--"; // 6 + null terminator
    ui_fps.val = fps_buffer;

    xTaskNotifyGive(xComputeTaskHandle);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(67); // ~15FPS target
    // (Last time I checked, we can push 60+ FPS, but with
    // speed being tied to FPS, it becomes worse than better
    // to watch)

    while (1)
    {
        // Blocks this task until compute task is done
        // calculating frame
        xSemaphoreTake(xFrameReadySemaphore, portMAX_DELAY);

        // Notify compute task to start on next frame
        // while this one is being rendered
        xTaskNotifyGive(xComputeTaskHandle);
        // (this can be done right away, as
        // compute task and render task
        // will both just read the old gen.)

        dd_clear();

        // UI FPS calculation
        frame_count++;
        TickType_t current_time = xTaskGetTickCount();
        if ((current_time - last_time) >= pdMS_TO_TICKS(1000))
        {
            update_fps_string(frame_count, fps_buffer);
            frame_count = 0;
            last_time = current_time;
        }

        // UI rendering
        ui_draw_string(&ui_banner, ui_banner.val);
        ui_draw_string(&ui_fps, ui_fps.val);
        dd_fill_circle(0, 0, 1, hbeat);
        hbeat = !hbeat;
        dd_draw_rect(4, 17, 120, 40, true);

        // Viewport rendering
        for (int x = 0; x < GOL_WIDTH; x++)
        {
            for (int y = 0; y < GOL_HEIGHT; y++)
            {
                if (current_grid[x][y])
                {
                    // 4 and 17 to offset the viewport
                    dd_set_pixel(4 + x, 17 + y, true);
                }
            }
        }

        // Start sending with DMA via SPI to the display
        dd_update();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

int main(void)
{

    xFrameReadySemaphore = xSemaphoreCreateBinary();
    xTaskCreate(vComputeTask, "compute", 256, NULL, 1, &xComputeTaskHandle);
    xTaskCreate(vRenderTask, "render", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    };
    return 0;
}