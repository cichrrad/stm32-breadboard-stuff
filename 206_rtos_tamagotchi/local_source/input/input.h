#ifndef INPUT_H
#define INPUT_H

typedef void (*actionFn)(void);

typedef enum
{
    INPUT_BTN1 = 0,
    INPUT_BTN2,
    INPUT_BTN3,
    INPUTS_COUNT
} GameInput;

typedef struct
{
    GameInput inputs[INPUTS_COUNT];
    actionFn actions[INPUTS_COUNT];

} InputMapping;

#endif