#include "button.h"

void Button_Init(void) {
    // Enable GPIOC clock
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
    
    // Configure PC13 as Input (Clear MODER bits 26 and 27)
    GPIOC->MODER &= ~GPIO_MODER_MODE13;
}

bool Button_IsPressed(void) {
    // Read the Input Data Register for pin 13
    return (GPIOC->IDR & GPIO_IDR_ID13) != 0;
}