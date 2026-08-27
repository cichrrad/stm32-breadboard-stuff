#include "int_sqrt.h"

uint32_t int_sqrt(uint32_t num)
{
    uint32_t res = 0;
    uint32_t bit = 1 << 30; // The second-to-top bit (for 32-bit unsigned ints)

    // "bit" starts at the highest power of four less than or equal to the argument
    while (bit > num)
    {
        bit >>= 2;
    }

    while (bit != 0)
    {
        if (num >= res + bit)
        {
            num -= res + bit;
            res = (res >> 1) + bit;
        }
        else
        {
            res >>= 1;
        }
        bit >>= 2;
    }
    return res;
}
