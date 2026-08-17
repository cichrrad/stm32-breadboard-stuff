# Notes

- This is once again another iteration of the blinky, but (once again) step closer to clean solution.
- This blinky uses functions defined in `../Include/device_drivers/gpio.h` and `../Source/device_drivers/gpio.c` to showcase another level of abstraction leading to agnostic functions able to init and toggle **any** GPIO ports and pins via same shared interface. It is rather clear why this is an improvement, as with combination with the CMSIS headers, it improves readability and abstracts boilerplate for setting up things. This is one step closer to the standard HAL.
- Also note the C macros for resetting -- Eventually, Using static inline instead is probably better
- Also we should check for input, because our inputs only use first 1 or 2 bits, not all 8 (comment in `gpio.c`, line ~20)