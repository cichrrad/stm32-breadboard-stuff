#include "usart_dma.h"
#include "device_headers/stm32g491xx.h"

#define RX_BUF_SIZE 8240 // 8 frames worth of ring buffer

static volatile uint8_t rx_buffer[RX_BUF_SIZE];
static uint32_t read_idx = 0;

void USART2_DMA_Init(uint32_t pclk_freq) {
    // Enable Clocks: GPIOA, USART2, DMA1, DMAMUX
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;

    // Configure PA2 (TX) and PA3 (RX) for Alternate Function 7 (USART2)
    // Clear MODER for PA2, PA3 then set to AF (10)
    GPIOA->MODER &= ~(GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
    GPIOA->MODER |= (2U << GPIO_MODER_MODE2_Pos) | (2U << GPIO_MODER_MODE3_Pos);
    
    // Set AFRL for PA2 and PA3 to AF7
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2_Msk | GPIO_AFRL_AFSEL3_Msk);
    GPIOA->AFR[0] |= (7U << GPIO_AFRL_AFSEL2_Pos) | (7U << GPIO_AFRL_AFSEL3_Pos);
    
    // High speed output for pins
    GPIOA->OSPEEDR |= (3U << GPIO_OSPEEDR_OSPEED2_Pos) | (3U << GPIO_OSPEEDR_OSPEED3_Pos);

    // Configure USART2 (1'000'000 baud, 8N1)
    USART2->CR1 &= ~USART_CR1_UE;               // Disable USART
    USART2->BRR = pclk_freq / 1000000;          // Set Baudrate
    USART2->CR3 |= USART_CR3_DMAR;              // Enable DMA Receiver
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE; // Enable TX, RX
    USART2->CR1 |= USART_CR1_UE;                // Enable USART

    // Configure DMAMUX (Route USART2_RX to DMA1_Channel2)
    // USART2_RX is DMAMUX Request 26 on STM32G4 (Thanks,Gemini)
    DMAMUX1_Channel1->CCR = 26U;

    // Configure DMA1 Channel 2
    DMA1_Channel2->CCR &= ~DMA_CCR_EN;              // disable DMA
    DMA1_Channel2->CPAR = (uint32_t)&USART2->RDR;   // Read from USART data register
    DMA1_Channel2->CMAR = (uint32_t)rx_buffer;      // Write to ring buffer
    DMA1_Channel2->CNDTR = RX_BUF_SIZE;             // Buffer size
    
    // Config: MINC (Memory Increment), CIRC (Circular), DIR (Periph to Mem), 8-bit
    DMA1_Channel2->CCR = DMA_CCR_MINC | DMA_CCR_CIRC;
    
    // start DMA
    DMA1_Channel2->CCR |= DMA_CCR_EN;
}

static inline uint8_t Read_Byte(uint32_t offset) {
    return rx_buffer[(read_idx + offset) % RX_BUF_SIZE];
}

bool Stream_ExtractFrame(uint8_t* out_buffer) {
    // DMA CNDTR counts down. Calculate current write index.
    uint32_t write_idx = RX_BUF_SIZE - DMA1_Channel2->CNDTR;
    
    // Calculate available bytes
    uint32_t available;
    if (write_idx >= read_idx) {
        available = write_idx - read_idx;
    } else {
        available = RX_BUF_SIZE - read_idx + write_idx;
    }

    // Scan buffer as long as we have at least one full frame's worth of data
    while (available >= FRAME_SIZE) {
        // Look for Magic Header
        if (Read_Byte(0) == 0xBA && Read_Byte(1) == 0xDA &&
            Read_Byte(2) == 0x55 && Read_Byte(3) == 0xAA) {
            
            // Expected packet found - Extract and calculate checksum
            uint8_t calculated_checksum = 0;
            
            for (uint32_t i = 0; i < PAYLOAD_SIZE; i++) {
                uint8_t b = Read_Byte(5 + i); // Offset: 4 (Magic) + 1 (Seq)
                out_buffer[i] = b;
                calculated_checksum ^= b;
            }
            
            uint8_t received_checksum = Read_Byte(FRAME_SIZE - 1);
            
            if (calculated_checksum == received_checksum) {
                // All good - Advance read pointer past this frame
                read_idx = (read_idx + FRAME_SIZE) % RX_BUF_SIZE;
                return true;
            } else {
                // Checksum mismatch (corrupted frame). 
                // Advance 1 byte and let the while loop resync.
                read_idx = (read_idx + 1) % RX_BUF_SIZE;
                available--;
            }
        } else {
            // Not a header, advance search window by 1
            read_idx = (read_idx + 1) % RX_BUF_SIZE;
            available--;
        }
    }
    
    return false; // Not enough data or no valid frame found
}