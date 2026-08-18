#ifndef USART_DMA_H
#define USART_DMA_H

#include <stdint.h>
#include <stdbool.h>

// 6 bytes reserved for metadata
#define FRAME_SIZE 1030
#define PAYLOAD_SIZE 1024

// Initialize USART2 and DMA1 Channel 2 for circular reception
// pclk_freq is the APB1 timer clock
void USART2_DMA_Init(uint32_t pclk_freq);

// Extract the next valid frame from the DMA ring buffer
// out_buffer must be at least PAYLOAD_SIZE (1024) bytes.
// Returns true if a valid frame was extracted, false otherwise.
bool Stream_ExtractFrame(uint8_t* out_buffer);

#endif // USART_DMA_H