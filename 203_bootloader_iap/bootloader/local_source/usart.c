#include "usart.h"

uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_transfer_complete = 0;
volatile uint32_t bytes_received = 0;

void USART2_Init(void)
{
    // Enable Clocks (GPIOA, USART2, DMA1, DMAMUX)
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;

    // Configure PA2 (TX) and PA3 (RX) for Alternate Function 7 (USART2)
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1); // AF mode
    GPIOA->AFR[0] |= (7 << GPIO_AFRL_AFSEL2_Pos) | (7 << GPIO_AFRL_AFSEL3_Pos);

    // Configure USART2 (115200 baud @ 16MHz)
    USART2->BRR = 139;                                         // 16,000,000 / 115200 = 138.88
    USART2->CR3 |= USART_CR3_DMAR;                             // Enable DMA for Reception
    USART2->CR1 |= USART_CR1_IDLEIE;                           // Enable IDLE line interrupt
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_UE; // Enable TX, RX, USART

    // Configure DMA1 Channel 2 for RX
    DMA1_Channel2->CCR &= ~DMA_CCR_EN;            // Ensure disabled before config
    DMA1_Channel2->CPAR = (uint32_t)&USART2->RDR; // Peripheral address
    DMA1_Channel2->CMAR = (uint32_t)rx_buffer;    // Memory address
    DMA1_Channel2->CNDTR = RX_BUFFER_SIZE;        // Transfer size
    DMA1_Channel2->CCR |= DMA_CCR_MINC;           // Memory increment mode

    // Route USART2_RX (Request 26) to DMA1_Channel2 via DMAMUX
    DMAMUX1_Channel1->CCR = 26; // Channel 1 in DMAMUX corresponds to DMA1_CH2

    // Enable USART2 Interrupt in NVIC (for IDLE line detection)
    NVIC_EnableIRQ(USART2_IRQn);
}

void USART2_DMA_Start(void)
{
    DMA1_Channel2->CCR |= DMA_CCR_EN; // Start listening!s
}

void USART2_IRQHandler(void)
{
    // Check if the IDLE flag is set
    if (USART2->ISR & USART_ISR_IDLE)
    {
        // Clear the IDLE flag by writing to the Interrupt Clear Register
        USART2->ICR |= USART_ICR_IDLECF;

        uint32_t bytes_left = DMA1_Channel2->CNDTR;

        // If counter is unchanged, no data has arrived yet
        if (bytes_left == RX_BUFFER_SIZE)
        {
            return;
        }

        // Stop the DMA
        DMA1_Channel2->CCR &= ~DMA_CCR_EN;

        // Calculate exactly how many bytes we caught
        // CNDTR counts DOWN from RX_BUFFER_SIZE.
        bytes_received = RX_BUFFER_SIZE - DMA1_Channel2->CNDTR;

        rx_transfer_complete = 1;
    }
}