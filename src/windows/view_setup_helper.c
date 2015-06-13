#include "view_setup_helper.h"

#define COLOR_BACKGROUND GColorBlack
#define COLOR_SECONDARY GColorDarkCandyAppleRed
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
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
}

// helper method to get the height of a specific font
int16_t get_font_height(const Window* window, const char* font ) {
    
    // creates a text layer, gets the height, and destroys it
    TextLayer *text_layer = text_layer_create(layer_get_bounds(window_get_root_layer(window)));
    text_layer_set_font(text_layer, fonts_get_system_font(font));
    text_layer_set_text(text_layer, "Test");

    int16_t height = text_layer_get_content_size(text_layer).h;

    text_layer_destroy(text_layer);
    return height;
}