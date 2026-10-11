#ifndef ILI9341_HW_H
#define ILI9341_HW_H

#include <stdint.h>
#include <stdbool.h>

// Number of pixels to send per DMA chunk.
// 1024 pixels = 2048 bytes of SRAM for the chunk array.
#define ILI9341_CHUNK_SIZE 1024

// GPIO, SPI, DMA, and LCD initialization
void ILI9341_HW_Init(void);

// Defines a drawing window on the ILI9341 hardware
void ILI9341_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

// Streams a single color over DMA for a specified number of pixels
void ILI9341_FillDMA(uint16_t color, uint32_t pixel_count);

void ILI9341_SendCommand(uint8_t cmd);
void ILI9341_SendData(uint8_t data);

void SPI_SendByte(uint8_t data);

#endif // ILI9341_HW_H