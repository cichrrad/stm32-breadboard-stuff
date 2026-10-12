#ifndef HW_SPI_H
#define HW_SPI_H

#include <stdint.h>

void HW_SPI_Init(void);
void HW_Display_Reset(void);
void HW_Display_Backlight(uint8_t state);

// Fundamental communication wrappers
void HW_SPI_SendCmd(uint8_t cmd);
void HW_SPI_SendData8(uint8_t data);
void HW_SPI_SendData16(uint16_t data);

// Bulk transfer for drawing shapes efficiently
void HW_SPI_SendPixels(uint16_t color, uint32_t pixel_count);

#endif // HW_SPI_H