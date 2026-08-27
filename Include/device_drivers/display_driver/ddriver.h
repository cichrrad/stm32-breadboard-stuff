#ifndef DDRIVER_H
#define DDDRIVER_H

#include <stdint.h>
#include <stdbool.h>

#define DD_WIDTH 128
#define DD_HEIGHT 64
#define DD_PIXELS_PER_BYTE 8

#define DD_FB_SIZE ((DD_WIDTH*DD_HEIGHT) / DD_PIXELS_PER_BYTE) 

void DD_Init();

void dd_draw_bitmap(int x, int y, int width, int height, const uint8_t *bitmap, bool use_msb);

void dd_draw_line(int ax, int ay, int bx, int by, bool state);

void dd_fill_rect(int x, int y, int width, int height, bool state);
void dd_fill_circle(int x,int y, int radius, bool state);

void dd_draw_rect(int x, int y, int width, int height, bool state);
void dd_draw_circle(int x,int y, int radius, bool state);

void dd_draw_triangle(int ax, int ay, int bx, int by, int cx, int xy, bool state);
void dd_fill_triangle(int ax, int ay, int bx, int by, int cx, int xy, bool state);

void dd_write_letter(int x, int y, char c, bool state, bool force_bg_clear);
void dd_write_string(int x, int y, const char *str, bool state, bool force_bg_clear);

void dd_update();

typedef struct 
{
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t val;
    uint8_t steps;
    bool active_equals_on;    
} DDLoadBarType;

void dd_loadbar_set_val(DDLoadBarType* lbar);

#endif 