# Notes

- Once again, this is basically `001`, but step closer towards *production*-esque implementation.
- Uses CMSIS (Common Microcontroller Software Interface Standard) for interfacing with the board peripherals. This is an abstraction built upon mapping structs to registers, just like we did in `002` with `USART_TypeDef`.
- All the files are `./Include/device_headers` (from repo root). Do note I changed all (2) includes to `<cstdint>` to be `<stdint.h>` instead. Other then that they can all be sourced from [STMicroelectronics Github](https://github.com/STMicroelectronics) - for mine board, I looked [here](https://github.com/STMicroelectronics/STM32CubeG4/tree/master)