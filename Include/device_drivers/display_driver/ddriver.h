#ifndef DDRIVER_H
#define DDDRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define DD_WIDTH 128
#define DD_HEIGHT 64
#define DD_PIXELS_PER_BYTE 8

#define DD_FB_SIZE ((DD_WIDTH*DD_HEIGHT) / DD_PIXELS_PER_BYTE) 

void DD_Init();

void dd_set_pixel(int x, int y, bool state);

void dd_draw_bitmap(int x, int y, int width, int height, const uint8_t *bitmap, bool use_msb);

void dd_draw_line(int ax, int ay, int bx, int by, bool state);

void dd_fill_rect(int x, int y, int width, int height, bool state);
void dd_fill_circle(int x,int y, int radius, bool state);

void dd_draw_rect(int x, int y, int width, int height, bool state);
void dd_draw_circle(int x,int y, int radius, bool state);

void dd_draw_triangle(int ax, int ay, int bx, int by, int cx, int xy, bool state);
void dd_fill_triangle(int ax, int ay, int bx, int by, int cx, int xy, bool state);

void dd_update();
void dd_clear();

#endif 