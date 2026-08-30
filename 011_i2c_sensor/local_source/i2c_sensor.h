#ifndef I2C_SENSOR_H
#define I2C_SENSOR_H
#include <stdint.h>

typedef struct {
    uint16_t dig_T1; int16_t dig_T2; int16_t dig_T3;
    uint16_t dig_P1; int16_t dig_P2; int16_t dig_P3; 
    int16_t dig_P4;  int16_t dig_P5; int16_t dig_P6; 
    int16_t dig_P7;  int16_t dig_P8; int16_t dig_P9;
    uint8_t  dig_H1; int16_t dig_H2; uint8_t  dig_H3; 
    int16_t  dig_H4; int16_t dig_H5; int8_t   dig_H6;
} BME280_CalibData;

extern BME280_CalibData bme_calib;

void I2C1_Init(void);

uint8_t BME280_ReadID(void);
void BME280_ReadCalibration(void);
void BME280_Start(void);
void BME280_ReadData(int32_t *temp, uint32_t *press, uint32_t *hum);

#endif