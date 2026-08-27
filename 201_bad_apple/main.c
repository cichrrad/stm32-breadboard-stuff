#include "device_headers/stm32g491xx.h"
#include "device_drivers/gpio.h"
#include "device_drivers/tick_engine/systick_timer.h"
#include "spi_dma.h"
#include "usart_dma.h"

// Default clock for G491RE
#define SYS_CLOCK_HZ 16000000

// DOUBLE BUFFERING ALLOCATION
static uint8_t frame_A[1024];
static uint8_t frame_B[1024];


void SPI_Init(void) {                                   
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    SPI1->CR1 &= ~SPI_CR1_SPE;
    // Master, Clock div 4 (16/4=4MHz), Software Slave Management
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSM | SPI_CR1_SSI;
    // 8-bit data size, RX threshold
    SPI1->CR2 = (7 << SPI_CR2_DS_Pos) | SPI_CR2_FRXTH;
    SPI1->CR1 |= SPI_CR1_SPE;
}

void SPI_SendByteBlocking(uint8_t data) {
    while (!(SPI1->SR & SPI_SR_TXE));             
    *(__IO uint8_t *)&SPI1->DR = data;            
    while ((SPI1->SR & SPI_SR_BSY));              
}

void OLED_SendCommand(uint8_t cmd) {
    // Signal to the display that
    // command is coming by
    // pulling down DC
    GPIOA->BSRR = GPIO_BSRR_BR9;
    // Pull CS down -> selects
    // display as the receiver of data 
    GPIOB->BSRR = GPIO_BSRR_BR6;    
    SPI_SendByteBlocking(cmd);
    // Deselect display        
    GPIOB->BSRR = GPIO_BSRR_BS6; // CS High  
}

void OLED_Init(void) {
    // Hardware Reset Sequence
    GPIOC->BSRR = GPIO_BSRR_BR7;
    delay_ms(10);
    GPIOC->BSRR = GPIO_BSRR_BS7;
    delay_ms(10);

    // Standard Initialization
    OLED_SendCommand(0xAE); // Display OFF
    OLED_SendCommand(0x20); // Set Memory Addressing Mode
    OLED_SendCommand(0x00); // Horizontal Addressing
    OLED_SendCommand(0x21); // Set Column Address
    OLED_SendCommand(0x00); 
    OLED_SendCommand(0x7F); 
    OLED_SendCommand(0x22); // Set Page Address
    OLED_SendCommand(0x00); 
    OLED_SendCommand(0x07); 
    OLED_SendCommand(0xC8); // Flip Vertically
    OLED_SendCommand(0xA1); // Flip Horizontally
    OLED_SendCommand(0x81); // Contrast
    OLED_SendCommand(0xFF); 
    OLED_SendCommand(0xA7); // Normal Display
    OLED_SendCommand(0x8D); // Charge pump
    OLED_SendCommand(0x14); 
    OLED_SendCommand(0xAF); // Display ON
}

int main(void) {
    // Core Initialization
    SysTick_Init(SYS_CLOCK_HZ);
    
    // Enable GPIO clocks
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);

    // Pin Configuration
    GPIO_Init(GPIOA, 5, 2, 0, 2, 0); // PA5 SCK (AF5)
    GPIO_Init(GPIOA, 7, 2, 0, 2, 0); // PA7 MOSI (AF5)
    GPIOA->AFR[0] &= ~((0xF << 20) | (0xF << 28)); 
    GPIOA->AFR[0] |=  ((5 << 20) | (5 << 28));     
    
    GPIO_Init(GPIOB, 6, 1, 0, 2, 0); // PB6 CS Output
    GPIO_Init(GPIOA, 9, 1, 0, 2, 0); // PA9 DC Output
    GPIO_Init(GPIOC, 7, 1, 0, 2, 0); // PC7 RES Output
    
    GPIOB->BSRR = GPIO_BSRR_BS6; 
    GPIOA->BSRR = GPIO_BSRR_BS9;
    GPIOC->BSRR = GPIO_BSRR_BS7;

    // Peripheral Initialization
    SPI_Init();
    OLED_Init();
    
    // Hand over control to the DMA modules
    SPI_DMA_Init();
    USART2_DMA_Init(SYS_CLOCK_HZ);

    // Double Buffering Setup
    uint8_t* active_rx_buffer = frame_A; // Where USART puts new frames
    uint8_t* active_tx_buffer = frame_B; // Where SPI sends frames from
    
    bool new_frame_ready = false;
    
    OLED_SendFrameAsync(test_frame);

    while (1) {
        
        // Drain the DMA ring buffer as fast as possible
        // If we find a valid frame, lock it into the active_rx_buffer
        if (Stream_ExtractFrame(active_rx_buffer)) {
            new_frame_ready = true;
        }

        // If the SPI hardware is done sending the last frame, and we have a new one ready...
        if (new_frame_ready && !OLED_IsTransferBusy()) {
            
            // Blast the newly extracted frame to the screen
            OLED_SendFrameAsync(active_rx_buffer);
            new_frame_ready = false;

            // POINTER SWAP: Swap the buffers so the background USART parser doesn't 
            // overwrite the frame currently being sent by the SPI hardware!
            uint8_t* temp = active_rx_buffer;
            active_rx_buffer = active_tx_buffer;
            active_tx_buffer = temp;
        }
    }
}