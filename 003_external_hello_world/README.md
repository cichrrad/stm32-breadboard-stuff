# Notes

- This is almost the same as `001`, but improved and meant to blink external LED. Things to focus on:

    1. Uses explicit size types from `stdint.h`
    2. Since here the goal is to actually trigger external LED component, toggling is done with `GPIOA_BSSR` register, which is purpose made for atomic set/reset of bits. Were we to toggle using `GPIOA_ODR` via XOR, compiler will break it into multiple assembly instructions (read-modify-write). In any system doing something of use, this introduces risk of interrupt rewriting register for it to be replaced by stale version.

- To make this blink on an external LED, we just need:

    1. Breadboard
    2. 2x Jumper cables
    3. 1x LED (I used green, 2.1-2.5V, 20mA)
    4. 1x appropriate resistor (maybe optional, I am out of my depth here, Gemini told me) - I used 220 Ohms

- Because my nucleo maps ARDUINO D13 pin to the PA5 pin (internal LED), wiring is rather easy, even for me:

    1. Bridge any ARDUINO GND to breadboard GND (minus column).
    2. Bridge ARDUINO D13 to any breadboard row (a..j column).
    3. On the same row from the previous step, put in 1 end of the resistor. Put the other end to any OTHER breadboard row.
    4. On the same row from step 3 (where 1 of the resistor legs is), put in the longer LED leg. Put the shorter one in GND (minus column).

> If it does not work, try swapping LED ends. From what I understand, the length trick can be cap.

---

- [Photo of my setup](./IMG20260815133545.jpg)