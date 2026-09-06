#include "sensor_hw.h"
#include "stm32g491xx.h"

void I2C1_Init(void)
{
    // Enable GPIOB and I2C1 clocks
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_I2C1EN;

    // Set PB8 (SCL) and PB9 (SDA) to Alternate Function 4 (I2C1), Open-Drain
    GPIOB->MODER &= ~(GPIO_MODER_MODE8 | GPIO_MODER_MODE9);
    GPIOB->MODER |= (GPIO_MODER_MODE8_1 | GPIO_MODER_MODE9_1);
    GPIOB->OTYPER |= (GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9);
    GPIOB->AFR[1] |= (4 << GPIO_AFRH_AFSEL8_Pos) | (4 << GPIO_AFRH_AFSEL9_Pos);

    // Configure I2C timing for 100kHz at 16MHz clock
    I2C1->CR1 &= ~I2C_CR1_PE;
    I2C1->TIMINGR = 0x00303D5B;
    I2C1->CR1 |= I2C_CR1_PE;
}

void I2C1_ReadRegisters(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    // Trigger START condition, so the sensor
    // knows we want to tell him where to
    // read data from
    I2C1->CR2 = (0x76 << 1) | (1 << 16) | I2C_CR2_START;
    
    while (!(I2C1->ISR & I2C_ISR_TXIS))
    ;
    // Send the register address we want to read from
    I2C1->TXDR = reg;
    while (!(I2C1->ISR & I2C_ISR_TC))
        ; // Wait for Transfer Complete

    // Specify we want
    // 'length' bytes back from our
    // specified starting reg
    // (AUTOEND will result in STOP flag being set after transfer)
    I2C1->CR2 = (0x76 << 1) | (length << 16) | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_AUTOEND;

    for (uint8_t i = 0; i < length; i++)
    {
        while (!(I2C1->ISR & I2C_ISR_RXNE))
            ; // Wait for Receive Buffer Not Empty
        buffer[i] = I2C1->RXDR;
    }

    // Wait for automatic STOP set, then clear the flag
    while (!(I2C1->ISR & I2C_ISR_STOPF))
        ;
    I2C1->ICR |= I2C_ICR_STOPCF;
}

void I2C1_WriteRegister(uint8_t reg, uint8_t value)
{
    // 2 bytes to send: Register Address, then Value
    I2C1->CR2 = (0x76 << 1) | (2 << 16) | I2C_CR2_START | I2C_CR2_AUTOEND;

    while (!(I2C1->ISR & I2C_ISR_TXIS))
        ;
    I2C1->TXDR = reg;

    while (!(I2C1->ISR & I2C_ISR_TXIS))
        ;
    I2C1->TXDR = value;

    while (!(I2C1->ISR & I2C_ISR_STOPF))
        ;
    I2C1->ICR |= I2C_ICR_STOPCF;
}
