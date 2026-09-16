#include "game_fp.h"

void jump(PlayerFlappyBird *p)
{
    p->dy -= p->jump_momentum;
}

void gameTick(PlayerFlappyBird *p, GameStateFlappyBird *gs)
{
    gs->tick_count++;
    // move in y
    
    // temp re-center when falled
    if (p->y >= 48 || p->y <= 0)
    {
        // break from game
        p->y = 24;
        p->dy = 0;
        gs->running = false;
    }
    // apply gravity
    // move in x
    if (gs->tick_count == gs->slide_after_ticks)
    {
        gs->tick_count = 0;
        // move pipes
        p->y += p->dy;

        // TODO
    }

    // check collision
    // TODO

    // conditionally add pipe
    // TODO
}
