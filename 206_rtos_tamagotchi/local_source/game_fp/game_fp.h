#ifndef GAME_FP_H
#define GAME_FP_H

#include <stdint.h>
#include <stdbool.h>

#define FLAPPY_BIRD_VIEWPORT_X 28
#define FLAPPY_BIRD_VIEWPORT_Y 16

#define FLAPPY_BIRD_SIZE 2
#define FLAPPY_BIRD_MAX_PIPES 8

typedef struct
{
    uint8_t x;
    uint8_t width;
    uint8_t height;
    bool active;
} ObjectPipeFlappyBird;

typedef struct
{
    bool running;
    uint8_t score;
    uint8_t slide_after_ticks;
    uint8_t gravity;

    uint8_t pipe_min_width;
    uint8_t pipe_max_width;

    uint8_t gap_min_width;
    uint8_t gap_max_width;

    uint32_t tick_count;
} GameStateFlappyBird;

typedef struct
{
    uint8_t x;
    uint8_t y;
    int16_t dy;
    uint8_t jump_momentum;
} PlayerFlappyBird;

void jump(PlayerFlappyBird *p);
void gameTick(PlayerFlappyBird *p, GameStateFlappyBird *gs);

#endif