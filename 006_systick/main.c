#include "device_drivers/gpio.h"
#include "device_headers/stm32g491xx.h"
#include <stdint.h>

volatile uint32_t sys_ticks = 0;

void SysTick_Init(void){
    // Set Reload value
    // with this value the frequency
    // is ~ 1ms
    // (Why? Because we set CTRL bit 2 to '1' = use processor clock
    // -> 16MHz = 16'000'000 cycles / 1000ms = 16'000 / 1 ms)
    SysTick->LOAD= 16000 - 1;

    // Clear current value register to force a reload
    SysTick->VAL = 0;

    // set priority to be lowest
    // (this is not needed here, but important
    // for multi taks programs...duh)
    // NOTE - this is CMSIS function for ease of use
    NVIC_SetPriority(SysTick_IRQn,15);

    // Enable SysTick
    // Bit 0: ENABLE (1 = timer runs)
    // Bit 1: TICKINT (1 = trigger exception 15 on count down to 0)
    // Bit 2: CLKSOURCE (1 = use processor clock, 0 = external clock/8)
    SysTick->CTRL = 0x7UL;  
}

// Must match startup.c handler name for the SysTick
void SysTick_Handler(void) {
    sys_ticks++;
}

void delay_ms(uint32_t ms) {
    uint32_t start = sys_ticks;
    
    // Unsigned integer subtraction naturally handles the 49-day overflow 
    // of the 32-bit tick counter. As long as 'ms' is less than the max 
    // 32-bit value, this loop is mathematically safe.
    while ((sys_ticks - start) < ms) {
        // CMSIS macro, expands to architecture agnostic "Wait-For-Interrupt"
        // not needed, but reduces polling (duh)
        __WFI();
    }
}

int main(void) {
    // Enable clock for GPIOA
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    
    GPIO_Init(GPIOA, 5, 1, 0, 0, 0);
    SysTick_Init();
    
    while(1) {
        GPIO_WritePin(GPIOA, 5, 1);
        delay_ms(200);
        GPIO_WritePin(GPIOA, 5, 0);
        delay_ms(200);
    }
    return 0;
}