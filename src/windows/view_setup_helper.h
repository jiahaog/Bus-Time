#pragma once

#include <pebble.h>

void menu_layer_set_up(MenuLayer* menu);
void window_set_up(Window* window);
void text_layer_set_up(TextLayer* text_layer);

int16_t get_font_height(const Window* window, const char* font);