#include "details_window.h"

#define LOADING_MESSAGE "Loading..."

static Window *s_details_window;
static TextLayer *s_details_text_layer;
static ActionBarLayer *s_action_bar;

char s_details_message[100] = LOADING_MESSAGE;

GBitmap *s_bitmap_set_alert;

void action_bar_click_config_provider(void *context) {
    // TODO: send app message for notification
}

void action_bar_load() {
    s_action_bar = action_bar_layer_create();
    action_bar_layer_add_to_window(s_action_bar, s_details_window);
    action_bar_layer_set_click_config_provider(s_action_bar, action_bar_click_config_provider);

    s_bitmap_set_alert = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ALERT_SET);

    #ifdef PBL_PLATFORM_APLITE
        action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_bitmap_set_alert);
    #else
        action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_SELECT, s_bitmap_set_alert, true);
    #endif
}

static void window_load(Window *window) {
    window_set_up(window);
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

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

    action_bar_layer_destroy(s_action_bar);
    gbitmap_destroy(s_bitmap_set_alert);
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

    if (strcmp(s_details_message,LOADING_MESSAGE) != 0 ) {
        action_bar_load();
    }

}
