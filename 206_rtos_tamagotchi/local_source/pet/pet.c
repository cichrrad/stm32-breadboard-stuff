#include "pet.h"
#include "utils_math.h"

void Pet_Calculate_Emotion(Pet *p)
{

    float food_fill = ((float)p->food / PET_MAX_STAT_VALUE);
    float bored_fill = ((float)p->bored / PET_MAX_STAT_VALUE);
    float alone_fill = ((float)p->alone / PET_MAX_STAT_VALUE);

    // starved to death
    if (p->food == PET_MIN_STAT_VALUE)
    {
        p->currentEmotion = EMOTION_RIP;
        p->alive = false;
        return;
    }

    // Famished
    if (food_fill <= 0.2)
    {
        p->food_status_severity = 3;
    }
    // Really Hungry
    else if (food_fill <= 0.4)
    {
        p->food_status_severity = 2;
    }
    // Hungry
    else if (food_fill <= 0.7)
    {
        p->food_status_severity = 1;
    }
    // OK
    else
    {
        p->food_status_severity = 0;
    }

    // Bored to death
    if (bored_fill >= 0.8)
    {
        p->bored_status_severity = 3;
    }
    // Really bored
    else if (bored_fill >= 0.6)
    {
        p->bored_status_severity = 2;
    }
    // Wants attention
    else if (bored_fill >= 0.3)
    {
        p->bored_status_severity = 1;
    }
    // OK
    else
    {
        p->bored_status_severity = 0;
    }

    // Devastated
    if (alone_fill >= 0.8)
    {
        p->alone_status_severity = 3;
    }
    // Feeling abandoned
    else if (alone_fill >= 0.6)
    {
        p->alone_status_severity = 2;
    }
    // Feeling left out
    else if (alone_fill >= 0.3)
    {
        p->alone_status_severity = 1;
    }
    // OK
    else
    {
        p->alone_status_severity = 0;
    }

    //================================

    // assigning emotions

    uint8_t severity_sum = p->alone_status_severity + p->food_status_severity + p->bored_status_severity;
    if (severity_sum >= 7)
    {
        p->currentEmotion = EMOTION_HEARTBROKEN;
        return;
    }

    if (severity_sum == 0)
    {
        p->currentEmotion = EMOTION_EXCITED;
        return;
    }

    if (severity_sum == 6)
    {
        p->currentEmotion = EMOTION_SICK;
        return;
    }

    // Take the major one
    if (severity_sum >= 3 && severity_sum <= 5)
    {

        if (p->food_status_severity == 3)
        {
            p->currentEmotion = EMOTION_HUNGRY;
            return;
        }

        if (p->alone_status_severity == 3)
        {
            p->currentEmotion = EMOTION_SAD;
            return;
        }

        if (p->bored_status_severity == 3)
        {
            p->currentEmotion = EMOTION_BORED;
            return;
        }

        // if nothing is major

        if (p->food_status_severity > 1)
        {
            p->currentEmotion = EMOTION_ANGRY;
            return;
        }

        p->currentEmotion = EMOTION_PINING;
        return;
    }

    p->currentEmotion = EMOTION_HAPPY;
}

void Pet_Update_Stats(Pet *p)
{

    p->last_time_fed++;
    p->last_time_pet++;
    p->last_time_played_with++;

    if (p->last_time_fed == p->food_change_time_ticks)
    {
        p->food -= utils_min(p->food, p->food_change_factor);
        p->last_time_fed = 0;
    }

    if (p->last_time_pet == p->alone_change_time_ticks)
    {
        p->alone += utils_min(PET_MAX_STAT_VALUE - p->alone, p->alone_change_factor);
        p->alone = utils_min(p->alone, PET_MAX_STAT_VALUE);
        p->last_time_pet = 0;
    }

    if (p->last_time_played_with == p->bored_change_time_ticks)
    {
        p->bored += utils_min(PET_MAX_STAT_VALUE - p->bored, p->bored_change_factor);
        p->last_time_played_with = 0;
    }
}

void Pet_Feed(Pet *p)
{
    p->food += utils_min(PET_MAX_STAT_VALUE - p->food, p->food_change_factor);
    p->last_time_fed = 0;
}

void Pet_Pet(Pet *p)
{
    p->alone -= utils_min(p->alone, p->alone_change_factor);
    p->last_time_pet = 0;
}

void Pet_Play(Pet *p)
{
    p->bored -= utils_min(p->bored, p->bored_change_factor);
    p->last_time_played_with = 0;
    p->currentActivity = ACTIVITY_IN_GAME;
}