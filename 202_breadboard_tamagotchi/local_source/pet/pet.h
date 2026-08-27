#ifndef PET_H
#define PET_H

#include <stdint.h>
#include <stdbool.h>
#include "device_drivers/tick_engine/systick_timer.h"

// must be divisible by width of the UI bars (25)
#define PET_MAX_STAT_VALUE 250
#define PET_MIN_STAT_VALUE 0


typedef enum {
    EMOTION_EXCITED=0,
    EMOTION_HAPPY,
    EMOTION_HEARTBROKEN,
    EMOTION_HUNGRY,
    EMOTION_LONELY,
    EMOTION_SICK,
    EMOTION_ANGRY,
    EMOTION_BORED,
    EMOTION_PINING,
    EMOTION_RIP,
    EMOTION_SAD,
    EMOTIONS_COUNT
} PetEmotion;

typedef enum{
    ACTIVITY_IDLE = 0,
    ACTIVITY_IN_GAME,
    ACTIVITIES_COUNT
} PetActivity;


typedef struct {
    uint8_t food;
    uint8_t bored;
    uint8_t alone;

    int8_t food_status_severity;
    int8_t bored_status_severity;
    int8_t alone_status_severity;

    uint8_t food_change_factor;
    uint8_t bored_change_factor;
    uint8_t alone_change_factor;
    
    uint32_t food_change_time_ms;
    uint32_t bored_change_time_ms;
    uint32_t alone_change_time_ms;

    uint32_t last_time_fed;
    uint32_t last_time_played_with;
    uint32_t last_time_pet;

    PetActivity currentActivity;

    const uint8_t* const* emotion_array;
    PetEmotion currentEmotion;

    bool alive;
} Pet;

void Pet_UpdateStats(Pet* p);
void Pet_Eat(Pet* p);
void Pet_Play(Pet* p);
void Pet_Pet(Pet* p);

void Pet_Transition(Pet* p);


#endif