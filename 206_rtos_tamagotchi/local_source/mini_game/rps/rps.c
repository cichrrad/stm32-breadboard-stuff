#include "rps.h"

#include "ddriver.h"
#include "ui_widgets.h"
#include "utils_rng.h"

#include "input.h"

enum RPSGameState
{
    WAITING_FOR_PICK = 0, // wating for first input
    PICK_SELECTED,
    PICK_CONFIRMED,
    AFTER_PICK_SHOW,
    RESET,
    END_WIN,
    END_LOSS,
    RPS_STATE_COUNT
};

typedef struct
{
    UITextType ui_label_rock;
    UITextType ui_label_paper;
    UITextType ui_label_scissors;
} rps_ui_t;

static rps_ui_t ui = {
    .ui_label_rock =
        {
            .val = "ROCK",
            .fill_state = true,
            .solid_bg = false,
            .x = 28 + 8,
            .y = 16 + 8,
        },
    .ui_label_paper =
        {
            .val = "PAPER",
            .fill_state = true,
            .solid_bg = false,
            .x = 28 + 8,
            .y = 16 + 2 * 8,
        },
    .ui_label_scissors =
        {
            .val = "SCSRS",
            .fill_state = true,
            .solid_bg = false,
            .x = 28 + 8,
            .y = 16 + 3 * 8,
        },

};

static enum RPSGameState current_state;
static char current_pick;
static char pre_seleced;
static char mcu_selected;

static uint32_t tick_counter;
static uint32_t show_period_ticks;

static uint8_t player_score;
static uint8_t mcu_score;
static bool confirmed_pick;

void dd_draw_static_ui()
{
    ui_draw_string(&ui.ui_label_rock, ui.ui_label_rock.val);
    ui_draw_string(&ui.ui_label_paper, ui.ui_label_paper.val);
    ui_draw_string(&ui.ui_label_scissors, ui.ui_label_scissors.val);
}

void dd_draw_pick(const char selected, const char confirmed)
{
    switch (selected)
    {
    case 'R':
        dd_draw_triangle(ui.ui_label_rock.x - 10, ui.ui_label_rock.y + 3, ui.ui_label_rock.x - 10, ui.ui_label_rock.y + 4 + 3, ui.ui_label_rock.x - 5, ui.ui_label_rock.y + 2 + 3, true);
        break;
    case 'P':
        dd_draw_triangle(ui.ui_label_paper.x - 10, ui.ui_label_paper.y + 3, ui.ui_label_paper.x - 10, ui.ui_label_paper.y + 4 + 3, ui.ui_label_paper.x - 5, ui.ui_label_paper.y + 2 + 3, true);

        break;
    case 'S':
        dd_draw_triangle(ui.ui_label_scissors.x - 10, ui.ui_label_scissors.y + 3, ui.ui_label_scissors.x - 10, ui.ui_label_scissors.y + 4 + 3, ui.ui_label_scissors.x - 5, ui.ui_label_scissors.y + 2 + 3, true);
        break;

    default:
        break;
    }

    switch (confirmed)
    {
    case 'R':
        dd_fill_triangle(ui.ui_label_rock.x - 10, ui.ui_label_rock.y + 3, ui.ui_label_rock.x - 10, ui.ui_label_rock.y + 4 + 3, ui.ui_label_rock.x - 5, ui.ui_label_rock.y + 2 + 3, true);
        break;
    case 'P':
        dd_fill_triangle(ui.ui_label_paper.x - 10, ui.ui_label_paper.y + 3, ui.ui_label_paper.x - 10, ui.ui_label_paper.y + 4 + 3, ui.ui_label_paper.x - 5, ui.ui_label_paper.y + 2 + 3, true);

        break;
    case 'S':
        dd_fill_triangle(ui.ui_label_scissors.x - 10, ui.ui_label_scissors.y + 3, ui.ui_label_scissors.x - 10, ui.ui_label_scissors.y + 4 + 3, ui.ui_label_scissors.x - 5, ui.ui_label_scissors.y + 2 + 3, true);
        break;

    default:
        break;
    }
};

void dd_draw_mcu_choice(const char selected)
{
    switch (selected)
    {
    case 'R':
        dd_draw_triangle(100, ui.ui_label_rock.y + 3, 100, ui.ui_label_rock.y + 4 + 3, 95, ui.ui_label_rock.y + 2 + 3, true);
        break;
    case 'P':
        dd_draw_triangle(100, ui.ui_label_paper.y + 3, 100, ui.ui_label_paper.y + 4 + 3, 95, ui.ui_label_paper.y + 2 + 3, true);

        break;
    case 'S':
        dd_draw_triangle(100, ui.ui_label_scissors.y + 3, 100, ui.ui_label_scissors.y + 4 + 3, 95, ui.ui_label_scissors.y + 2 + 3, true);
        break;

    default:
        break;
    }
}

