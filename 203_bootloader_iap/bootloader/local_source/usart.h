#ifndef USART_H
#define USART_H

#include "stm32g491xx.h"
#include <stdint.h>

#define RX_BUFFER_SIZE 32768

extern uint8_t rx_buffer[RX_BUFFER_SIZE];
extern volatile uint8_t rx_transfer_complete;

void USART2_Init(void);
void USART2_DMA_Start(void);

#endif