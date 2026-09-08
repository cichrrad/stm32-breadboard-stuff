> Still WIP

# FreeRTOS BME280 Temperature/Humidity/Pressure sensor logger with chart plotting

TODO write out

---

## DEMO

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