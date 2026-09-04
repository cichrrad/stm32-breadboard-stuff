#include "utils_rng.h"
#include "stm32g491xx.h"

void RNG_Init(void)
{
    // Enable the HSI48 internal oscillator
    RCC->CRRCR |= RCC_CRRCR_HSI48ON;

    // Wait until HSI48 is stable and ready
    while ((RCC->CRRCR & RCC_CRRCR_HSI48RDY) == 0)
        ;

    // Select HSI48 as the 48MHz clock source (Clear bits 27:26 to 00)
    RCC->CCIPR &= ~RCC_CCIPR_CLK48SEL;

    // Enable RNG peripheral clock on the AHB2 bus
    RCC->AHB2ENR |= RCC_AHB2ENR_RNGEN;

    // Enable the RNG peripheral
    RNG->CR |= RNG_CR_RNGEN;
}

uint32_t utils_rand(void)
{
    while ((RNG->SR & RNG_SR_DRDY) == 0)
        ;

    // Read and return the 32-bit hardware-generated random number
    return RNG->DR;
}