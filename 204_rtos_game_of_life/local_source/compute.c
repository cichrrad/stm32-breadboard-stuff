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

uint8_t count_neighbors(uint8_t grid[GOL_WIDTH][GOL_HEIGHT], int x, int y)
{
    uint8_t count = 0;

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            if (i == 0 && j == 0)
                continue; // Skip the cell itself

            // Wrap-around logic
            int nx = (x + i + GOL_WIDTH) % GOL_WIDTH;
            int ny = (y + j + GOL_HEIGHT) % GOL_HEIGHT;

            if (grid[nx][ny])
            {
                count++;
            }
        }
    }
    return count;
}

void compute_next_gen(uint8_t old_gen[GOL_WIDTH][GOL_HEIGHT], uint8_t new_gen[GOL_WIDTH][GOL_HEIGHT])
{
    for (int x = 0; x < GOL_WIDTH; x++)
    {
        for (int y = 0; y < GOL_HEIGHT; y++)
        {

            uint8_t neighbors = count_neighbors(old_gen, x, y);
            uint8_t is_alive = old_gen[x][y];

            // game rules
            if (is_alive && (neighbors == 2 || neighbors == 3))
            {
                new_gen[x][y] = 1; // Survival
            }
            else if (!is_alive && neighbors == 3)
            {
                new_gen[x][y] = 1; // Reproduction
            }
            else
            {
                new_gen[x][y] = 0; // Death (Under/Overpopulation)
            }
        }
    }
}