#include "view_setup_helper.h"

#define COLOR_BACKGROUND GColorDarkGray
#define COLOR_SECONDARY GColorSunsetOrange
#define COLOR_TEXT GColorWhite
#define FONT_BASIC_TEXT FONT_KEY_GOTHIC_24



void menu_layer_set_up(MenuLayer* menu_layer) {

    #ifdef PBL_COLOR
        menu_layer_set_normal_colors(menu_layer, COLOR_BACKGROUND, COLOR_TEXT);
        menu_layer_set_highlight_colors(menu_layer, COLOR_SECONDARY, COLOR_TEXT);
    #endif
}


void window_set_up(Window* window) {
     #ifdef PBL_COLOR
        window_set_background_color(window, COLOR_BACKGROUND);
    #endif
}

void text_layer_set_up(TextLayer *text_layer) {
    #ifdef PBL_COLOR
        text_layer_set_text_color(text_layer, COLOR_TEXT);
    #endif
    text_layer_set_background_color(text_layer, GColorClear);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_BASIC_TEXT));
}
