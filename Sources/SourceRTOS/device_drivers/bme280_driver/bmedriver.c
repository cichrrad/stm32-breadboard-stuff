#include "bmedriver.h"
#include "sensor_hw.h"

typedef struct {
    uint16_t dig_T1; int16_t dig_T2; int16_t dig_T3;
    uint16_t dig_P1; int16_t dig_P2; int16_t dig_P3; 
    int16_t dig_P4;  int16_t dig_P5; int16_t dig_P6; 
    int16_t dig_P7;  int16_t dig_P8; int16_t dig_P9;
    uint8_t  dig_H1; int16_t dig_H2; uint8_t  dig_H3; 
    int16_t  dig_H4; int16_t dig_H5; int8_t   dig_H6;
} BME280_CalibData;

static BME280_CalibData bme_calib;
static int32_t t_fine;

static void BME280_ReadCalibration(void)
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

void BME280_Init(void)
{
    I2C1_Init();
    BME280_ReadCalibration();
    
    // Set Humidity Oversampling to 1x (Register 0xF2).
    // Note: Changes to 0xF2 only take effect after writing to 0xF4.
    I2C1_WriteRegister(0xF2, 0x01); 
}

void BME280_TriggerMeasurement(void)
{
    // Write to 0xF4: Temp OS 1x (001), Press OS 1x (001), Forced Mode (01)
    // 001_001_01 = 0x25
    I2C1_WriteRegister(0xF4, 0x25);
}

void BME280_FetchData(BME280_Data *data)
{
    uint8_t raw[8];
    I2C1_ReadRegisters(0xF7, raw, 8); // Read Pressure, Temp, Humidity

    int32_t adc_P = (raw[0] << 12) | (raw[1] << 4) | (raw[2] >> 4);
    int32_t adc_T = (raw[3] << 12) | (raw[4] << 4) | (raw[5] >> 4);
    int32_t adc_H = (raw[6] << 8) | raw[7];

    // --- Temperature Compensation ---
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)bme_calib.dig_T1 << 1))) * ((int32_t)bme_calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)bme_calib.dig_T1)) * ((adc_T >> 4) - ((int32_t)bme_calib.dig_T1))) >> 12) * ((int32_t)bme_calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    data->temperature = (t_fine * 5 + 128) >> 8; // DegC * 100

    // --- Pressure Compensation (32-bit version) ---
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
        data->pressure = 0; // Avoid division by zero
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
        data->pressure = (uint32_t)((int32_t)p + ((p_var1 + p_var2 + ((int32_t)bme_calib.dig_P7)) >> 4));
    }

    // --- Humidity Compensation ---
    int32_t v_x1_u32r;
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)bme_calib.dig_H4) << 20) - (((int32_t)bme_calib.dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)bme_calib.dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)bme_calib.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)bme_calib.dig_H2) + 8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)bme_calib.dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    data->humidity = (uint32_t)(v_x1_u32r >> 12); // %RH * 1024
}

void BME280_FormatStrings(const BME280_Data *data, char *t_str, char *p_str, char *h_str)
{
    // Temperature: DegC * 100
    if (t_str) {
        int32_t t = data->temperature;
        int t_int = t / 100;
        int t_frac = (t < 0 ? -t : t) % 100;
        sprintf(t_str, "%d.%02d C", t_int, t_frac);
    }

    // Pressure: Pascals -> hPa (1 hPa = 100 Pa)
    if (p_str) {
        uint32_t p = data->pressure;
        uint32_t p_int = p / 100;
        uint32_t p_frac = p % 100;
        sprintf(p_str, "%lu.%02lu hPa", p_int, p_frac);
    }

    // Humidity: %RH * 1024 -> map to standard decimal percentage
    if (h_str) {
        uint32_t h = (data->humidity * 100) >> 10; 
        uint32_t h_int = h / 100;
        uint32_t h_frac = h % 100;
        sprintf(h_str, "%lu.%02lu %%", h_int, h_frac);
    }
}