#ifndef DDRIVER_H
#define DDDRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define DD_WIDTH 128
#define DD_HEIGHT 64
#define DD_PIXELS_PER_BYTE 8

#define DD_FB_SIZE ((DD_WIDTH*DD_HEIGHT) / DD_PIXELS_PER_BYTE) 

extern uint8_t DD_FRAMEBUFFER[DD_FB_SIZE];

void dd_draw_bitmap(int x, int y, int width, int height, const uint8_t *bitmap, bool use_msb);
#endif 