#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <float.h>

#include "gpio.h"
#include "ddriver.h"
#include "ui_widgets.h"
#include "bmedriver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define CHART_VIEWPORT_X 4U
#define CHART_VIEWPORT_Y 20U
#define CHART_VIEWPORT_WIDTH 120U
#define CHART_VIEWPORT_HEIGHT 40U

#define TIME_SERIES_LEN CHART_VIEWPORT_WIDTH
#define MOVE_PLOT_AFTER 360U
#define UI_REFRESH_RATE_MS 1000U
#define SPOTLIGHT_PERIOD 30U

static UITextType ui_current_val = {
    .val = "XX.XX",
    .x = 4,
    .y = 5,
    .fill_state = true,
    .solid_bg = false};

typedef enum
{
    TEMPERATURE = 0,
    PRESSURE,
    HUMIDITY,
    METRICS_COUNT
} e_METRIC;

typedef struct
{
    float time_series[TIME_SERIES_LEN];

    uint8_t current_index;
    uint32_t min;
    uint32_t max;
    e_METRIC metric;

} BMEMetric_type;

static BMEMetric_type bme_tmp = {
    .min = 5,
    .max = 35,
    .current_index = 0,
    .metric = TEMPERATURE};

static BMEMetric_type bme_hum = {
    .min = 0,
    .max = 100,
    .current_index = 0,
    .metric = HUMIDITY};

static BMEMetric_type bme_prs = {
    .min = 900,
    .max = 1100,
    .current_index = 0,
    .metric = PRESSURE};

static BMEMetric_type *active_metric;
SemaphoreHandle_t xMetricMutex = NULL;

volatile bool bme280_is_initialized = false;
BME280_Data sensor_data = {
    .humidity = 0UL,
    .pressure = 0UL,
    .temperature = 0UL};

void render_time_series()
{

    for (int i = 0; i < TIME_SERIES_LEN; i++)
    {
        /// start at the oldest index and wrap around
        int buffer_idx = (active_metric->current_index + i) % TIME_SERIES_LEN;
        float val = active_metric->time_series[buffer_idx];

        // bound between 0.0 - 1.0 based on bounds
        float range = (float)(active_metric->max - active_metric->min);
        float normalized = (val - (float)active_metric->min) / range;

        // Clamp it in case of spikes
        if (normalized < 0.0f)
            normalized = 0.0f;
        if (normalized > 1.0f)
            normalized = 1.0f;

        // Map to pixel height
        int y_pixel = ((float)CHART_VIEWPORT_Y) + ((float)(CHART_VIEWPORT_HEIGHT) - (int)(normalized * ((float)CHART_VIEWPORT_HEIGHT)));

        // Plot the dot
        dd_set_pixel(CHART_VIEWPORT_X + i, y_pixel, true);
    }
}

void vSensorTask(void *pvParameters)
{
    BME280_Init();
    bme280_is_initialized = true;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    // this polls
    const TickType_t xFrequency = pdMS_TO_TICKS(UI_REFRESH_RATE_MS);
    uint32_t polls_per_plot = 0;
    uint32_t spotlight_count = 0;
    bool first_plot_poll = true;
    while (1)
    {
        BME280_TriggerMeasurement();
        vTaskDelay(pdMS_TO_TICKS(10));
        BME280_FetchData(&sensor_data);
        polls_per_plot++;
        spotlight_count++;

        if (first_plot_poll)
        {
            first_plot_poll = false;
            // scale accordingly
            bme_hum.time_series[bme_hum.current_index] = sensor_data.humidity / 1024.0f;
            bme_prs.time_series[bme_prs.current_index] = sensor_data.pressure / 100.0f;
            bme_tmp.time_series[bme_tmp.current_index] = sensor_data.temperature / 100.0f;
        }

        if (polls_per_plot == MOVE_PLOT_AFTER)
        {

            polls_per_plot = 0;
            first_plot_poll = true;
            bme_hum.current_index = (bme_hum.current_index + 1) % TIME_SERIES_LEN;
            bme_prs.current_index = (bme_prs.current_index + 1) % TIME_SERIES_LEN;
            bme_tmp.current_index = (bme_tmp.current_index + 1) % TIME_SERIES_LEN;
        }

        if(spotlight_count == SPOTLIGHT_PERIOD){
            spotlight_count = 0;
            if (active_metric->metric == TEMPERATURE){
                active_metric = &bme_hum;
            }
            else if(active_metric->metric == HUMIDITY){
                active_metric = &bme_prs;
            }
            else{
                active_metric = &bme_tmp;
            }
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

void vRenderTask(void *pvParameters)
{
    DD_Init();

    bool hbeat = true;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // ~1FPS target to update top banner value
    const TickType_t xFrequency = pdMS_TO_TICKS(UI_REFRESH_RATE_MS);
    char t_str[16];
    char h_str[16];
    char p_str[16];

    while (1)
    {
        dd_clear();
        dd_set_pixel(127, 0, hbeat);
        hbeat = !hbeat;
        if (bme280_is_initialized)
        {
            BME280_FormatStrings(&sensor_data, t_str, p_str , h_str);
            switch (active_metric->metric)
            {
            case TEMPERATURE:
                ui_draw_string(&ui_current_val, t_str);
                break;
            case PRESSURE:
                ui_draw_string(&ui_current_val, p_str);
                break;
            case HUMIDITY:
                ui_draw_string(&ui_current_val, h_str);
                break;
            default:
                ui_draw_string(&ui_current_val, "????");
                break;
            }
        }

        dd_draw_rect(4, 20, TIME_SERIES_LEN, 40, true);

        render_time_series();

        dd_update();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

void vInputTask(void *pvParameters) {
    // TODO
};

// NOTE -- Init task ? would make it much simpler

int main(void)
{
    for (int i = 0; i < TIME_SERIES_LEN; i++)
    {
        bme_hum.time_series[i] = 0.0f;
        bme_prs.time_series[i] = 0.0f;
        bme_tmp.time_series[i] = 0.0f;
    }

    active_metric = &bme_tmp;

    xTaskCreate(vRenderTask, "RenderTask", 256, NULL, 1, NULL);
    xTaskCreate(vSensorTask, "SensorTask", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    }
    return 0;
}