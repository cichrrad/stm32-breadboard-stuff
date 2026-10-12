#include "ili9341.h"
#include "hw_spi.h"
#include "FreeRTOS.h"
#include "task.h"

void ILI9341_Init(void)
{
    HW_SPI_Init();
    HW_Display_Reset();

    // Software Reset
    HW_SPI_SendCmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Power Control A
    HW_SPI_SendCmd(0xCB);
    HW_SPI_SendData8(0x39);
    HW_SPI_SendData8(0x2C);
    HW_SPI_SendData8(0x00);
    HW_SPI_SendData8(0x34);
    HW_SPI_SendData8(0x02);

    // Pixel Format Set (16-bits per pixel)
    HW_SPI_SendCmd(0x3A);
    HW_SPI_SendData8(0x55);

    // Memory Access Control (Orientation)
    HW_SPI_SendCmd(0x36);
    HW_SPI_SendData8(0x48); // MX, BGR

    // Sleep Out
    HW_SPI_SendCmd(0x11);
    vTaskDelay(pdMS_TO_TICKS(120)); // Strictly required 120ms to exit sleep

    // Display On
    HW_SPI_SendCmd(0x29);

    // Turn on Backlight
    HW_Display_Backlight(1);
}

void ILI9341_SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    // Column Address Set
    HW_SPI_SendCmd(0x2A);
    HW_SPI_SendData16(x1);
    HW_SPI_SendData16(x2);

    // Page Address Set
    HW_SPI_SendCmd(0x2B);
    HW_SPI_SendData16(y1);
    HW_SPI_SendData16(y2);

    // Memory Write
    HW_SPI_SendCmd(0x2C);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
        return;
    ILI9341_SetWindow(x, y, x, y);
    HW_SPI_SendData16(color);
}

void ILI9341_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if ((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
        return;
    if ((x + w - 1) >= ILI9341_WIDTH)
        w = ILI9341_WIDTH - x;
    if ((y + h - 1) >= ILI9341_HEIGHT)
        h = ILI9341_HEIGHT - y;

    ILI9341_SetWindow(x, y, x + w - 1, y + h - 1);
    HW_SPI_SendPixels(color, w * h);
}