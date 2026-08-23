#include "pet.h"

void Pet_UpdateStats(Pet *p)
{

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


void Pet_Transition(Pet *p) {
    
    // starved to death 
    if(p->food == PET_MIN_STAT_VALUE){
        p->currentEmotion = EMOTION_RIP;
        p->alive = false;
        return;
    }

    // Famished
    if(p->food <= PET_MIN_STAT_VALUE + p->food_change_factor){
        p->food_status_severity = 3;
    }
    // Really Hungry
    else if (p->food <= PET_MIN_STAT_VALUE + 2*p->food_change_factor){
        p->food_status_severity = 2;
    }
    // Hungry
    else if (p->food <= PET_MIN_STAT_VALUE + 3*p->food_change_factor){
        p->food_status_severity = 1;
    }
    // OK
    else{
        p->food_status_severity=0;
    }

    // Bored to death
    if(p->bored >= PET_MAX_STAT_VALUE - p->bored_change_factor){
        p->bored_status_severity = 3;
    }
    // Really bored
    else if(p->bored >= PET_MAX_STAT_VALUE - 2*p->bored_change_factor){
        p->bored_status_severity = 2;
    }
    // Wants attention
    else if(p->bored >= PET_MAX_STAT_VALUE - 3*p->bored_change_factor){
        p->bored_status_severity = 1;
    }
    // OK
    else{
        p->bored_status_severity = 0;
    }

    // Devastated
    if(p->alone >= PET_MAX_STAT_VALUE - p->alone_change_factor){
        p->alone_status_severity = 3;
    }
    // Feeling abandoned
    else if(p->alone >= PET_MAX_STAT_VALUE - 2*p->alone_change_factor){
        p->alone_status_severity = 2;
    }
    // Feeling left out 
    else if(p->alone >= PET_MAX_STAT_VALUE - 3*p->alone_change_factor){
        p->alone_status_severity = 1;
    }
    // OK
    else{
        p->alone_status_severity = 0;
    }

    //================================

    //assigning emotions

    uint8_t severity_sum = p->alone_status_severity + p->food_status_severity + p->bored_status_severity;
    if( severity_sum >= 7){
        p->currentEmotion = EMOTION_HEARTBROKEN;
        return;
    }

    if(severity_sum == 0){
        p->currentEmotion = EMOTION_EXCITED;
        return;
    }

    if (severity_sum == 6){
        p->currentEmotion = EMOTION_SICK;
        return;
    }

    // Take the major one 
    if(severity_sum >= 3 && severity_sum <= 5){
        
        if(p->food_status_severity == 3){
            p->currentEmotion = EMOTION_HUNGRY;
            return;
            

        }

        if(p->alone_status_severity == 3){
            p->currentEmotion = EMOTION_SAD;
            return;

        }

        if(p->bored_status_severity == 3){
            p->currentEmotion = EMOTION_BORED;
            return;
        }

        // if nothing is major
        
        if(p->food_status_severity >1){
            p->currentEmotion = EMOTION_ANGRY;
            return;
        }

        p->currentEmotion = EMOTION_PINING;
        return;
    
    }

    p->currentEmotion = EMOTION_HAPPY;

};