#ifndef INPUT_DRIVER_H
#define INPUT_DRIVER_H

#include <stdbool.h>

typedef struct {
    volatile bool pet_flag;
    volatile bool feed_flag;
    volatile bool play_flag;
} ButtonState;

extern ButtonState g_buttons;

void Input_Init(void);
void Input_Update(void);

#endif // INPUT_DRIVER_H