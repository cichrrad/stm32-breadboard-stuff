#include "gpio.h"


void GPIO_Init(GPIO_TypeDef *port,
               uint8_t pin,
               uint8_t mode,
               uint8_t outputType,
               uint8_t outputSpeed,
               uint8_t pullUpDown)

{

    // Resets
    MACRO_GPIO_MODER_RESET(port,pin);
    MACRO_GPIO_OTYPER_RESET(port,pin);
    MACRO_GPIO_OSPEEDR_RESET(port,pin);
    MACRO_GPIO_PUPDR_RESET(port,pin);

    // Inits
    // TODO - masking for uint8_t's to actually only care about
    // relevant bits (here just first 2) ???
    port->MODER |= ((uint32_t)(mode << (pin * 2U)));
    port->OTYPER |= ((uint32_t)(outputType << pin));
    port->OSPEEDR |= ((uint32_t)(outputSpeed << (pin * 2U)));
    port->PUPDR |= ((uint32_t)(pullUpDown << (pin * 2U)));
}

void GPIO_WritePin(GPIO_TypeDef *port, uint8_t pin, uint8_t value)
{
    // SET on any non-zero
    if(value){
        port->BSRR = (1UL << pin);
    }
    // RESET on 0
    else{
        port->BSRR = (1UL << (pin + 16UL));
    }
}
