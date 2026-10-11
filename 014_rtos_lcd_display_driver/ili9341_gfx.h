#ifndef ILI9341_GFX_H
#define ILI9341_GFX_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32g491xx.h"

// Landscape orientation dimensions
#define ILI9341_WIDTH 320
#define ILI9341_HEIGHT 240

// Core drawing primitives
void ILI9341_DrawPixel(int x, int y, uint16_t color);
void ILI9341_FillRect(int x, int y, int w, int h, uint16_t color);
void ILI9341_Clear(uint16_t color);

#endif // ILI9341_GFX_H