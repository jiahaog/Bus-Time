#include <pebble.h>

enum {
    KEY_BUS_SERVICE_LIST = 0,
    KEY_BUS_SERVICE_DETAILS = 1
};

static Window *window;
static TextLayer *text_layer;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    // text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    // text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    // text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


// AppMessage api

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

    static char bus_message_buffer[18];

    // read first item    
    Tuple *t = dict_read_first(iterator);

    while(t != NULL) {
        switch (t->key) {
            case KEY_BUS_SERVICE_LIST:
                // assigns the string to the buffer
                snprintf(bus_message_buffer, sizeof(bus_message_buffer), "%s", t->value->cstring);
                break;
            default:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
                break;
        }

        t = dict_read_next(iterator);
    }

    // this concatenates strings
    // snprintf(verse_layer_buffer, sizeof(verse_layer_buffer), "%s", verse_content_buffer, verse_reference_buffer);
    text_layer_set_text(text_layer, bus_message_buffer);

}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 50));
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text(text_layer, "Loading...");

    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    
    #ifdef PBL_COLOR
        text_layer_set_background_color(text_layer, GColorOrange);
    #else
        text_layer_set_background_color(text_layer, GColorWhite);
    #endif

    layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(text_layer);
}

static void init(void) {
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);

    // AppMessage
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);

    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {
    window_destroy(window);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

    app_event_loop();
    deinit();
}
