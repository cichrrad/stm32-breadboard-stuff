#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "bmedriver.h"
#include "usart_driver.h"

#define POLL_RATE_MS 500
#define USART_SEND_RATE_MS 1000

static_assert(POLL_RATE_MS <= USART_SEND_RATE_MS, "Poll rate must be faster than usart send rate");

#define QUEUE_SIZE 1

QueueHandle_t xSensorQueue;

typedef struct
{
    BME280_Data sensor_data;
    uint32_t timestamp;
} SensorPacket_t;

void vSensorTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(POLL_RATE_MS);
    SensorPacket_t packet = {
        .sensor_data = {
            .humidity = 0UL,
            .pressure = 0UL,
            .temperature = 0UL

        },
        .timestamp = 0UL

    };

    while (1)
    {
        BME280_TriggerMeasurement();
        // wait so the data are ready
        // in sensor registers
        vTaskDelay(pdMS_TO_TICKS(10));

        BME280_FetchData(&packet.sensor_data);
        packet.timestamp = xTaskGetTickCount();
        xQueueOverwrite(xSensorQueue, &packet);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

void vUSARTTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(USART_SEND_RATE_MS);

    SensorPacket_t packet;
    char t_str[16];
    char p_str[24];
    char h_str[16];
    char uart_buf[128];

    while (1)
    {

        if (xQueueReceive(xSensorQueue, &packet, 0) == pdTRUE)
        {
            BME280_FormatStrings(&packet.sensor_data, t_str, p_str, h_str);

            /* expected format example =======
            Time: 25511 ms 
            Temp: 24.81 C 
            Press: 984.91 hPa 
            Hum: 42.03 %            
            */
            snprintf(uart_buf, sizeof(uart_buf),
                     "Time: %lu ms \r\nTemp: %s \r\nPress: %s \r\nHum: %s\r\n",
                     (uint32_t)packet.timestamp,
                     t_str,
                     p_str,
                     h_str);

            USART2_SendString(uart_buf);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
};

int main(void)
{
    BME280_Init();
    USART2_Init(115200, 16000000);

    xSensorQueue = xQueueCreate(QUEUE_SIZE, sizeof(SensorPacket_t));

    xTaskCreate(vSensorTask, "SensorTask", 256, NULL, 1, NULL);
    xTaskCreate(vUSARTTask, "USARTTask", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1)
    {
        // UNREACHABLE
    }
    return 0;
}