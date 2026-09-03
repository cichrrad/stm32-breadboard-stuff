#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include <stdint.h>
#include <stdbool.h>

// UI
typedef struct 
{
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t val;
    bool fill_state;    
} UILoadBarType;

void ui_draw_loadbar(UILoadBarType *lbar);

// TEXT
typedef struct 
{
    uint8_t x;
    uint8_t y;
    bool fill_state;
    bool solid_bg;
    char* val;
} UITextType;

void ui_draw_string(UITextType *tbar, const char *str);


#endif 