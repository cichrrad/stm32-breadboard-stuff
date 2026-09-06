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

#define TIME_SERIES_LEN 120U

static UITextType ui_current_val = {
    .val = "XX.XX",
    .x = 5,
    .y = 5,
    .fill_state = true,
    .solid_bg = false};

typedef struct
{
    float time_series[TIME_SERIES_LEN];

    uint8_t current_index;
    uint32_t min;
    uint32_t max;

    char *prefix_label;
    char *postfix_unit;
} BMEMetric_type;

static BMEMetric_type bme_tmp = {
    .min = 0,
    .max = 40,
    .current_index = 0,
    .prefix_label = "TMP: ",
    .postfix_unit = " C"};

static BMEMetric_type bme_hum = {
    .min = 0,
    .max = 100,
    .current_index = 0,
    .prefix_label = "HMD: ",
    .postfix_unit = " %%"};

static BMEMetric_type bme_prs = {
    .min = 900,
    .max = 1100,
    .current_index = 0,
    .prefix_label = "PRS: ",
    .postfix_unit = " hPa"};

static BMEMetric_type *active_metric;
SemaphoreHandle_t xMetricMutex = NULL;

void vSensorTask(void *pvParameters) {
    BME280_Init();


    while(1){
        // TODO
    }
};

void vRenderTask(void *pvParameters)
{
    DD_Init();

    bool hbeat = true;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    // ~1FPS target to update top banner value
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);

    while (1)
    {
        dd_clear();
        dd_set_pixel(127,0,hbeat);
        hbeat = !hbeat;
        
        // TODO

        dd_update();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

void vInputTask(void *pvParameters) {
    // TODO
};

int main(void)
{
    for(int i = 0; i < TIME_SERIES_LEN; i++){
        bme_hum.time_series[i] = 0.1f;
        bme_prs.time_series[i] = 0.1f;
        bme_tmp.time_series[i] = 0.1f;
    }

    active_metric = &bme_hum;
    
    // Create our single test task
    xTaskCreate(vRenderTask, "RenderTask", 256, NULL, 1, NULL);

    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    }
    return 0;
}