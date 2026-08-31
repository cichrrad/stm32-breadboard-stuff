#ifndef BUTTON_H
#define BUTTON_H
#include "stm32g491xx.h"
#include <stdbool.h>

void Button_Init(void);
bool Button_IsPressed(void);


#endif