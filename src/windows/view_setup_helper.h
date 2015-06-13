#pragma once

#include <pebble.h>



#define CELL_H1_FONT FONT_KEY_GOTHIC_24_BOLD
#define CELL_H2_FONT FONT_KEY_GOTHIC_18
#define CELL_TEXT_COLOR GColorWhite

void menu_layer_set_up(MenuLayer* menu);
void window_set_up(Window* window);
void text_layer_set_up(TextLayer* text_layer);

int16_t get_font_height(const Window* window, const char* font);