// Official stm32g491xx include headers with
// structs overlaying the memory to align
// with registers using CMSIS
// (Common Microcontroller Software Interface Standard)
#include "../Include/device_headers/stm32g491xx.h"

// NOTE - under the hood, CMSIS works
// very similar to structs in 002,
// but for all the things + with added
// boilerplate for compatibility
// with whole chip series etc...


// Same as 001, BUT now using CMSIS
int main(void)
{
    // Bit 0 enables clock for GPIOA
    // clock must be enabled in order
    // for reads/writes to the registers
    // to have effect
    RCC->AHB2ENR |= (1 << 0);

    // Change Port A pin 5 (LED) mode to be:
    // [01]: General purpose output mode
    // -> set [01] on pins [11,10]
    GPIOA->MODER &= ~(3 << 10);
    GPIOA->MODER |= (1 << 10);
    
    while (1)
    {
        if (GPIOA->ODR & (1 << 5))
        {
            // LED is ON -> reset it
            GPIOA->BSRR = (1 << (5 + 16));
        }
        else
        {

            // LED if OFF -> set it
            GPIOA->BSRR = (1 << 5);
        }

        for (volatile int i = 0; i < 5000000; i++)
        {
            // Zzzzz
        }
    }
    return 0;

}
