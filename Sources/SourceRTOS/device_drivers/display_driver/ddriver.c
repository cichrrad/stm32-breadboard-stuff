#include "ddriver.h"
#include "oled_hw.h"
#include "utils_math.h"
#include "fonts.h"
#include "stm32g491xx.h"

#include "FreeRTOS.h"
#include "task.h"

static uint8_t buffer_A[DD_FB_SIZE] = {0};
static uint8_t buffer_B[DD_FB_SIZE] = {0};

// Pointer to the buffer we are currently drawing into
static uint8_t *draw_buffer = buffer_A;

void DD_Init()
{
    OLED_HW_Init(); // Calls SPI_Init, OLED_Init, DMA_Init internally
    xTaskNotifyGive(xTaskGetCurrentTaskHandle());
}

uint32_t dd_euclidian_distance(int ax, int ay, int bx, int by)
{
    uint32_t x = (ax - bx) * (ax - bx);
    uint32_t y = (ay - by) * (ay - by);

    return utils_sqrt(x + y);
}

void dd_set_pixel(int x, int y, bool state)
{
    if (x < 0 || x >= DD_WIDTH || y < 0 || y >= DD_HEIGHT)
        return;
    int idx = x + ((y / 8) * DD_WIDTH);

    if (state)
        draw_buffer[idx] |= (0x01 << (y & 0x07));
    else
        draw_buffer[idx] &= ~(0x01 << (y & 0x07));
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

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void dd_draw_line(int ax, int ay, int bx, int by, bool state)
{
    int dx = utils_abs(bx - ax);
    int dy = -utils_abs(by - ay);
    int sx = ax < bx ? 1 : -1;
    int sy = ay < by ? 1 : -1;
    int err = dx + dy;

    while (true)
    {
        dd_set_pixel(ax, ay, state);
        if (ax == bx && ay == by)
            break;
        int e2 = 2 * err;
        if (e2 >= dy)
        {
            err += dy;
            ax += sx;
        }
        if (e2 <= dx)
        {
            err += dx;
            ay += sy;
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

void dd_fill_circle(int x, int y, int radius, bool state)
{
    // calculate top-left corner from center [x,y]
    int cornerX = x - radius;
    int cornerY = y - radius;

    // go over the square, filling only pixels <= than radius
    for (int curr_y = cornerY; curr_y <= cornerY + 2 * radius; curr_y++)
    {
        for (int curr_x = cornerX; curr_x <= cornerX + 2 * radius; curr_x++)
        {
            if (dd_euclidian_distance(x, y, curr_x, curr_y) <= radius)
            {
                dd_set_pixel(curr_x, curr_y, state);
            }
        }
    }
}

void dd_draw_circle(int x, int y, int radius, bool state)
{
    // calculate top-left corner from center [x,y]
    int cornerX = x - radius;
    int cornerY = y - radius;

    // go over the square, filling only pixels <= than radius
    for (int curr_y = cornerY; curr_y <= cornerY + 2 * radius; curr_y++)
    {
        for (int curr_x = cornerX; curr_x <= cornerX + 2 * radius; curr_x++)
        {
            if (dd_euclidian_distance(x, y, curr_x, curr_y) == radius)
            {
                dd_set_pixel(curr_x, curr_y, state);
            }
        }
    }
}

void dd_draw_triangle(int ax, int ay, int bx, int by, int cx, int cy, bool state)
{
    dd_draw_line(ax, ay, bx, by, state);
    dd_draw_line(bx, by, cx, cy, state);
    dd_draw_line(cx, cy, ax, ay, state);
}

void dd_fill_triangle(int ax, int ay, int bx, int by, int cx, int cy, bool state)
{
    int minX = utils_min3(ax, bx, cx);
    int minY = utils_min3(ay, by, cy);
    int maxX = utils_max3(ax, bx, cx);
    int maxY = utils_max3(ay, by, cy);

    // Scan bounding box and check edge functions
    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {

            int w0 = (bx - ax) * (y - ay) - (by - ay) * (x - ax);
            int w1 = (cx - bx) * (y - by) - (cy - by) * (x - bx);
            int w2 = (ax - cx) * (y - cy) - (ay - cy) * (x - cx);

            // If signs match (all positive or all negative), pixel is inside
            if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0))
            {
                dd_set_pixel(x, y, state);
            }
        }
    }
}

// TODO: use draw_line now that we have it
void dd_draw_rect(int x, int y, int width, int height, bool state)
{
    // draw horizontal lines
    for (int i = 0; i < 2; i++)
    {
        for (int j = x; j <= x + width; j++)
        {
            dd_set_pixel(j, y + i * height, state);
        }
    }

    // draw vertical lines
    for (int i = 0; i < 2; i++)
    {
        for (int j = y; j <= y + height; j++)
        {
            dd_set_pixel(x + i * width, j, state);
        }
    }
}

void dd_update()
{
    // Hand the CURRENT draw buffer to the DMA
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // check if very last bit from last 
    // transmition was sent already
    while ((SPI1->SR & SPI_SR_BSY));
    GPIOB->BSRR = GPIO_BSRR_BS6;

    OLED_Update_DMA(draw_buffer);
    // Swap the draw buffer for the NEXT frame
    draw_buffer = (draw_buffer == buffer_A) ? buffer_B : buffer_A;
}

void dd_clear()
{
    for (int i = 0; i < DD_FB_SIZE; i++)
    {
        draw_buffer[i] = 0x00;
    }
}