void rps_mcu_select()
{
    uint32_t roll = (((uint64_t)utils_rand() * 3) >> 32);
    if (roll == 0)
    {
        // rock
        mcu_selected = 'R';
    }
    else if (roll == 1)
    {
        // paper
        mcu_selected = 'P';
    }
    else
    {
        // scissors
        mcu_selected = 'S';
    }
}

void rps_initFn(void)
{
    current_state = WAITING_FOR_PICK;
    current_pick = 'x';
    pre_seleced = 'x';
    mcu_selected = 'x';
    confirmed_pick = false;

    player_score = 0;
    mcu_score = 0;

    tick_counter = 0;
    show_period_ticks = 50 * 5;
}

void rps_renderFn(void)
{
    dd_fill_rect(0, 16, 128, 48, false);
    // dd_fill_rect(0, 16, 28, 48, true);
    // dd_fill_rect(100, 16, 28, 48, true);

    switch (current_state)
    {
    // render the options, no selection
    case WAITING_FOR_PICK:
        dd_draw_static_ui();
        break;
    case PICK_SELECTED:
        dd_draw_static_ui();
        dd_draw_pick(pre_seleced, current_pick);
        break;
    case PICK_CONFIRMED:
        dd_draw_static_ui();
        dd_draw_pick(pre_seleced, current_pick);
        break;
    case AFTER_PICK_SHOW:
        dd_draw_static_ui();
        dd_draw_pick(pre_seleced, current_pick);
        dd_draw_mcu_choice(mcu_selected);
        break;
    case END_WIN:
        ui_draw_string(&ui.ui_label_rock, "YOU");
        ui_draw_string(&ui.ui_label_paper, "WON!");
        break;
    case END_LOSS:
        ui_draw_string(&ui.ui_label_rock, "YOU");
        ui_draw_string(&ui.ui_label_paper, "LOST!");
        break;
    }
}

void rps_updateFn(bool *exit_flag)
{
    switch (current_state)
    {
    case WAITING_FOR_PICK:
        if (pre_seleced != 'x')
        {
            current_state = PICK_SELECTED;
        }
        break;
    case PICK_SELECTED:
        if (current_pick != 'x' && current_pick == pre_seleced)
        {
            current_state = PICK_CONFIRMED;
            confirmed_pick = true;
        }
        break;
    case PICK_CONFIRMED:
        // randomly choose
        if (mcu_selected == 'x')
        {
            rps_mcu_select();
        }

        if (mcu_selected == current_pick)
        {
            mcu_score++;
            player_score++;
        }
        else if (mcu_selected == 'R')
        {
            if (current_pick == 'P')
            {
                player_score++;
            }
            else
            {
                mcu_score++;
            }
        }
        else if (mcu_selected == 'P')
        {
            if (current_pick == 'S')
            {
                player_score++;
            }
            else
            {
                mcu_score++;
            }
        }
        // mcu_selected == S
        else if (current_pick == 'R')
        {
            player_score++;
        }
        else
        {
            mcu_score++;
        }

        // transition to showcase
        current_state = AFTER_PICK_SHOW;

        break;
    case AFTER_PICK_SHOW:
        if (tick_counter < show_period_ticks)
        {
            tick_counter++;
        }
        else
        {
            tick_counter = 0;

            if (mcu_score > player_score)
            {
                current_state = END_LOSS;
                break;
            }

            if (player_score > mcu_score)
            {
                current_state = END_WIN;
                break;
            }

            current_state = RESET;
        }
        break;
    case RESET:
        rps_initFn();
        break;
    case END_LOSS:
        if (tick_counter < show_period_ticks)
        {
            tick_counter++;
        }
        else
        {
            // to be sure
            rps_initFn();
            *exit_flag = true;
        }
        break;
    case END_WIN:
        if (tick_counter < show_period_ticks)
        {
            tick_counter++;
        }
        else
        {
            // to be sure
            rps_initFn();
            *exit_flag = true;
        }
        break;
    default:
        break;
    }
}

void rps_select_rock(void)
{
    if (confirmed_pick)
    {
        return;
    }

    if (pre_seleced == 'x')
    {
        pre_seleced = 'R';
    }
    else if (pre_seleced == 'R')
    {
        current_pick = 'R';
    }
    else
    {
        pre_seleced = 'R';
    }
}

void rps_select_paper(void)
{
    if (confirmed_pick)
    {
        return;
    }

    if (pre_seleced == 'x')
    {
        pre_seleced = 'P';
    }
    else if (pre_seleced == 'P')
    {
        current_pick = 'P';
    }
    else
    {
        pre_seleced = 'P';
    }
}

void rps_select_scissors(void)
{
    if (confirmed_pick)
    {
        return;
    }

    if (pre_seleced == 'x')
    {
        pre_seleced = 'S';
    }
    else if (pre_seleced == 'S')
    {
        current_pick = 'S';
    }
    else
    {
        pre_seleced = 'S';
    }
}
