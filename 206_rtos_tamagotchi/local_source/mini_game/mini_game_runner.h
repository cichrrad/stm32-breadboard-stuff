#ifndef MINI_GAME_RUNNER_H
#define MINI_GAME_RUNNER_H

#include <stdint.h>
#include <float.h>
#include <stdbool.h>
#include "input.h"

typedef void (*fn)(void);

typedef enum
{
    MINI_GAME_RPS = 0,
    MINI_GAME_COUNT
} Minigame_t;

typedef struct {
    fn initFn;
    fn updateFn;
    fn renderFn;
    InputMapping im;
} GameInstance_t;

typedef struct
{
    bool running;
    Minigame_t currentMinigame;
    GameInstance_t games[MINI_GAME_COUNT];
} MiniGameRunner_t;

#endif