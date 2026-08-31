#ifndef FLASH_H
#define FLASH_H

#include "stm32g491xx.h"
#include <stdint.h>
#include <stdbool.h>

#define FLASH_KEY1 0x45670123U
#define FLASH_KEY2 0xCDEF89ABU

// Your app starts at 0x08008000
bool Flash_Write_App(uint8_t *data, uint32_t length);

#endif