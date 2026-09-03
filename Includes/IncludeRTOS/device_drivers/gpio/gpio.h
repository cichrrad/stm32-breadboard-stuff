#ifndef DEVICE_DRIVERS_GPIO_H
#define DEVICE_DRIVERS_GPIO_H
#include <stdint.h>
#include "cmsis_headers/stm32g491xx.h"

// DEFINES
// in safety-critical, this is discouraged because of lack of typecheck etc...
// eventually static inline functions should be used
#define MACRO_GPIO_MODER_RESET(port, pin)   ((port)->MODER      &= ~(0x3UL << ((pin) * 2)))
#define MACRO_GPIO_OSPEEDR_RESET(port, pin) ((port)->OSPEEDR    &= ~(0x3UL << ((pin) * 2)))
#define MACRO_GPIO_PUPDR_RESET(port, pin)   ((port)->PUPDR      &= ~(0x3UL << ((pin) * 2)))
#define MACRO_GPIO_OTYPER_RESET(port, pin)  ((port)->OTYPER     &= ~(0x1UL << (pin)))

void GPIO_Init(GPIO_TypeDef *port,
               uint8_t pin,
               uint8_t mode,
               uint8_t outputType,
               uint8_t outputSpeed,
               uint8_t pullUpDown);

void GPIO_WritePin(GPIO_TypeDef *port,
                   uint8_t pin,
                   uint8_t value);

#endif /* DEVICE_DRIVERS_GPIO_H */