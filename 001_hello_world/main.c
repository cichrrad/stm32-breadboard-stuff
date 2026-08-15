// mem layout from RM0440 

// explicit uint32_t would be better
#define RCC_AHB2ENR (*(volatile unsigned int *)0x4002104C) // Peripheral clock enable reg.
#define GPIOA_MODER (*(volatile unsigned int *)0x48000000) // Port A pins mode reg.
#define GPIOA_ODR (*(volatile unsigned int *)0x48000014)   // Port A pins output data reg.

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
        // Toggle Port A pin 5 (LED) output
        GPIOA_ODR ^= (1 << 5);
        for (volatile int i = 0; i < 5000000; i++)
        {
            // Zzzzz
        }
    }
    return 0;
}