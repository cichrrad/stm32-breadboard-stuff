#include "rps.h"
#include "ddriver.h"


static char current_pick;
static uint8_t player_score;
static uint8_t mcu_score;
static bool confirmed_pick;
static uint8_t temp_radius;

void rps_initFns(void)
{
    current_pick = 'x';
    player_score = 0;
    mcu_score = 0;
    confirmed_pick = false;
    temp_radius = 5;
}

void rps_renderFn(void)
{
    dd_fill_rect(28, 16, 72, 48, false);
    dd_draw_circle(64, 32, temp_radius, true);
}

void rps_updateFn(void)
{
}

void rps_select_rock(void)
{
    temp_radius = 1;
}

void rps_select_paper(void)
{
    temp_radius = 3;
}

void rps_select_scissors(void)
{
    temp_radius = 8;
}
