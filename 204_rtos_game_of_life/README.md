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

TODO - Write out
