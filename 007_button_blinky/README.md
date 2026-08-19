# Notes

- This mini program toggles on-board LED in response to external button press. I specifically wanted to make this with external button and after trying interrupts with SysTick, because external interrupts require some additional setup to wire them up, as opposed to the interrupt caused by SysTick, which is internal.

- LED is toggled in response to `EXTI0` line (external interrupt line 0). It is wired to GPIOB pin 0, which has mode set for input and is configured as pull-up (see `Button_EXTI_Init`). Interrupt handler for the line is added to the vector in `startup.c`, and handling is in `EXTI0_IRQHandler` function.

- Because I use `__WFI` function to not poll like crazy, I found out (the hard way) that its also really good to **enable debug even while sleeping** (`DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;`). If this is not the case, you are forced to hold reset button as you are flashing new program onto the board.

---

- ![Photo of my setup](IMG20260817180457.jpg)