# Notes

- This program uses I2C to communicate with and read data from BME280 purple board sensor for temperature, pressure, and humidity. Data are displayed on oled display (via display driver for it, see `010_spi_display_driver`, for wiring of it, see `008_spi_oled`). 

- To convert and display the data in human readable form, a rather convoluted process (reading the chip calibration and using it in series of equations to arrive at the converted values) is needed. Correct and *production* way of doing this would be using [BOSH official API](https://github.com/boschsensortec/BME280_SensorAPI/tree/master). Here, we read the configuration in `BME280_ReadCalibration` function, which stores it to `extern BME280_CalibData bme_calib` struct, which is then used in `BME280_ReadData` to actually make sense of the data. (Thanks for pulling up the configuration sheet, Gemini).

## I2C Init & Wiring

- I2C is initialized in `I2C_Init` function, doing the usual:

1. Enable clock for the Ports whose pins we want to use (`GPIOB`).
2. Enable clock for `I2C1` we will use for communication.
3. Set pins `PB8` and `PB9` (`D14` and `D15` labels on female Arduino pins) to be Alternate function 4 (for `I2C1`), open-drain. These are the main pins, functioning as data and clock for the I2C.
4. Set Timing register for `I2C1`.

- It is wired like so:

| STM32       | BME280 |
| ----------- | ------ |
| 3V3 (Power) | VCC    |
| GND         | GND    |
| SCL/D15     | SCL    |
| SDA/D14     | SDA    |

- Then wire BME280 CSB to + rail on the breadboard to select I2C mode
- Lastly, wire BME280 SDO to - rail on the breadboard to set I2C address to `0x76`
