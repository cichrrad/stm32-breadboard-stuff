#include "oled_hw.h"
#include "device_headers/stm32g491xx.h"
#include "device_drivers/gpio.h"
#include "device_drivers/tick_engine/systick_timer.h"

volatile bool dma_busy = false;

void OLED_Init(void)
{
    // HW Reset sequence
    // Toggle display OFF via RES wire
    GPIOC->BSRR = GPIO_BSRR_BR7;
    delay_ms(10);
    // Toggle ON
    GPIOC->BSRR = GPIO_BSRR_BS7;
    delay_ms(10);

    // Standard SSD1306/SH1106 Initialization Sequence
    // Say "Thank you, Gemini"
    OLED_SendCommand(0xAE); // Display OFF

    OLED_SendCommand(0x20); // Set Memory Addressing Mode
    OLED_SendCommand(0x00); // 00=Horizontal Addressing (Wraps around edges automatically)

    OLED_SendCommand(0x21); // Set Column Address (X axis bounds)
    OLED_SendCommand(0x00); // Start: 0
    OLED_SendCommand(0x7F); // End: 127

    OLED_SendCommand(0x22); // Set Page Address (Y axis bounds)
    OLED_SendCommand(0x00); // Start: 0
    OLED_SendCommand(0x07); // End: 7 (8 pages total * 8 bits = 64 pixels tall)

    OLED_SendCommand(0xC8); // COM Output Scan Direction (Flips screen vertically)
    OLED_SendCommand(0xA1); // Segment Re-map (Flips screen horizontally)
    
    OLED_SendCommand(0x81); // Set Contrast Control
    OLED_SendCommand(0xFF); // Contrast value (0x00 to 0xFF)
    
    OLED_SendCommand(0xA6); // Normal Display (A7 would invert colors)
    
    OLED_SendCommand(0x8D); // Charge pump setting
    OLED_SendCommand(0x14); // Enable charge pump
    
    OLED_SendCommand(0xAF); // Display ON
}

void SPI_Init(void)
{
    // Disable SPI before config changes
    SPI1->CR1 &= ~SPI_CR1_SPE;

    // CR1 Configuration
    // MSTR = 1 (Master Mode)
    // BR = 001 (Clock div 4 -> 16Mhz/4= 4MHz)
    // SSM = 1, SSI = 1 (Software Slave Management for CS)
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSM | SPI_CR1_SSI;

    // CR2 Configuration
    // DS = 0111 (8-bit data size)
    // FRXTH = 1 (FRXTH register will signal if >= 8-bits came in FIFO)
    SPI1->CR2 = (7 << SPI_CR2_DS_Pos) | SPI_CR2_FRXTH;

    // Re-enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

// Crucial: STM32G4 Data Register requires strict 8-bit pointer casting
// If you do `SPI1->DR = data`, it writes 16 bits (packing a 0x00), which ruins the stream!
void SPI_SendByte(uint8_t data)
{

    // Wait until Transmit buffer is empty
    while (!(SPI1->SR & SPI_SR_TXE))
        ;
    // Make 8-bit write to Data Register
    *(volatile uint8_t *)&SPI1->DR = data;
    // Wait until SPI is not busy
    while ((SPI1->SR & SPI_SR_BSY))
        ;
}

void OLED_SendCommand(uint8_t cmd)
{
    // Signal to the display that
    // command is coming by
    // pulling down Port A pin 9 (DC)
    GPIOA->BSRR = GPIO_BSRR_BR9;
    // Pull Port B pin 6 (CS) down -> selects
    // display as the receiver of data
    GPIOB->BSRR = GPIO_BSRR_BR6;
    SPI_SendByte(cmd);
    // Deselect display
    GPIOB->BSRR = GPIO_BSRR_BS6;
}

// Analogous to OLED_SendCommand
void OLED_SendData(uint8_t data)
{
    GPIOA->BSRR = GPIO_BSRR_BS9;
    GPIOB->BSRR = GPIO_BSRR_BR6;
    SPI_SendByte(data);
    GPIOB->BSRR = GPIO_BSRR_BS6;
}

void OLED_DisplayFrame(const uint8_t *frame)
{
    for (int i = 0; i < 1024; i++)
    {
        OLED_SendData(frame[i]);
    }
}

void DMA_Init(void) {
    // Enable DMA1 and DMAMUX clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;

    // Route SPI1_TX (Request 11) to DMA1 Channel 1 via DMAMUX
    DMAMUX1_Channel0->CCR = 11; 

    // Configure DMA1 Channel 1
    // Memory to Peripheral (DIR = 1), Memory Increment (MINC = 1)
    // 8-bit sizes default. Enable Transfer Complete Interrupt (TCIE = 1)
    DMA1_Channel1->CPAR = (uint32_t)&SPI1->DR;
    DMA1_Channel1->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE;

    // Enable DMA1 Channel 1 Interrupt in NVIC
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    
    // Tell SPI1 to issue DMA requests for TX
    SPI1->CR2 |= SPI_CR2_TXDMAEN;
}


void OLED_HW_Init(void) {
    
    // Enable Clocks (GPIOA, GPIOB, GPIOC, SPI1)
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    
    // Init GPIO Pins
    // SCK (PA5)  -- Alternate Mode (2), Push-pull, fast, no pull
    GPIO_Init(GPIOA, 5, 2, 0, 2, 0);
    // MOSI (PA7) -- Alternate Mode (2), Push-pull, fast, no pull
    GPIO_Init(GPIOA, 7, 2, 0, 2, 0);

    // CS (PB6), DC (PA9), RES (PC7) -- General Purpose Output (1)
    GPIO_Init(GPIOB, 6, 1, 0, 2, 0);
    GPIO_Init(GPIOA, 9, 1, 0, 2, 0);
    GPIO_Init(GPIOC, 7, 1, 0, 2, 0);

    // Default states for outputs: CS High, DC High, RES High
    GPIOB->BSRR = GPIO_BSRR_BS6;
    GPIOA->BSRR = GPIO_BSRR_BS9;
    GPIOC->BSRR = GPIO_BSRR_BS7;

    // Configure Alternate Functions for SPI (PA5 and PA7 to AF5)
    GPIOA->AFR[0] &= ~((0xF << 20) | (0xF << 28)); 
    GPIOA->AFR[0] |= ((5 << 20) | (5 << 28));      

    // Initialize Peripherals
    SPI_Init();
    DMA_Init();
    OLED_Init();
}
void OLED_Update_DMA(const uint8_t *framebuffer) {
    if (dma_busy) return;
    dma_busy = true;

    // Prepare for Data Transmission
    GPIOA->BSRR = GPIO_BSRR_BS9; // DC High (Data)
    GPIOB->BSRR = GPIO_BSRR_BR6; // CS Low  (Select Display)
    
    // Fire DMA
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;             
    DMA1_Channel1->CMAR = (uint32_t)framebuffer;   
    DMA1_Channel1->CNDTR = 1024;                   
    DMA1_Channel1->CCR |= DMA_CCR_EN;              
}

void DMA1_Channel1_IRQHandler(void) {
    if (DMA1->ISR & DMA_ISR_TCIF1) {
        // Wait for SPI to finish physically shifting out the last bit!
        while ((SPI1->SR & SPI_SR_BSY));
        
        // Deselect the display
        GPIOB->BSRR = GPIO_BSRR_BS6; // CS High 
        
        // Clear ALL interrupt flags for Channel 1 to prevent IRQ loops
        DMA1->IFCR = DMA_IFCR_CGIF1; 
        
        // Unblock the render pipeline
        dma_busy = false;             
    }
}