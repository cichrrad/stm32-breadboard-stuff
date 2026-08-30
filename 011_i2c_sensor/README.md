# Notes

- This program uses I2C to communicate with and read data from BME280 purple board sensor for temperature, pressure, and humidity. Data are displayed on oled display (via display driver for it, see `010_spi_display_driver`, for wiring of it, see `008_spi_oled`). 

- To convert and display the data in human readable form, a rather convoluted process (reading the chip calibration and using it in series of equations to arrive at the converted values) is needed. Correct and *production* way of doing this would be using [BOSH official API](https://github.com/boschsensortec/BME280_SensorAPI/tree/master). Here, we read the configuration in `BME280_ReadCalibration` function, which stores it to `extern BME280_CalibData bme_calib` struct, which is then used in `BME280_ReadData` to actually make sense of the data. (Thanks for pulling up the configuration sheet, Gemini).

# I2C

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
- Lastly, wire BME280 SDO to - rail on the breadboard to set I2C address to `0x76`, which we will use to address the sensor

## I2C Functions

- MCU communicates with the sensor over I2C using only 2 wires -- SCL for synchronization, and SDA for data. While the communication is bi-directional, sensor is a slave and will only talk when spoken to. To *talk* to the sensor, MCU sends out a byte in format:

```
[7-bit addres][1-bit R/W flag]
```
Where only slave with matching address will be selected and react to next instructions, even if many slaves are connected on that same SDA wire. LSB is 0 for WRITE, and 1 for READ.

> Note the actual byte building is done by the HW of the MCU, we do basically all of the things by filling in `CR2` 32-bit register for `I2C1`. Most important thing to realize is that what will be LSB bit used as R/W flag is actually 10th bit in this register! So setting R/W is done by setting bit 10 (which the CMSIS macros expand to just fine, of course).

### `I2C1_ReadRegisters`

- Function for reading specified length of bytes from specific address. In order to specify starting addres and range, we:


1. Do a *dummy write* by sending WRITE command for 1 byte to the sensor with `I2C_CR2_START` so the sensor will expect and interpret next incoming byte as address to put its internal reading pointer to.
2. We send the register address we want to start reading from.
3. We send READ command with the specified length of bytes we want to read. We also set `I2C_CR2_AUTOEND` flag, so that after transfer, STOP flag will be auto-set.
4. We recieve the bytes in `for` loop, moving them out of RX register to a buffer.

### `I2C1_WriteRegister`

- Function for writing specified length of bytes to a specific starting address. Very much analogous to `I2C1_ReadRegisters`, and  actually more straightforward -- we send WRITE command for 2 bytes, then send the address, then send the value.

# BME280 layer

- On top of the SPI functions, abstraction for working with the sensor itself is implemented. This is because were we to just read bytes starting at address `0xF7` (Ref. Manual) of the sensor, it would seemingly be garbage data. Reason for this is that sensor data dump is obscured and incomplete part of the final results. From what I read up on, there are 2 reasons that go hand-in-hand:

1. Every single sensor is unique and behaves differently. Because of this, it is calibrated during production and some correction factors unique to that specific unit are written to its ROM. To get data of the sensor, you have to not only have the data it recorded, but these configuration metadata, and together, through a (crazy) set of equations (in `BME280_ReadData`), you arrive at the correct numbers. This ties in with the second reason...
2. Cost/Power usage - While both the data and configuration data are on the sensor, in order to report final numbers, you would have to add HW supporting and doing the math to be on the sensor, making it expensive, bigger, and use more power.