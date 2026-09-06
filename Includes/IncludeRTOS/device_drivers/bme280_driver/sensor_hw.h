#ifndef SENSOR_HW_H
#define SENSOR_HW_H
#include <stdint.h>

void I2C1_Init(void);
void I2C1_ReadRegisters(uint8_t reg, uint8_t *buffer, uint8_t length);
void I2C1_WriteRegister(uint8_t reg, uint8_t value);

#endif