# FreeRTOS Game of life renderer

This program uses FreeRTOS to run Conway's Game of Life and display it on an OLED monochrome display along with hearthbeat indication and FPS counter. Program has 2 tasks, working akin to Producer-Consumer:

1. `vRenderTask` -- Renders UI and viewport with the current generation. Blocks until generation is ready to be rendered, at which points it signals the `vComputeTask` to calculate next generation, then it gets started on rendering. If last generation rendering is still on-going, it blocks until DMA moving the bytes to SPI TX registers notifies it.

2. `vComputeTask` -- Initializes random grid for generation 0, then it calculates generation 1 based on gen. 0 and standard rules for G.O.L. (see `local_source/compute.c/.h`). After generation is computed, signals `vRenderTask` that new frame is ready to be drawn. Right after that, it blocks until it receives signal from `vRenderTask` to get on to calculating upcoming generation.

> Note: with `-O3` it currently hits 60+FPS. Biggest boost was said compilation flag AND unrolling the check for neigbours, so edge-cells are processed separately from inner cells, for whom math is much easier.

---

## DEMO

![demo](./assets/demo.jpg)

---

## Overview & Architecture

Program works in a producer-consumer fasion, where `vComputeTask` is supplying new generations for the `vRenderTask` to display on the OLED display. Similar to double-buffering used in the display driver (see `display_driver` directories in `SourceRTOS` and `IncludeRTOS` dirs), we also use 2 arrays for computing -- one for generation `i` and another for `i+1`, because we need previous generation to produce the next one. This also allows us to work on `i+1`th generation while `vRenderTask` renders `i`th generation, as both tasks will simply be reading `i`th gen without modification.

In `vComputeTask` we initialize the game with hardware supported RNG generation (see `Common/utils/utils_rng.c|.h`). Note that this does have spin loops, for instance when waiting for clock to stablizie before supplying it to the RNG hardware. But it is done once in the init part of the task, so I suppose its good enough. The main loop of the task calculates `i+1`th gen, swaps pointers so `i+2`th will override array `i`th generation is stored in, and then it uses `        xSemaphoreGive(xFrameReadySemaphore)` to signal `vRenderTask` new frame (`i+1`th) is ready to be rendered in `current_grid` array. After that it blocks on `ulTaskNotifyTake(pdTRUE, portMAX_DELAY)` until `vRenderTask` confirms it is starting to render `i+1`th frame. At that point, we know for sure we can start overriding `i`th generation, because `vRenderTask` will be reading from `current_grid` with `i+1`th frame, which `vComputeTask` will also be reading from to compute `i+2`th generation frame. And so the process repeats

Looking at `vRenderTask` POV, it has some setting up for the display driver and UI first, Then it asks for first frame and enters the loop. This loop is different from `vComputeTask` mainly in the fact it actually has delay set up and is not looping as fast as it can during task time. This is, of course, to hit target fps, which is `~15`. While `60+FPS` is possible, good luck seeing anything at that speed. First thing we do in the main loop though is block on semaphore and wait for new frame to render, like the consumer we are. The moment we recieve it, we fire back with `xTaskNotifyGive(xComputeTaskHandle)` to allow `vComputeTask` to start on producing another frame to come. After that, `vRenderTask` simply creates the frame and fires `dd_update()` to start DMA worker on piping bytes over SPI to the display.