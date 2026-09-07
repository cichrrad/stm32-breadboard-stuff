# FreeRTOS BME280 Temperature/Humidity/Pressure sensor logger with chart plotting

---

## DEMO

![demo_temp](./assets/demo_temp.jpg)

![demo_hum](./assets/demo_hum.jpg)

![demo_press](./assets/demo_press.jpg)


## TODOs

* Rewrite `sensor_hw` layer to not spin-lock when waiting for transfer back
* Add Input polling to change `active_metric` and reset spotlight counter when pressed.