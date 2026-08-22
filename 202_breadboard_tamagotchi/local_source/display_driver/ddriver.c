#include "ddriver.h"
#include "oled_hw.h"

static uint8_t buffer_A[DD_FB_SIZE] = {0};
static uint8_t buffer_B[DD_FB_SIZE] = {0};

// Pointer to the buffer we are currently drawing into
static uint8_t *draw_buffer = buffer_A;

void DD_Init() {
    OLED_HW_Init(); // Calls SPI_Init, OLED_Init, DMA_Init internally
}

void dd_set_pixel(int x, int y, bool state) {
    if (x < 0 || x >= DD_WIDTH || y < 0 || y >= DD_HEIGHT) return;
    int idx = x + ((y / 8) * DD_WIDTH);
    
    if (state) draw_buffer[idx] |= (0x01 << (y & 0x07));
    else       draw_buffer[idx] &= ~(0x01 << (y & 0x07));
}

void dd_draw_bitmap(int x, int y, int width, int height, const uint8_t *bitmap, bool use_msb)
{
    uint8_t mask = 0;
    int size = (width * height) / 8;
    int startX = x;
    int startY = y;

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // If use_msb is true, read bit 7 first (0x80). If false, read bit 0 first (0x01).
            mask = use_msb ? (0x80 >> j) : (0x01 << j);

            dd_set_pixel(x, y, bitmap[i] & mask);
            x++;

            // Wrap-around to new line
            if ((x - startX) == width)
            {
                y++;
                x = startX;
            }

            if ((y - startY) == height || y >= DD_HEIGHT)
            {
                return;
            }
        }
    }
}

void dd_fill_rect(int x, int y, int width, int height, bool state)
{
    for (int curr_y = y; curr_y < y + height; curr_y++)
    {
        for (int curr_x = x; curr_x < x + width; curr_x++)
        {
            // dd_set_pixel already handles our screen boundary checking!
            dd_set_pixel(curr_x, curr_y, state);
        }
    }
}

void dd_update() {
    // Wait for the previous frame to finish transmitting if needed
    while(dma_busy); 

    // Hand the CURRENT draw buffer to the DMA
    OLED_Update_DMA(draw_buffer);
    
    // Swap the draw buffer for the NEXT frame
    draw_buffer = (draw_buffer == buffer_A) ? buffer_B : buffer_A;
}
