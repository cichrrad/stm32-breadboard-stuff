# Notes

- This is (**ONCE AGAIN**) just a blinky. There's a lot of new stuff going on here, making it better than the previous implementations. It uses SysTick to activate LED via much more precise interrupt, than just burning cycles in for loop. To **NOT** burn the cycles as much, a CMSIS function `__WFI()` is used to sleep the chip to *Wait-For-Interrupt*.

---

## Changes in `linker.ld`

- Till now, a very simple (read as *AI generated*) linker script was used, which just set up FLASH and RAM, stack, and `.text` section.
- In `006`, a system tick counter is declared as global variable. To have global, we need section they go into (`.bss`). So to get it over with, a more complete linker script is used from now on, intializing and exposing `.data` and `.bss` section, as it should.

---

## Changes in `startup.c`

- Firstly, we added initialization for our newly added sections in `Reset_handler` function, before we hand the control over to main.
- Secondly, we need to wire up our interrupts so that we recognize SysTick interrupt, when it fires. This means adding first 15 entries to the `.irs_vector` (ignoring almost all the entries), because 15th entry is reserved for SysTick. We state handler for it (`SysTick_Handler`)

---

- In SysTick handler (in `main.c`), we simply increment our global tick counter. Because we initialized (in `SysTick_Init`) the timer to fire interrupt after `16000` cycles (after 1ms, because internal clock runs at 16MHz), increasing the variable. So instead of being busy inside a `while` loop in our `sleep_ms`, wasting ~`16 000 000` cycles, before 1 second passes, we waste only ~`1000` cycles!

[Intuitive grasp and explanation of PWM and timer mechanics](https://www.youtube.com/watch?v=AjN58ceQaF4&t=668s)