#include <pebble.h>

enum {
    KEY_BUS_SERVICE_LIST_START = 0,
    KEY_BUS_SERVICE_LIST_VALUE = 1,
    KEY_BUS_SERVICE_LIST_END = 2,
    KEY_BUS_SERVICE_DETAILS_START = 3,
    KEY_BUS_SERVICE_DETAILS_VALUE = 4,
    KEY_BUS_SERVICE_DETAILS_END = 5,
};

static char s_services_list[20][12];

static Window *s_services_menu_window;
static Window *s_service_detail_window;

static MenuLayer *s_services_menu_layer;
static TextLayer *s_service_detail_text_layer;

static int s_current_service; 
static int s_service_list_message_counter = 0; // for use with app message as a counter

// service list store

static void setUpServicesList() {
    strcpy(s_services_list[0], "Loading...");
}

static int numberOfServices() {
    int arraySize = sizeof(s_services_list) / sizeof(s_services_list[0]);
    int counter = 0;
    for (int index = 0; index < arraySize; index++) {
        char *currentElement = s_services_list[index];
        // APP_LOG(APP_LOG_LEVEL_DEBUG, "Service: |%s|", currentElement);
        if (strlen(currentElement) > 1) {
            counter++;
        }
    }

    return counter;
}

// s_services_menu_window callbacks

static uint16_t get_num_rows(struct MenuLayer* menu_layer, uint16_t section_index, void *callback_context) {
    return numberOfServices();
}

// callback to draw all the rows
static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    uint16_t row_index = cell_index->row;
    char* title = s_services_list[row_index];

    menu_cell_basic_draw(ctx, cell_layer, title, NULL, NULL);
}

// Whatp happens when the select button is pushed
static void select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {

    // push the service details window in
    char *currentService = s_services_list[s_current_service];
    window_stack_push(s_service_detail_window, true);
}

// AppMessage 

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message received!");

    // read first item    
    Tuple *t = dict_read_first(iterator);

    while(t != NULL) {
        switch (t->key) {
            case KEY_BUS_SERVICE_LIST_START:
                s_service_list_message_counter = 0;
                break;
            case KEY_BUS_SERVICE_LIST_VALUE:
                // assigns the string to the buffer
                snprintf(s_services_list[s_service_list_message_counter], sizeof(s_services_list[s_service_list_message_counter]), "%s", t->value->cstring);
                s_service_list_message_counter++;


                APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Service list: %s", s_services_list[s_service_list_message_counter]);
                break;
            case KEY_BUS_SERVICE_LIST_END:
                menu_layer_reload_data(s_services_menu_layer);
                break;
            default:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
                break;
        }

        t = dict_read_next(iterator);
    }

    // this concatenates strings
    // snprintf(verse_layer_buffer, sizeof(verse_layer_buffer), "%s", verse_content_buffer, verse_reference_buffer);
    // text_layer_set_text(text_layer, bus_message_buffer);

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

// window load

static void window_load_services_menu(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    s_services_menu_layer = menu_layer_create(layer_get_bounds(window_layer));

    menu_layer_set_callbacks(s_services_menu_layer, s_services_list, (MenuLayerCallbacks) {
        .get_num_rows = get_num_rows,
        .draw_row = draw_row,
        .select_click = select_click
    });

    menu_layer_set_click_config_onto_window(s_services_menu_layer, window);

    layer_add_child(window_layer, menu_layer_get_layer(s_services_menu_layer));
}

static void window_unload_services_menu(Window *window) {
    menu_layer_destroy(s_services_menu_layer);
}

static void window_load_service_details(Window *window) {
    Layer *window_layer = window_get_root_layer(window);


    GRect bounds = layer_get_bounds(window_layer);

    // Create and Add to layer hierarchy:
    s_service_detail_text_layer = text_layer_create(bounds);

    #ifdef PBL_COLOR
        text_layer_set_background_color(s_service_detail_text_layer, GColorMayGreen);
    #endif

    text_layer_set_text(s_service_detail_text_layer, "This is TextLayer");
    layer_add_child(window_layer, text_layer_get_layer(s_service_detail_text_layer));
}

static void window_unload_service_details(Window *window) {
    text_layer_destroy(s_service_detail_text_layer);
}

// init and deinit

static void init(void) {
    setUpServicesList();

    s_services_menu_window = window_create();
    window_set_window_handlers(s_services_menu_window, (WindowHandlers) {
        .load = window_load_services_menu,
        .unload = window_unload_services_menu,
    });
    

    s_service_detail_window = window_create();
    window_set_window_handlers(s_service_detail_window, (WindowHandlers) {
        .load = window_load_service_details,
        .unload = window_unload_service_details
    });

    // AppMessage
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

    window_stack_push(s_services_menu_window, true);
}

static void deinit(void) {
    window_destroy(s_services_menu_window);
    window_destroy(s_service_detail_window);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_services_menu_window);

    app_event_loop();
    deinit();
}
