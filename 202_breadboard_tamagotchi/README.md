# Breadboard tamagotchi

## Design doc (WIP)

Tamagotchi game running on the MCU, utilizing 128x64 0.96 OLED monochrome display with 128x16 top banner being yellow, rest being blue. Game will show character pixel art in the blue part of the screen, centered in viewport sized at 72x48 (to make ratio less crazy than 128x48) - this means it will have its top left corner at 28,16 and bottom right at 100,64 (top left of screen treated as 0,0). Top part will be reserved for GUI / information (since that is what the yellow part is designed for, anyway). The game will have time tracking using SysTick firing every 1ms. For player input, buttons with interrupt handlers will be present. Player will be able to "Pet", "Feed", and "Play with" using 3 buttons. "Play with" button will toggle a minigame, assigning buttons a different function (rock-paper-scissors, for instance). Tamagotchi will have internal state machine which will reflect its current well-being. to determine this, we will track some metrics, namely its `hunger`,`loneliness`, and `boredom`. Over time, these will change to reflect the state of the pet, and the state machine transitions will be tied to these metrics. Pet pixel art will also react to its state by making appropriate face etc

Aim is to make it so that the code is modular and we are able to extend it rather easily -- this means wirting separate and somewhat portable:

1. Tick System using SysTick.
2. Display Driver to allow scene rendering and paiting several layered elemenents, like top banner UI, pet art reflecting current state, and some effects like blinking its portrait if its hungry etc.. (will be fleshed out later, but display driver must be good enough to not stand in our way)
3. Input mapping / handling system to handle interrupts coming from specific buttons (adding the resource they should add etc) and based on situation -- we either introduce game state to differentiate if buttons now do the actions like "feed" or are in game mode for rock-paper-scissors, for instance, OR we tie it to the pet state `IN_GAME_RPC`. I'm still on the fence here, but I want something good for future adding of another game (maybe pet will choose game at random, for example RPC or Simon says)
4. State machine and transitions engine

--- 

## DESIGN DOG WRITTEN WITH GEMINI

### Breadboard Tamagotchi

#### Hardware & Display Layout

* **MCU:** STM32G491RE running bare-metal C (CMSIS for register access).
* **Display:** 128x64 0.96" OLED monochrome (SSD1306/SH1106) driven via SPI.


* **Screen Zones:**
* Top 128x16 yellow section is strictly reserved for the static GUI banner and dynamic metric bars.


* Bottom 128x48 blue section is for gameplay.




* **Viewport:** The pet sprite is constrained to a 72x48 virtual viewport, starting at coordinate `x:28, y:16`, preventing visual distortion.



#### Core Architecture

* **Tick Engine:** A 1ms non-blocking timekeeping system driven by `SysTick`. This global clock governs render loop pacing, metric degradation, and hardware debouncing.


* **Display Pipeline:** A modular driver managing a 1024-byte framebuffer in RAM. The render pass (`clear` -> `draw_gui` -> `fill_bars` -> `draw_sprite`) is completely decoupled from the hardware SPI transmission.


* **Draw Primitives:** The driver utilizes `dd_set_pixel` and `dd_draw_bitmap`, with an upcoming `dd_fill_rect` addition specifically for rendering dynamic progress bars.



#### Pet Mechanics & Inputs

* **Metrics:** Managed as `uint8_t` (0-100 scale). `FOOD` starts at 100 and degrades to 0. `BORED` and `ALONE` start at 0 and rise to 100.


* **State Machine:** A unified engine transitioning the pet between discrete states (e.g., `STATE_IDLE`, `STATE_HUNGRY`) based on metric thresholds. Transitions trigger sprite updates (like loading the `pika_yawn` asset).


* **Contextual Routing:** 3 physical buttons trigger EXTI interrupts. After software debouncing, the ISR calls function pointers defined in an active "Input Context" struct, allowing O(1) hot-swapping of button behaviors.



#### UI & Minigame Modules

* **Dynamic Bars:** The UI rendering pass lays down the static GUI banner, calculates fill percentages based on the core metrics, and draws dynamic rectangles directly over the static empty bar outlines.


* **Pluggable Minigames:** When entering a state like `IN_GAME_RPC`, the core loop delegates framebuffer control and reassigns the input function pointers to an isolated minigame module until the game concludes.
