#include "stm32g491xx.h"
#include "device_drivers/display_driver/ddriver.h"
#include "device_drivers/display_driver/ui_widgets.h"
#include "device_drivers/tick_engine/systick_timer.h"
#include "local_source/i2c_sensor.h"

#include <stdint.h>

void format_sensor_data(char *buf, const char *prefix, int32_t val, const char *suffix)
{
    char temp_buf[16];
    int i = 0;
    bool is_neg = false;

    // Handle negative values (for sub-zero temperatures)
    if (val < 0)
    {
        is_neg = true;
        val = -val;
    }

    // Extract integer and fractional parts (assuming input is scaled by 100)
    int32_t int_part = val / 100;
    int32_t frac_part = val % 100;

    // 1. Build fractional part first (always 2 digits)
    temp_buf[i++] = (frac_part % 10) + '0';
    temp_buf[i++] = (frac_part / 10) + '0';
    temp_buf[i++] = '.';

    // 2. Build integer part
    if (int_part == 0)
    {
        temp_buf[i++] = '0';
    }
    else
    {
        while (int_part > 0)
        {
            temp_buf[i++] = (int_part % 10) + '0';
            int_part /= 10;
        }
    }

    // 3. Add negative sign if needed
    if (is_neg)
    {
        temp_buf[i++] = '-';
    }

    // 4. Assemble the final string: Prefix + Number (reversed) + Suffix
    while (*prefix)
        *buf++ = *prefix++;
    while (i > 0)
        *buf++ = temp_buf[--i];
    while (*suffix)
        *buf++ = *suffix++;

    *buf = '\0';
}

int main(void)
{

    SysTick_Init(16000000);
    DD_Init();
    I2C1_Init();
    BME280_ReadCalibration();
    BME280_Start();

    UITextType hbeat = {
        .x = 1,
        .y = 1,
        .val = "hbeat",
        .fill_state = true,
        .solid_bg = true};

    UITextType temp = {
        .x = 1,
        .y = 20,
        .val = "temp",
        .fill_state = true,
        .solid_bg = true};

    UITextType pressure = {
        .x = 1,
        .y = 35,
        .val = "press",
        .fill_state = true,
        .solid_bg = true};

    UITextType humid = {
        .x = 1,
        .y = 50,
        .val = "humid",
        .fill_state = true,
        .solid_bg = true};

    uint32_t last_tick_ui_update = GetTick();
    bool flip = false;
    while (1)
    {
        if (SysTick_IsElapsed(last_tick_ui_update, 1000))
        {
            last_tick_ui_update = GetTick();
            dd_clear();
            ui_draw_string(&hbeat, (flip ? "TICK" : "TOCK"));
            flip = !flip;

            int32_t temp_val;
            uint32_t press_val, hum_val;
            BME280_ReadData(&temp_val, &press_val, &hum_val);

            // Scale humidity from base-1024 to base-100 to match formatter
            // Max humidity is 100 * 1024 = 102400.
            // 102400 * 100 fits safely inside a 32-bit integer, so we don't need 64-bit math
            uint32_t hum_base100 = (hum_val * 100) / 1024;

            char t_str[24], p_str[24], h_str[24];

            format_sensor_data(t_str, "T: ", temp_val, " C");
            format_sensor_data(p_str, "P: ", press_val, " hPa");
            format_sensor_data(h_str, "H: ", hum_base100, " %");

            ui_draw_string(&temp, t_str);
            ui_draw_string(&pressure, p_str);
            ui_draw_string(&humid, h_str);
            dd_update();
        }
        __WFI();
    }
    return 0;
}