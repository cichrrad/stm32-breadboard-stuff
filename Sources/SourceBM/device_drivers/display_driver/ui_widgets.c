#include "ui_widgets.h"
#include "ddriver.h"
#include "fonts.h"

void ui_draw_loadbar(UILoadBarType *lbar)
{
    // draw frame
    dd_draw_rect(lbar->x, lbar->y, lbar->width, lbar->height, lbar->fill_state);

    if (lbar->val == 0)
    {
        return;
    }

    int filled_width = ((uint16_t)lbar->val * (lbar->width - 1)) / 255;
    
    // clamp just in case val somehow exceeds 255
    if (filled_width > (lbar->width - 1))
    {
        filled_width = (lbar->width - 1);
    }

    if (filled_width > 0)
    {
        dd_fill_rect(lbar->x + 1, lbar->y + 1, filled_width, lbar->height-1, lbar->fill_state);
    }
}

void __ui_write_letter(int x, int y, char c, bool state, bool force_bg_clear)
{

    if (c < LOCHAR || c > HICHAR)
    {
        c = '?'; // Fallback for unsupported characters
    }

    // starting index of the character in the array
    int font_idx = c - LOCHAR;

    // Loop through the height of the character
    for (int row = 0; row < FONT_HEIGHT; row++)
    {
        char row_data = FONT[font_idx][row];

        // Loop through the width of the character
        // NOTE: Defining width for each char to not
        // have to do spaced out mono font would be better
        for (int col = 0; col < 8; col++)
        {
            bool draw_pixel = (row_data >> col) & 0x01;

            if (draw_pixel)
            {
                dd_set_pixel(x + col, y + row, state);
            }
            else
            {
                // toggle bg for contrast
                if (force_bg_clear)
                {
                    dd_set_pixel(x + col, y + row, !state);
                }
            }
        }
    }
}

void ui_draw_string(UITextType *tbar, const char *str)
{
    int curr_x = tbar->x;
    while (*str) // loop till \0
    {
        __ui_write_letter(curr_x, tbar->y, *str, tbar->fill_state, tbar->solid_bg);
        // Advance X by 8 pixels for the next character
        curr_x += (FONT_BWIDTH * 8);
        str++;
    }
}
