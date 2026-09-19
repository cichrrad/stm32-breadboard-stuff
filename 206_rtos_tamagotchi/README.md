# Tamagotchi using FreeRTOS, with mini game runner for one of the buttons

This project is adaptation and extension of `202`. It uses FreeRTOS task to run the same tamagotchi-esque game as `202`, BUT it extends it with a mini game runner, which runs a minigame every time you choose to lower boredom of your pet. This was rather complicated to do in a way which allows extension to add any number of games -- specifically, there was need to add input abstraction in order to be able to swap out button assigned functions on the go AND mini-game itself is abstracted and operated via `MiniGameRunner` struct. It can hold multiple mini games, provided they match specific API and provide:

* Init function `void initFn(void)`
* Render function `void renderFn(void)`
* Update function `void updateFn(bool*)`. This takes bool so that `MiniGameRunner` can pass in a boolean variable to find out when it should end the game -- you would set this to true the moment you are supposed to back out into the normal tamagotchi loop. This is horrid, you don't have to tell me.
* Input mapping struct for the minigame `im`, which defines inputs and assigned actions (functions). In order to make this generic, function signature is casted to generic `void(*)(void)`. Should it require different signature for handling button press (such as passing object in), a wrapper must be used -- reason for this is because I am a moron and this is not ideal, but such is life.

Currently, there is only 1 minigame, that being Rock-Paper-Scissors (very original). It does not matter who wins, point is you had fun and your pet lowered their boredom stat :).

TODO -- will be written out in detail