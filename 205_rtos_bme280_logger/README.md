> Still WIP

# FreeRTOS BME280 Temperature/Humidity/Pressure sensor logger with chart plotting

TODO write out

---

## DEMO

> These are after ~11 hours of running overnight in a room with open window into the early morning. Needle for the chart was moved after 360 polls = whole chart logs 12 hours after 6 minute intervals (1 pixel of the chart on X axis is 6 minute window). Chart goes LEFT -> RIGHT, meaning latest data are on the right side, while oldest on the left side.

![demo_temp](./assets/demo_temp.jpg)

![demo_hum](./assets/demo_hum.jpg)

![demo_press](./assets/demo_press.jpg)

---

## Overview & Architecture 

TODO write out 

## TODOs

* Rewrite `sensor_hw` layer to not spin-lock when waiting for transfer back
* Add Input polling to change `active_metric` and reset spotlight counter when pressed.
* have separate array for logging all polls within recorded range and have the chart plot not every first poll for that pixel in time series, BUT an average from that window
* Have the top UI show current value | min | max ? (if it fits lmao)
* Currently, there is no synchronization between sensor data poll and rendering those data, meaning render task might plot last second data twice, moments before new ones are fetched -- not a big issue, because we dont expect metric to naturally spike in a major way and IF they do, we will see it another poll in worst-case scenario. Only scenario where this might be actually harmful is when the spike concludes within that second window before next poll, basically never being shown by the UI.