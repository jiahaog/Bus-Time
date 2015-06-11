#include "details_window.h"


static Window *s_details_window;
static TextLayer *s_details_text_layer;

static void window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);


    GRect bounds = layer_get_bounds(window_layer);

    // Create and Add to layer hierarchy:
    s_details_text_layer = text_layer_create(bounds);

    #ifdef PBL_COLOR
        text_layer_set_background_color(s_details_text_layer, GColorMayGreen);
    #endif

    text_layer_set_font(s_details_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text(s_details_text_layer, "Loading...");
    layer_add_child(window_layer, text_layer_get_layer(s_details_text_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(s_details_text_layer);
    window_destroy(window);
    s_details_window = NULL;
}

void details_window_push() {

    if (!s_details_window) {
        s_details_window = window_create();
        window_set_window_handlers(s_details_window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
        });
    }

    window_stack_push(s_details_window, true);

}

void details_window_set_text(char *message) {
    text_layer_set_text(s_details_text_layer, message);
}
