#include "ddriver.h"

uint8_t DD_FRAMEBUFFER[DD_FB_SIZE] = {0};

void dd_set_pixel(int x, int y, bool state)
{
    if (x < 0 || x >= DD_WIDTH || y < 0 || y >= DD_HEIGHT) return;

    int idx = x + ((y / 8) * DD_WIDTH); 

    if (state)
    {
        DD_FRAMEBUFFER[idx] |= (0x01 << (y & 0x07));
    }
    else
    {
        DD_FRAMEBUFFER[idx] &= ~(0x01 << (y & 0x07));
    }
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
