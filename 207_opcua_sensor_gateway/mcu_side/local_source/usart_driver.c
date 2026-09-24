#include "usart_driver.h"

void USART2_Init(uint32_t baud_rate, uint32_t pclk_freq)
{
    // 1. Enable clocks for GPIOA and USART2
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;

    // PA2 (TX) and PA3 (RX) for Alternate Function 7 -- for USART2
    // Clear mode bits for PA2 and PA3
    GPIOA->MODER &= ~(GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
    // Set to Alternate Function mode (10 in binary is 2U)
    GPIOA->MODER |= (2U << GPIO_MODER_MODE2_Pos) | (2U << GPIO_MODER_MODE3_Pos);

    // Clear and set Alternate Function to AF7 for PA2 and PA3 (AFR[0] is AFRL)
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk);
    GPIOA->AFR[0] |= (7U << GPIO_AFRL_AFSEL2_Pos) | (7U << GPIO_AFRL_AFSEL3_Pos);

    // Optional but good practice: set pins to high speed
    GPIOA->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED2_Pos) | (3U << GPIO_OSPEEDR_OSPEED3_Pos);

    // Disable USART before configuration
    USART2->CR1 &= ~USART_CR1_UE;

    // Set Baud Rate. G4 uses standard BRR formula: USARTDIV = fCK / baud
    // Assuming oversampling by 16 (default)
    USART2->BRR = pclk_freq / baud_rate;

    // Configure CR1: 8 data bits, 1 start bit, 1 stop bit, no parity (Defaults are mostly 0)
    // Enable Transmitter (TE) and Receiver (RE)
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;

    // Enable USART2
    USART2->CR1 |= USART_CR1_UE;
}

void USART2_Putc(char c)
{
    // Wait until Transmit Data Register is empty (TXE)
    while (!(USART2->ISR & USART_ISR_TXE_TXFNF)); 
    
    USART2->TDR = (uint8_t)c;
}

void USART2_Send(const uint8_t *data, uint16_t length)
{
    for (uint16_t i = 0; i < length; i++)
    {
        USART2_Putc((char)data[i]);
    }
    
    // Wait until transmission is completely finished (TC) before exiting
    while (!(USART2->ISR & USART_ISR_TC));
}

void USART2_SendString(const char *str)
{
    while (*str)
    {
        USART2_Putc(*str++);
    }
    
    // Wait for Transmission Complete
    while (!(USART2->ISR & USART_ISR_TC));
}