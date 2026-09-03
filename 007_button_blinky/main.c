#include "gpio.h"

void EXTI0_IRQHandler(void) {
    // Check if the interrupt came from EXTI Line 0
    if ((EXTI->PR1 & EXTI_PR1_PIF0) != 0) {
        
        // Clear the pending irq bit by writing 1 to it
        // why write 1 ? Read up on "rc_w1" mechanism
        EXTI->PR1 = EXTI_PR1_PIF0; 
        
        // We are inside irq, so toggle will
        // ACTUALLY be "atomic" here
        GPIOA->ODR ^= GPIO_ODR_OD5; 
    }
}

void Button_EXTI_Init(void) {
    
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

int main(void) {

    // SUPER IMPORTANT, UNLESS YOU WANT TO
    // LOCK YOURSELF OUT OF YOUR CHIP
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP; // enable debug in sleep
    // if not set, while loop below with WFI
    // would not allow us to flash new stuff
    // unless we reset flash
    
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    GPIO_Init(GPIOA, 5, 1, 0, 0, 0);   
    Button_EXTI_Init();
    
    while(1) {
        __WFI(); 
    }
    return 0;
}