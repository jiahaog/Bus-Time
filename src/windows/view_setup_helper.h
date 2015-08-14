#pragma once

#include <pebble.h>


#define COLOR_BACKGROUND GColorCobaltBlue
#define COLOR_SECONDARY GColorBulgarianRose
#define COLOR_TEXT GColorWhite

#define NO_COLOR_BACKGROUND GColorWhite
#define NO_COLOR_SECONDARY GColorClear
#define NO_COLOR_TEXT GColorBlack

#define FONT_BASIC_TEXT FONT_KEY_GOTHIC_24

#define CELL_H1_FONT FONT_KEY_GOTHIC_24_BOLD

#ifdef PBL_COLOR
    #define CELL_TEXT_COLOR GColorWhite
#else
    #define CELL_TEXT_COLOR GColorBlack
#endif

void menu_layer_set_up(MenuLayer* menu);
void window_set_up(Window* window);
void text_layer_set_up(TextLayer* text_layer);

#ifdef PBL_SDK_3
    void status_bar_layer_set_up(StatusBarLayer *status_bar_layer);
    GRect window_with_status_bar_content_bounds(Layer *window_layer, StatusBarLayer *status_bar_layer);
#endif

// need to offset for aplite because of different coordinate system
#ifdef PBL_PLATFORM_APLITE
    #define APLITE_Y_OFFSET -16 // equal to STATUS_BAR_LAYER_HEIGHT
#else
    #define APLITE_Y_OFFSET 0
#endif

int16_t get_font_height(Window* window, GFont font);
