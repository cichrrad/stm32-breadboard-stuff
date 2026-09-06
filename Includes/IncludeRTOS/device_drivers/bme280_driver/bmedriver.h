#ifndef BMEDRIVER_H
#define BMEDRIVER_H
#include <stdint.h>
#include <stdio.h>


typedef struct {
    volatile int32_t temperature;
    volatile uint32_t pressure;
    volatile uint32_t humidity;
} BME280_Data;

void BME280_Init(void);
void BME280_TriggerMeasurement(void);
void BME280_FetchData(BME280_Data *data);
void BME280_FormatStrings(const BME280_Data *data, char *t_str, char *p_str, char *h_str);

#endif