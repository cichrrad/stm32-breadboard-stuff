#include "device_drivers/gpio.h"
#include "display_driver/ddriver.h"
#include "assets/pika_yawn.h"
#include "assets/gui_banner.h"
// Rough delay for the reset sequence
void delay_ms(volatile uint32_t ms) {
    // Note: Very rough, assumes ~16MHz default clock. 
    // -> TODO?: integrate 006 systick :)
    ms *= 2000; 
    while(ms--) { __NOP(); }
}

void SPI_Init(void) {                                   
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
void SPI_SendByte(uint8_t data) {
    
    // Wait until Transmit buffer is empty
    while (!(SPI1->SR & SPI_SR_TXE));             
    // Make 8-bit write to Data Register
    *(volatile uint8_t *)&SPI1->DR = data;            
    // Wait until SPI is not busy
    while ((SPI1->SR & SPI_SR_BSY));              
}

void OLED_SendCommand(uint8_t cmd) {
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
void OLED_SendData(uint8_t data) {
    GPIOA->BSRR = GPIO_BSRR_BS9;
    GPIOB->BSRR = GPIO_BSRR_BR6;
    SPI_SendByte(data);    
    GPIOB->BSRR = GPIO_BSRR_BS6;
}

void OLED_Init(void) {
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

void OLED_DisplayFrame(const uint8_t *frame){
        for (int i = 0; i < 1024; i++) {
        OLED_SendData(frame[i]);
    }
}

int main(void) {
    // Enable Clocks (GPIOA, GPIOB, GPIOC, SPI1)
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Init GPIO Pins
    // SCK (PA5)  -- Alternate Mode (2), Push-pull, fast, no pull
    GPIO_Init(GPIOA, 5, 2, 0, 2, 0); 
    // MOSI (PA7) -- Alternate Mode (2), Push-pull, fast, no pull
    GPIO_Init(GPIOA, 7, 2, 0, 2, 0); 
    
    // CS (PB6)   -- General Purpose Output (1)
    // This gets pulled down to "select"
    // the slave connected on this wire
    // (we only have one CS wire so its easy)
    GPIO_Init(GPIOB, 6, 1, 0, 2, 0); 
    // DC (PA9)   -- General Purpose Output (1)
    // This gets pulled down to signal
    // the display incoming bytes are meant
    // for its internal configuration rather
    // than data to put onto the matrix
    GPIO_Init(GPIOA, 9, 1, 0, 2, 0);
    // RES (PC7)  -- General Purpose Output (1)
    GPIO_Init(GPIOC, 7, 1, 0, 2, 0);

    // Default states for outputs: CS High, DC High, RES High
    GPIOB->BSRR = GPIO_BSRR_BS6; 
    GPIOA->BSRR = GPIO_BSRR_BS9;
    GPIOC->BSRR = GPIO_BSRR_BS7;

    // Configure Alternate Functions for SPI
    // PA5 (Bits 23:20) and PA7 (Bits 31:28) to AF5
    GPIOA->AFR[0] &= ~((0xF << 20) | (0xF << 28)); // Clear
    GPIOA->AFR[0] |=  ((5 << 20) | (5 << 28));     // Set AF5

    // Initialize peripherals and screen
    SPI_Init();
    OLED_Init();

    dd_draw_bitmap(28,16,PIKA_YAWN_WIDTH,PIKA_YAWN_HEIGHT,pika_yawn,true);
    dd_draw_bitmap(0,0,GUI_WIDTH,GUI_HEIGHT,gui,true);

    OLED_DisplayFrame(DD_FRAMEBUFFER);    

    while (1) {
        // Zzzz
    }
}