#include "spi_dma.h"

// STM32G4 Reference Manual (RM0440): SPI1_TX is DMAMUX Request ID 11
#define DMAMUX_REQ_SPI1_TX 11

// Volatile flag to safely share state between the ISR and the main loop
static volatile bool transfer_busy = false;

void SPI_DMA_Init(void) {
    // Enable clocks for DMA1 and DMAMUX1
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;
    
    // Read back to ensure the clock is stabilized before accessing registers
    (void)RCC->AHB1ENR; 

    // Configure DMAMUX1 Channel 0 to route SPI1_TX to DMA1 Channel 1
    DMAMUX1_Channel0->CCR = DMAMUX_REQ_SPI1_TX;

    // Configure DMA1 Channel 1
    // Disable the channel
    DMA1_Channel1->CCR = 0;
    
    // Set peripheral address to the SPI1 Data Register
    DMA1_Channel1->CPAR = (uint32_t)&(SPI1->DR);
    
    // Configuration:
    // DIR = 1: Memory to Peripheral
    // MINC = 1: Increment Memory address pointer
    // PINC = 0: Do not increment Peripheral address pointer
    // MSIZE = 00: 8-bit memory data size
    // PSIZE = 00: 8-bit peripheral data size
    // TCIE = 1: Enable Transfer Complete Interrupt
    DMA1_Channel1->CCR = DMA_CCR_DIR | 
                         DMA_CCR_MINC | 
                         DMA_CCR_TCIE;

    // Enable the TX DMA request generation in the SPI1 peripheral
    SPI1->CR2 |= SPI_CR2_TXDMAEN;

    // Configure the NVIC for the DMA interrupt
    NVIC_SetPriority(DMA1_Channel1_IRQn, 1); // Set a reasonable priority
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void OLED_SendFrameAsync(const uint8_t* frame_buffer) {
    // Safety check: Prevent stomping on an active transfer
    if (transfer_busy) {
        return;
    }
    
    transfer_busy = true;

    // Ensure DMA is disabled before updating target addresses and counters
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    // Set Memory Address and Transfer Length (1024 bytes)
    DMA1_Channel1->CMAR = (uint32_t)frame_buffer;
    DMA1_Channel1->CNDTR = 1024;

    // Set Data/Command (DC) HIGH for Data (PA9)
    GPIOA->BSRR = GPIO_BSRR_BS9;
    
    // Set Chip Select (CS) LOW to select the display (PB6)
    GPIOB->BSRR = GPIO_BSRR_BR6;

    // send the DMA
    DMA1_Channel1->CCR |= DMA_CCR_EN;
}

bool OLED_IsTransferBusy(void) {
    return transfer_busy;
}

// DMA1 Channel 1 Interrupt Service Routine
void DMA1_Channel1_IRQHandler(void) {
    // Check if the Transfer Complete (TC) flag for Channel 1 is set
    if (DMA1->ISR & DMA_ISR_TCIF1) {
        
        // Clear the interrupt flag in the IFCR (Interrupt Flag Clear Register)
        DMA1->IFCR = DMA_IFCR_CTCIF1;

        // Disable the DMA channel
        DMA1_Channel1->CCR &= ~DMA_CCR_EN;

        // WAIT FOR SPI TO FINISH
        // DMA finishes when the last byte enters the SPI FIFO.
        // -> must wait for the FIFO to drain (TXE) and the 
        // shift register to finish (BSY).
        while (!(SPI1->SR & SPI_SR_TXE)) {
            // Wait for Transmit buffer to empty
        }
        while (SPI1->SR & SPI_SR_BSY) {
            // Wait for SPI hardware to physically finish shifting the final bit
        }

        // Pull CS HIGH to deselect the display (PB6)
        GPIOB->BSRR = GPIO_BSRR_BS6;

        // Release the lock so the main loop can queue the next frame
        transfer_busy = false;
    }
}