#include "rps.h"
#include "ddriver.h"


static char current_pick;
static uint8_t player_score;
static uint8_t mcu_score;
static bool confirmed_pick;

void rps_initFns(void)
{
    current_pick = 'x';
    player_score = 0;
    mcu_score = 0;
    confirmed_pick = false;
}

void rps_renderFn(void)
{
    dd_fill_rect(28, 16, 72, 48, false);
    dd_draw_circle(64, 32, 3, true);
}

void rps_updateFn(void)
{
}

void rps_select_rock(void)
{
}

void rps_select_paper(void)
{
}

void rps_select_scissors(void)
{
}
