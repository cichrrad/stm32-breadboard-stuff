#include "ili9341_hw.h"
#include "stm32g491xx.h"
#include "FreeRTOS.h"
#include "task.h"

TaskHandle_t xRenderTaskHandle = NULL;

// A small DMA chunk buffer to stream solid colors without a full framebuffer
static uint8_t dma_color_chunk[ILI9341_CHUNK_SIZE * 2];

void SPI_SendByte(uint8_t data)
{
    while (!(SPI1->SR & SPI_SR_TXE))
        ;
    *(volatile uint8_t *)&SPI1->DR = data;
    while ((SPI1->SR & SPI_SR_BSY))
        ;
}

void ILI9341_SendCommand(uint8_t cmd)
{
    GPIOA->BSRR = GPIO_BSRR_BR9; // DC Low
    GPIOB->BSRR = GPIO_BSRR_BR6; // CS Low
    SPI_SendByte(cmd);
    GPIOB->BSRR = GPIO_BSRR_BS6; // CS High
}

void ILI9341_SendData(uint8_t data)
{
    GPIOA->BSRR = GPIO_BSRR_BS9; // DC High
    GPIOB->BSRR = GPIO_BSRR_BR6;
    SPI_SendByte(data);
    GPIOB->BSRR = GPIO_BSRR_BS6;
}

void ILI9341_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    ILI9341_SendCommand(0x2A); // Column Address Set
    ILI9341_SendData(x0 >> 8);
    ILI9341_SendData(x0 & 0xFF);
    ILI9341_SendData(x1 >> 8);
    ILI9341_SendData(x1 & 0xFF);

    ILI9341_SendCommand(0x2B); // Page Address Set
    ILI9341_SendData(y0 >> 8);
    ILI9341_SendData(y0 & 0xFF);
    ILI9341_SendData(y1 >> 8);
    ILI9341_SendData(y1 & 0xFF);

    ILI9341_SendCommand(0x2C); // Memory Write
}

void ILI9341_Init(void)
{
    // Hardware Reset
    GPIOC->BSRR = GPIO_BSRR_BR7;
    vTaskDelay(pdMS_TO_TICKS(50));
    GPIOC->BSRR = GPIO_BSRR_BS7;
    vTaskDelay(pdMS_TO_TICKS(150));

    ILI9341_SendCommand(0x01); // Software reset
    vTaskDelay(pdMS_TO_TICKS(10));

    // Power Control
    ILI9341_SendCommand(0xC0);
    ILI9341_SendData(0x23);
    ILI9341_SendCommand(0xC1);
    ILI9341_SendData(0x10);
    // VCOM Control
    ILI9341_SendCommand(0xC5);
    ILI9341_SendData(0x3E);
    ILI9341_SendData(0x28);
    // Memory Access Control (Landscape)
    ILI9341_SendCommand(0x36);
    ILI9341_SendData(0x28);
    // Pixel Format (16-bit / RGB565)
    ILI9341_SendCommand(0x3A);
    ILI9341_SendData(0x55);

    ILI9341_SendCommand(0x11); // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(120));
    ILI9341_SendCommand(0x29); // Display ON

    // Turn on backlight
    GPIOB->BSRR = GPIO_BSRR_BS8;
}

void ILI9341_HW_Init(void)
{
    // Clocks
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;

    // SPI Pins (PA5 SCK, PA7 MOSI -> AF5)
    GPIOA->MODER &= ~((3 << 10) | (3 << 14));
    GPIOA->MODER |= ((2 << 10) | (2 << 14));
    GPIOA->OSPEEDR |= ((3 << 10) | (3 << 14));
    GPIOA->AFR[0] &= ~((0xF << 20) | (0xF << 28));
    GPIOA->AFR[0] |= ((5 << 20) | (5 << 28));

    // Control Pins: CS(PB6), DC(PA9), RST(PC7), BL(PB8) as Outputs
    GPIOB->MODER &= ~((3 << 12) | (3 << 16));
    GPIOB->MODER |= ((1 << 12) | (1 << 16));
    GPIOA->MODER &= ~(3 << 18);
    GPIOA->MODER |= (1 << 18);
    GPIOC->MODER &= ~(3 << 14);
    GPIOC->MODER |= (1 << 14);

    // Initial states High
    GPIOB->BSRR = GPIO_BSRR_BS6 | GPIO_BSRR_BR8; // BL off initially
    GPIOA->BSRR = GPIO_BSRR_BS9;
    GPIOC->BSRR = GPIO_BSRR_BS7;

    // SPI1 Init (Master, 4MHz, 8-bit, SSM/SSI)
    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_0 | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR2 = (7 << SPI_CR2_DS_Pos) | SPI_CR2_FRXTH | SPI_CR2_TXDMAEN;
    SPI1->CR1 |= SPI_CR1_SPE;

    // DMA Init
    DMAMUX1_Channel0->CCR = 11;
    DMA1_Channel1->CPAR = (uint32_t)&SPI1->DR;
    DMA1_Channel1->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE;
    NVIC_SetPriority(DMA1_Channel1_IRQn, 5);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    ILI9341_Init();
}

void ILI9341_FillDMA(uint16_t color, uint32_t pixel_count)
{
    xRenderTaskHandle = xTaskGetCurrentTaskHandle();

    // Prep the chunk buffer
    for (int i = 0; i < ILI9341_CHUNK_SIZE * 2; i += 2)
    {
        dma_color_chunk[i] = color >> 8;
        dma_color_chunk[i + 1] = color & 0xFF;
    }

    GPIOA->BSRR = GPIO_BSRR_BS9; // DC Data
    GPIOB->BSRR = GPIO_BSRR_BR6; // CS Select

    uint32_t remaining = pixel_count;

    while (remaining > 0)
    {
        uint32_t to_send = (remaining > ILI9341_CHUNK_SIZE) ? ILI9341_CHUNK_SIZE : remaining;

        DMA1_Channel1->CCR &= ~DMA_CCR_EN;
        DMA1_Channel1->CMAR = (uint32_t)dma_color_chunk;
        DMA1_Channel1->CNDTR = to_send * 2; // bytes
        DMA1_Channel1->CCR |= DMA_CCR_EN;

        // Wait for this chunk to finish via FreeRTOS
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        remaining -= to_send;
    }

    // Ensure SPI is done before raising CS
    while ((SPI1->SR & SPI_SR_BSY))
        ;
    GPIOB->BSRR = GPIO_BSRR_BS6;
}

void DMA1_Channel1_IRQHandler(void)
{
    if (DMA1->ISR & DMA_ISR_TCIF1)
    {
        DMA1->IFCR = DMA_IFCR_CGIF1;
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(xRenderTaskHandle, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}