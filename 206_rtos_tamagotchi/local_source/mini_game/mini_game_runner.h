#ifndef MINI_GAME_RUNNER_H
#define MINI_GAME_RUNNER_H

#include <stdint.h>
#include <float.h>
#include <stdbool.h>
#include "input.h"

typedef void (*fn)(void);
typedef void(*ufn)(bool*);

typedef enum
{
    MINI_GAME_RPS = 0,
    MINI_GAME_COUNT
} Minigame_t;

typedef struct {
    fn initFn;
    ufn updateFn;
    fn renderFn;
    InputMapping im;
    bool exit_flag;
} GameInstance_t;

typedef struct
{
    bool running;
    Minigame_t currentMinigame;
    GameInstance_t games[MINI_GAME_COUNT];
} MiniGameRunner_t;

GameInstance_t* mgr_get_current_game_instance(MiniGameRunner_t* runner);
void mgr_run_minigame(MiniGameRunner_t* runner, Minigame_t game);
bool mgr_is_running(MiniGameRunner_t* runner);
void mgr_call_initFn(MiniGameRunner_t* runner);
void mgr_call_updateFn(MiniGameRunner_t* runner);
void mgr_call_renderFn(MiniGameRunner_t* runner);

#endif