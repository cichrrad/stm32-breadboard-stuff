#include "device_drivers/gpio.h"
#include "device_drivers/tick_engine/systick_timer.h"

static uint8_t frame[1024];


// BUTTON ============================================================

// Handler
void EXTI0_IRQHandler(void) {
    // Check if the interrupt came from EXTI Line 0
    if ((EXTI->PR1 & EXTI_PR1_PIF0) != 0) {
        
        // FEED THE DOG!
        IWDG->KR = 0xAAAA;

        // Clear the interrupt pending bit by writing a 1
        EXTI->PR1 |= EXTI_PR1_PIF0;
    }
}

void Button_Init(void) {
    
    // Enable GPIOB clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;        
    // Init PB0 with mode to input (00) and Pull-Up (01)
    GPIO_Init(GPIOB,0,0,0,0,1);

    // SYSCFG Configuration
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;        // Enable SYSCFG clock
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI0;  // Clear existing routing for EXTI0
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PB;// Route EXTI0 to Port B (PB0)

    // EXTI Configuration
    EXTI->IMR1 |= EXTI_IMR1_IM0;                 // Unmask EXTI Line 0 (enable it)
    EXTI->RTSR1 &= ~EXTI_RTSR1_RT0;              // Disable Rising Edge trigger
    EXTI->FTSR1 |= EXTI_FTSR1_FT0;               // Enable Falling Edge trigger
    // why? Because the button will (upon press)
    // bridge to GND, overpowering pull-up
    // and causing voltage to drop 
    // (falling edge -> caugt by our irq)

    // NVIC Configuration
    NVIC_SetPriority(EXTI0_IRQn, 5);             // Set priority
    NVIC_EnableIRQ(EXTI0_IRQn);                  // Enable EXTI0 interrupt in the NVIC
}

// DISPLAY ===========================================================

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
    *(__IO uint8_t *)&SPI1->DR = data;            
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
    // Unplug MOSI
    GPIOC->BSRR = GPIO_BSRR_BR7;
    uint32_t start = GetTick();
    while (!SysTick_IsElapsed(start, 10)) { __NOP(); }
    // Plug MOSI
    GPIOC->BSRR = GPIO_BSRR_BS7;
    start = GetTick();
    while (!SysTick_IsElapsed(start, 10)) { __NOP(); }

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
    OLED_SendCommand(0xCF); // Contrast value (0x00 to 0xFF)
    
    OLED_SendCommand(0xA6); // Normal Display (A7 would invert colors)

    OLED_SendCommand(0x8D); // Charge pump setting
    OLED_SendCommand(0x14); // Enable charge pump

    OLED_SendCommand(0xAF); // Display ON
}

void OLED_DisplayFrame(uint8_t *frame){
        for (int i = 0; i < 1024; i++) {
        frame[i] = frame[i];
        OLED_SendData(frame[i]);
    }
}

// WATCHDOG =========================================================

void IWDG_Init(void) {
    // Start the dog
    // Note: From what I understand, this SHOULD be first thing you do 
    // before further init, because it enables LSI domain sync
    // with the CPU clock
    IWDG->KR = 0xCCCC; 

    // Unlock IWDG registers (Write access to PR and RLR)
    IWDG->KR = 0x5555;

    // Set the Prescaler to 256 (32000 Hz / 256 = 125 Hz -> 1 tick = 8 ms)
    IWDG->PR = 7;

    // Set the Reload Register for ~2 seconds 
    // (2000 ms / 8 ms per tick = 250)
    IWDG->RLR = 250;

    // Wait until the registers are updated
    // This would freeze if "IWDG->KR = 0xCCCC;" was after
    while (IWDG->SR != 0);

    // Feed the dog for the first time
    IWDG->KR = 0xAAAA; 
}

// ==================================================================

void init_all(){
    // Button Init
    Button_Init();

    // Display & SPI Init
    RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // Init GPIO Pins
    // SCK (PA5)  -- Alternate Mode (2), Push-pull, fast, no pull
    GPIO_Init(GPIOA, 5, 2, 0, 2, 0); 
    // MOSI (PA7) -- Alternate Mode (2), Push-pull, fast, no pull
    GPIO_Init(GPIOA, 7, 2, 0, 2, 0); 
    
    // CS (PB6)   -- General Purpose Output (1)
    GPIO_Init(GPIOB, 6, 1, 0, 2, 0); 
    // DC (PA9)   -- General Purpose Output (1)
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
}

int main(void) {

    SysTick_Init(16000000); // internal clock

    // SUPER IMPORTANT, UNLESS YOU WANT TO
    // LOCK YOURSELF OUT OF YOUR CHIP
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP; // enable debug in sleep
    // if not set, while loop below with WFI
    // would not allow us to flash new stuff
    // unless we reset flash
    
    init_all();
    IWDG_Init(); // Watchdog init

    // clear the screen
    for(uint32_t i = 1; i < 1024; i++){
        frame[i] = 0x00;
    }
    OLED_DisplayFrame(frame);
    uint32_t last_idx = 0;
    uint32_t start_time = GetTick();
    while(1) {
        // every 17 ms, flip next byte
        if(SysTick_IsElapsed(start_time, 17)){
            frame[(last_idx)%1024] ^= 0xFF;
            last_idx++; 
            OLED_DisplayFrame(frame);
            
            // UNCOMMENT TO NOT HAVE TO FEED THE DOG
            // USING BUTTON
            // IWDG->KR = 0xAAAA;
            
            start_time = GetTick();
        } 
    }
    return 0;
}