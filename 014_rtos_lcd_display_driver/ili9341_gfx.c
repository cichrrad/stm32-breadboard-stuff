#include "ili9341_gfx.h"
#include "ili9341_hw.h"

void ILI9341_DrawPixel(int x, int y, uint16_t color)
{
    if (x < 0 || x >= ILI9341_WIDTH || y < 0 || y >= ILI9341_HEIGHT)
        return;
    ILI9341_SetWindow(x, y, x, y);

    GPIOA->BSRR = GPIO_BSRR_BS9;
    GPIOB->BSRR = GPIO_BSRR_BR6;
    SPI_SendByte(color >> 8);
    SPI_SendByte(color & 0xFF);
    GPIOB->BSRR = GPIO_BSRR_BS6;
}

void ILI9341_FillRect(int x, int y, int w, int h, uint16_t color)
{
    // Boundary checks
    if (x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT)
        return;
    if (x + w > ILI9341_WIDTH)
        w = ILI9341_WIDTH - x;
    if (y + h > ILI9341_HEIGHT)
        h = ILI9341_HEIGHT - y;

    ILI9341_SetWindow(x, y, x + w - 1, y + h - 1);
    ILI9341_FillDMA(color, w * h);
}

void ILI9341_Clear(uint16_t color)
{
    ILI9341_FillRect(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}