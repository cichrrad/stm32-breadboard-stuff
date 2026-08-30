#include "stm32g491xx.h"
#include "i2c_sensor.h"

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

void BME280_Start(void)
{
    // 1x Oversampling for Humidity
    I2C1_WriteRegister(0xF2, 0x01);
    // 1x Oversampling Temp/Press, Normal Mode (0x03)
    I2C1_WriteRegister(0xF4, 0x27);
}

BME280_CalibData bme_calib;

void BME280_ReadCalibration(void)
{
    uint8_t calib1[26];
    uint8_t calib2[7];

    // Read Temp, Pressure, and H1 (Registers 0x88 to 0xA1)
    I2C1_ReadRegisters(0x88, calib1, 26);
    // Read H2 to H6 (Registers 0xE1 to 0xE7)
    I2C1_ReadRegisters(0xE1, calib2, 7);

    // Parse Temperature
    bme_calib.dig_T1 = (calib1[1] << 8) | calib1[0];
    bme_calib.dig_T2 = (calib1[3] << 8) | calib1[2];
    bme_calib.dig_T3 = (calib1[5] << 8) | calib1[4];

    // Parse Pressure
    bme_calib.dig_P1 = (calib1[7] << 8) | calib1[6];
    bme_calib.dig_P2 = (calib1[9] << 8) | calib1[8];
    bme_calib.dig_P3 = (calib1[11] << 8) | calib1[10];
    bme_calib.dig_P4 = (calib1[13] << 8) | calib1[12];
    bme_calib.dig_P5 = (calib1[15] << 8) | calib1[14];
    bme_calib.dig_P6 = (calib1[17] << 8) | calib1[16];
    bme_calib.dig_P7 = (calib1[19] << 8) | calib1[18];
    bme_calib.dig_P8 = (calib1[21] << 8) | calib1[20];
    bme_calib.dig_P9 = (calib1[23] << 8) | calib1[22];

    // Parse Humidity
    bme_calib.dig_H1 = calib1[25];
    bme_calib.dig_H2 = (calib2[1] << 8) | calib2[0];
    bme_calib.dig_H3 = calib2[2];

    // 12-bit parsing for H4 and H5
    bme_calib.dig_H4 = (calib2[3] << 4) | (calib2[4] & 0x0F);
    bme_calib.dig_H5 = (calib2[5] << 4) | (calib2[4] >> 4);

    bme_calib.dig_H6 = (int8_t)calib2[6];
}

int32_t t_fine;

void BME280_ReadData(int32_t *temp, uint32_t *press, uint32_t *hum)
{
    uint8_t raw[8];
    I2C1_ReadRegisters(0xF7, raw, 8); // Read Pressure, Temp, Humidity

    int32_t adc_P = (raw[0] << 12) | (raw[1] << 4) | (raw[2] >> 4);
    int32_t adc_T = (raw[3] << 12) | (raw[4] << 4) | (raw[5] >> 4);
    int32_t adc_H = (raw[6] << 8) | raw[7];

    // Temperature Compensation
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)bme_calib.dig_T1 << 1))) * ((int32_t)bme_calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)bme_calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)bme_calib.dig_T1))) >> 12) * ((int32_t)bme_calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    *temp = (t_fine * 5 + 128) >> 8; // DegC * 100

    // Pressure Compensation (32-bit version)
    int32_t p_var1, p_var2;
    uint32_t p;

    p_var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
    p_var2 = (((p_var1 >> 2) * (p_var1 >> 2)) >> 11) * ((int32_t)bme_calib.dig_P6);
    p_var2 = p_var2 + ((p_var1 * ((int32_t)bme_calib.dig_P5)) << 1);
    p_var2 = (p_var2 >> 2) + (((int32_t)bme_calib.dig_P4) << 16);
    p_var1 = (((((int32_t)bme_calib.dig_P3) * (((p_var1 >> 2) * (p_var1 >> 2)) >> 13)) >> 3) + ((((int32_t)bme_calib.dig_P2) * p_var1) >> 1)) >> 18;
    p_var1 = ((((32768 + p_var1)) * ((int32_t)bme_calib.dig_P1)) >> 15);

    if (p_var1 == 0)
    {
        *press = 0; // Avoid division by zero
    }
    else
    {
        p = (((uint32_t)(((int32_t)1048576) - adc_P) - (p_var2 >> 12))) * 3125;
        if (p < 0x80000000)
        {
            p = (p << 1) / ((uint32_t)p_var1);
        }
        else
        {
            p = (p / (uint32_t)p_var1) * 2;
        }
        p_var1 = (((int32_t)bme_calib.dig_P9) * ((int32_t)(((p >> 3) * (p >> 3)) >> 13))) >> 12;
        p_var2 = (((int32_t)(p >> 2)) * ((int32_t)bme_calib.dig_P8)) >> 13;
        *press = (uint32_t)((int32_t)p + ((p_var1 + p_var2 + ((int32_t)bme_calib.dig_P7)) >> 4));
    }

    // Humidity Compensation
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)bme_calib.dig_H4) << 20) - (((int32_t)bme_calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)bme_calib.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)bme_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)bme_calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)bme_calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    *hum = (uint32_t)(v_x1_u32r >> 12); // %RH * 1024
}