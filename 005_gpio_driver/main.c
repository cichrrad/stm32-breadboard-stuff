#include "device_drivers/gpio.h"

void mySleep(uint32_t dur){
    for(volatile uint32_t i = 0; i < dur; i++){
        // Zzzzz
    }
}

int main(void)
{
	// Enable clock for GPIOA
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
    // Init and blink via the "driver"
    GPIO_Init(GPIOA, 5, 1, 0, 0, 0);
    while(1){
        GPIO_WritePin(GPIOA,5,1);
        mySleep(1000000);
        GPIO_WritePin(GPIOA,5,0);
        mySleep(1000000);
    }

}
