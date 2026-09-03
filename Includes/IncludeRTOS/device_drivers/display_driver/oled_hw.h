#ifndef OLED_HW_H
#define OLED_HW_H

#include <stdint.h>
#include <stdbool.h>

// Expose the busy flag so the graphics engine can check it
extern volatile bool dma_busy;

// Consolidates SPI, OLED, and DMA initialization
void OLED_HW_Init(void);

// Triggers the DMA transfer for the active buffer
void OLED_Update_DMA(const uint8_t *framebuffer);

void OLED_SendCommand(uint8_t cmd);
void OLED_SendData(uint8_t data);

#endif // OLED_HW_H