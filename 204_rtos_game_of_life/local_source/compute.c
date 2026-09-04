#include "compute.h"
#include "utils_rng.h"

void rng_populate_grid(uint8_t grid[GOL_WIDTH][GOL_HEIGHT])
{
    // ~ srand
    RNG_Init();

    for (int x = 0; x < GOL_WIDTH; x++)
    {
        for (int y = 0; y < GOL_HEIGHT; y++)
        {
            if ((utils_rand() % 100) < RNG_ALIVE_PROB)
            {
                grid[x][y] = 1;
            }
            else
            {
                grid[x][y] = 0;
            }
        }
    }
}

static inline void apply_rules(uint8_t is_alive, uint8_t neighbors, uint8_t *next_state)
{
    if (is_alive && (neighbors == 2 || neighbors == 3))
    {
        *next_state = 1; // Survival
    }
    else if (!is_alive && neighbors == 3)
    {
        *next_state = 1; // Reproduction
    }
    else
    {
        *next_state = 0; // Death
    }
}

static inline uint8_t count_neighbors_edge(uint8_t grid[GOL_WIDTH][GOL_HEIGHT], int x, int y)
{
    uint8_t count = 0;
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0)
                continue; // Skip the cell itself

            int nx = x + i;
            int ny = y + j;

            // Fast wrap-around (NO MODULO)
            if (nx < 0)
                nx = GOL_WIDTH - 1;
            else if (nx >= GOL_WIDTH)
                nx = 0;

            if (ny < 0)
                ny = GOL_HEIGHT - 1;
            else if (ny >= GOL_HEIGHT)
                ny = 0;

            count += grid[nx][ny];
        }
    }
    return count;
}

void compute_next_gen(uint8_t old_gen[GOL_WIDTH][GOL_HEIGHT], uint8_t new_gen[GOL_WIDTH][GOL_HEIGHT])
{
    // Non-edge cells
    // separated from edge cells for performance
    for (int x = 1; x < GOL_WIDTH - 1; x++)
    {
        for (int y = 1; y < GOL_HEIGHT - 1; y++)
        {
            uint8_t neighbors =
                old_gen[x - 1][y - 1] + old_gen[x][y - 1] + old_gen[x + 1][y - 1] +
                old_gen[x - 1][y] + old_gen[x + 1][y] +
                old_gen[x - 1][y + 1] + old_gen[x][y + 1] + old_gen[x + 1][y + 1];

            apply_rules(old_gen[x][y], neighbors, &new_gen[x][y]);
        }
    }

    // Top/bottom row (with corners)
    for (int x = 0; x < GOL_WIDTH; x++)
    {
        apply_rules(old_gen[x][0], count_neighbors_edge(old_gen, x, 0), &new_gen[x][0]);
        apply_rules(old_gen[x][GOL_HEIGHT - 1], count_neighbors_edge(old_gen, x, GOL_HEIGHT - 1), &new_gen[x][GOL_HEIGHT - 1]);
    }

    // Left/right col (no corners)
    for (int y = 1; y < GOL_HEIGHT - 1; y++)
    {
        apply_rules(old_gen[0][y], count_neighbors_edge(old_gen, 0, y), &new_gen[0][y]);
        apply_rules(old_gen[GOL_WIDTH - 1][y], count_neighbors_edge(old_gen, GOL_WIDTH - 1, y), &new_gen[GOL_WIDTH - 1][y]);
    }
}