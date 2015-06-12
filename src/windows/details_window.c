#include "details_window.h"


static Window *s_details_window;
static TextLayer *s_details_text_layer;

char s_details_message[100] = "Loading...";


static void window_load(Window *window) {
    window_set_up(window);
    Layer *window_layer = window_get_root_layer(window);


    GRect bounds = layer_get_bounds(window_layer);

    // Create and Add to layer hierarchy:
    s_details_text_layer = text_layer_create(bounds);
    text_layer_set_up(s_details_text_layer);

    text_layer_set_text(s_details_text_layer, s_details_message);
    layer_add_child(window_layer, text_layer_get_layer(s_details_text_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(s_details_text_layer);
    window_destroy(window);
    snprintf(s_details_message, sizeof(s_details_message), "Loading...");
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
    snprintf(s_details_message, sizeof(s_details_message), "%s", message);
    layer_mark_dirty(text_layer_get_layer(s_details_text_layer));
}
