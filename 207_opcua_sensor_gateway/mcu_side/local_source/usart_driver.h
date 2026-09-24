#ifndef USART_DRIVER_H
#define USART_DRIVER_H

#include <stdint.h>
#include "stm32g491xx.h"

void USART2_Init(uint32_t baud_rate, uint32_t pclk_freq);

void USART2_Putc(char c);

void USART2_Send(const uint8_t *data, uint16_t length);

void USART2_SendString(const char *str);

#endif // USART_DRIVER_H