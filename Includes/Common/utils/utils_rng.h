#ifndef UTILS_RNG_H
#define UTILS_RNG_H

#include <stdbool.h>
#include <stdint.h>

void RNG_Init(void);
uint32_t utils_rand(void);

#endif