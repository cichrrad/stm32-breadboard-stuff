// THIS IS BASICALLY 001, BUT WITH SOME IMPROVEMENTS
#include <stdint.h>

// mem layout from RM0440
#define RCC_BASE 0x40021000
#define GPIOA_BASE 0x48000000

#define RCC_AHB2ENR (*(volatile uint32_t *)(RCC_BASE + 0x4C))   // Peripheral clock enable reg.
#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00)) // Port A pins mode reg.
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14))   // Port A pins output data reg.

// IMPORTANT
// This register should be used over direct manipulation
// to set/reset bits, because it is atomic, while doing
// something like "GPIOA_ODR ^= (1 << 5)" is NOT.
// (it will be broken into read-modify-write assembly ops)
#define GPIOA_BSRR (*(volatile uint32_t *)(GPIOA_BASE + 0x18)) // Atomic Set/Reset register
// Note: first half is for set, second half for reset

int main(void)
{

    // Bit 0 enables clock for GPIOA
    // clock must be enabled in order
    // for reads/writes to the registers
    // to have effect
    RCC_AHB2ENR |= (1 << 0);

    // Change Port A pin 5 (LED) mode to be:
    // [01]: General purpose output mode
    // -> set [01] on pins [11,10]
    GPIOA_MODER &= ~(3 << 10);
    GPIOA_MODER |= (1 << 10);

    while (1)
    {
        
        if (GPIOA_ODR & (1 << 5))
        {
            // LED is ON -> reset it
            GPIOA_BSRR = (1 << (5 + 16));
        }
        else
        {

            // LED if OFF -> set it
            GPIOA_BSRR = (1 << 5);
        }

        for (volatile int i = 0; i < 5000000; i++)
        {
            // Zzzzz
        }
    }
    return 0;
}