#include "hw_spi.h"
#include "stm32g491xx.h"
#include "FreeRTOS.h"
#include "semphr.h"

SemaphoreHandle_t xSpiDmaSemaphore = NULL;

// Helpers for GPIO manipulation
#define CS_LOW() (GPIOA->BSRR = GPIO_BSRR_BR4)
#define CS_HIGH() (GPIOA->BSRR = GPIO_BSRR_BS4)
#define DC_CMD() (GPIOA->BSRR = GPIO_BSRR_BR8)
#define DC_DATA() (GPIOA->BSRR = GPIO_BSRR_BS8)

void HW_SPI_Init(void)
{
    // Enable clocks for GPIOA, SPI1, DMA1, and DMAMUX
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;

    // GPIO Configuration
    GPIOA->MODER &= ~((3U << 8) | (3U << 10) | (3U << 14) | (3U << 16) | (3U << 18) | (3U << 20));
    GPIOA->MODER |= ((2U << 10) | (2U << 14) | (1U << 8) | (1U << 16) | (1U << 18) | (1U << 20));
    GPIOA->AFR[0] &= ~((0xFU << 20) | (0xFU << 28));
    GPIOA->AFR[0] |= ((5U << 20) | (5U << 28));
    GPIOA->OSPEEDR |= ((3U << 10) | (3U << 14));
    CS_HIGH();
    HW_Display_Backlight(0);

    // SPI1 Configuration
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM | (2U << 3);
    SPI1->CR2 = (7U << 8) | SPI_CR2_FRXTH;

    // DMA Configuration
    // Route SPI1_TX (Request 11) to DMA1_Channel1 via DMAMUX
    DMAMUX1_Channel0->CCR = 11;

    // Configure DMA1 Channel 1: Memory to Peripheral, 8-bit data, Memory Increment, Transfer Complete Interrupt
    DMA1_Channel1->CCR = DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_TCIE;
    DMA1_Channel1->CPAR = (uint32_t)&SPI1->DR;

    // NVIC Configuration for DMA Interrupt
    // Priority MUST be >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (which is 5) to use FreeRTOS ISR APIs safely
    NVIC_SetPriority(DMA1_Channel1_IRQn, 5);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    // Create the synchronization semaphore
    xSpiDmaSemaphore = xSemaphoreCreateBinary();

    SPI1->CR1 |= SPI_CR1_SPE;
}

void HW_Display_Reset(void)
{
    GPIOA->BSRR = GPIO_BSRR_BR9; // RST LOW
    vTaskDelay(pdMS_TO_TICKS(50));
    GPIOA->BSRR = GPIO_BSRR_BS9; // RST HIGH
    vTaskDelay(pdMS_TO_TICKS(120));
}

void HW_Display_Backlight(uint8_t state)
{
    if (state)
        GPIOA->BSRR = GPIO_BSRR_BS10;
    else
        GPIOA->BSRR = GPIO_BSRR_BR10;
}

// Internal inline helper for raw TX
static inline void SPI_Transmit8(uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_TXE))
        ;
    // Must cast to volatile uint8_t* to force 8-bit FIFO packing on STM32G4
    *(volatile uint8_t *)&SPI1->DR = data;
    while (SPI1->SR & SPI_SR_BSY)
        ;
}

void HW_SPI_SendCmd(uint8_t cmd)
{
    DC_CMD();
    CS_LOW();
    SPI_Transmit8(cmd);
    CS_HIGH();
}

void HW_SPI_SendData8(uint8_t data)
{
    DC_DATA();
    CS_LOW();
    SPI_Transmit8(data);
    CS_HIGH();
}

void HW_SPI_SendData16(uint16_t data)
{
    DC_DATA();
    CS_LOW();
    SPI_Transmit8(data >> 8);
    SPI_Transmit8(data & 0xFF);
    CS_HIGH();
}

void HW_SPI_SendPixels(uint16_t color, uint32_t pixel_count)
{
    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;

    DC_DATA();
    CS_LOW();
    while (pixel_count--)
    {
        SPI_Transmit8(hi);
        SPI_Transmit8(lo);
    }
    CS_HIGH();
}

void HW_SPI_SendFrame_DMA(const uint8_t *data, uint32_t length)
{
    DC_DATA();
    CS_LOW();

    // Disable DMA channel to configure it
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;

    // Set memory address and transfer length
    DMA1_Channel1->CMAR = (uint32_t)data;
    DMA1_Channel1->CNDTR = length;

    // Enable DMA SPI TX request and start DMA
    SPI1->CR2 |= SPI_CR2_TXDMAEN;
    DMA1_Channel1->CCR |= DMA_CCR_EN;

    // Task blocks here until the DMA interrupt gives the semaphore
    xSemaphoreTake(xSpiDmaSemaphore, portMAX_DELAY);

    // Wait for the final byte to physically exit the SPI shift register
    while (SPI1->SR & SPI_SR_BSY)
        ;

    CS_HIGH();
    SPI1->CR2 &= ~SPI_CR2_TXDMAEN;
}

void DMA1_Channel1_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Check if Transfer Complete Interrupt flag is set
    if (DMA1->ISR & DMA_ISR_TCIF1)
    {
        // Clear the interrupt flag
        DMA1->IFCR = DMA_IFCR_CTCIF1;

        // Unblock the rendering task
        xSemaphoreGiveFromISR(xSpiDmaSemaphore, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}