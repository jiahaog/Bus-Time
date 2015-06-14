#include "error_window.h"

static Window *s_error_window;
static TextLayer *s_error_text_layer;

char s_error_message[25] = "DEFAULT ERROR MESSAGE";

static void window_load(Window *window) {
    window_set_up(window);
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Create and Add to layer hierarchy:
    s_error_text_layer = text_layer_create(bounds);
    text_layer_set_up(s_error_text_layer);

    text_layer_set_text(s_error_text_layer, s_error_message);

    #ifdef PBL_COLOR
        window_set_background_color(window, GColorRed);
    #endif

    layer_add_child(window_layer, text_layer_get_layer(s_error_text_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(s_error_text_layer);
    window_destroy(window);

    s_error_window = NULL;
}

void error_window_push(char *message) {

    snprintf(s_error_message, sizeof(s_error_message), "%s", message);
    
    if (!s_error_window) {
        s_error_window = window_create();
        window_set_window_handlers(s_error_window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
        });
    }

    
    window_stack_push(s_error_window, true);

}
