#include "pet.h"
#include "../tick_engine/systick_timer.h"

void Pet_updateStats(Pet *p, ButtonState *buttons)
{

    // Check buttons and tweak stats based on it
    if (buttons->feed_flag)
    {
        buttons->feed_flag = false;
        Pet_Eat(p);
    }
    if (buttons->pet_flag)
    {
        buttons->pet_flag = false;
        Pet_Pet(p);
    }
    if (buttons->play_flag)
    {
        buttons->play_flag = false;
        Pet_Play(p);
    }

    // Decay stats if untreated
    if (SysTick_IsElapsed(p->last_time_fed, p->food_change_time_ms))
    {
        p->last_time_fed = GetTick();
        if (p->food > p->food_change_factor)
        {
            p->food -= p->food_change_factor;
        }
        else
        {
            p->food = PET_MIN_STAT_VALUE;
        }
    }

    if (SysTick_IsElapsed(p->last_time_played_with, p->bored_change_time_ms))
    {
        p->last_time_played_with = GetTick();
        if (p->bored + p->bored_change_factor <= PET_MAX_STAT_VALUE)
        {
            p->bored += p->bored_change_factor;
        }
        else{
            p->bored = PET_MAX_STAT_VALUE;
        }
    }

    if (SysTick_IsElapsed(p->last_time_pet, p->alone_change_time_ms))
    {
        p->last_time_pet = GetTick();
        if (p->alone + p->alone_change_factor <= PET_MAX_STAT_VALUE)
        {
            p->alone += p->alone_change_factor;
        }
        else{
            p->alone = PET_MAX_STAT_VALUE;
        }
    }
}

void Pet_Eat(Pet *p)
{
    p->last_time_fed = GetTick();
    if (p->food + p->food_change_factor > PET_MAX_STAT_VALUE)
    {
        p->food = PET_MAX_STAT_VALUE;
    }
    else
    {
        p->food += p->food_change_factor;
    }
};

void Pet_Play(Pet *p)
{
    p->last_time_played_with = GetTick();
    if (p->bored < p->bored_change_factor)
    {
        p->bored = PET_MIN_STAT_VALUE;
    }
    else
    {
        p->bored -= p->bored_change_factor;
    }
};
void Pet_Pet(Pet *p) {
    p->last_time_pet = GetTick();
    if (p->alone < p->alone_change_factor)
    {
        p->alone = PET_MIN_STAT_VALUE;
    }
    else
    {
        p->alone -= p->alone_change_factor;
    }
}
void Pet_transition(Pet *p) {
    // TODO
};