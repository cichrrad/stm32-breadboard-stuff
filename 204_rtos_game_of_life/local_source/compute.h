#ifndef COMPUTE_H
#define COMPUTE_H

#include <stdint.h>
#include <stdbool.h>

#define GOL_WIDTH       120
#define GOL_HEIGHT      40
#define RNG_ALIVE_PROB  10

void rng_populate_grid(uint8_t grid[GOL_WIDTH][GOL_HEIGHT]);
void compute_next_gen(uint8_t old_gen[GOL_WIDTH][GOL_HEIGHT], uint8_t new_gen[GOL_WIDTH][GOL_HEIGHT]);

#endif