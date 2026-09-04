#include <stdbool.h>
#include <stdint.h>

#include "gpio.h"
#include "ddriver.h"
#include "ui_widgets.h"

#include "local_source/compute.h"

#include "FreeRTOS.h"
#include "task.h"

static uint8_t world_grid_A[GOL_WIDTH][GOL_HEIGHT];
static uint8_t world_grid_B[GOL_WIDTH][GOL_HEIGHT];
static uint8_t (*current_grid)[GOL_HEIGHT];

static UITextType ui_banner = {
    .val = "STM32 G.O.L.",
    .x = 15,
    .y = 5,
    .fill_state = true,
    .solid_bg = false,
};

void vRenderTask(void *rgs)
{
    DD_Init();

    bool hbeat = true;
    uint8_t (*read_grid)[GOL_HEIGHT] = world_grid_B;
    current_grid = world_grid_A;

    rng_populate_grid(read_grid);

    while (1)
    {
        dd_clear();
        // UI
        ui_draw_string(&ui_banner, ui_banner.val);
        dd_fill_circle(1, 1, 1, hbeat);
        hbeat = !hbeat;
        dd_draw_rect(4, 17, 120, 40, true);

        // viewport
        compute_next_gen(read_grid, current_grid);
        for (int x = 0; x < GOL_WIDTH; x++)
        {
            for (int y = 0; y < GOL_HEIGHT; y++)
            {
                if(current_grid[x][y]){
                    // 4 and 17 to offset the viewport
                    // withing our designated rectangle
                    dd_set_pixel(4+x,17+y,true);
                }
            }
        }

        // swap grids
        void *temp = read_grid;
        read_grid = current_grid;
        current_grid = temp;
        dd_update();
        vTaskDelay(pdTICKS_TO_MS(17));
    }
}

int main(void)
{

    xTaskCreate(vRenderTask, "render", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    };
    return 0;
}