#include <pebble.h>

enum {
    KEY_BUS_SERVICE_LIST_START = 0,
    KEY_BUS_SERVICE_LIST_VALUE = 1,
    KEY_BUS_SERVICE_LIST_END = 2,

    KEY_BUS_SERVICE_DETAILS_START = 3,
    KEY_BUS_SERVICE_DETAILS_VALUE = 4,
    KEY_BUS_SERVICE_DETAILS_END = 5,

    KEY_BUS_STOP_LIST_START = 6,
    KEY_BUS_STOP_LIST_VALUE = 7,
    KEY_BUS_STOP_LIST_END = 8,

    KEY_CONNECTION_ERROR = 9
};

// [size of string][number of elements]
static char s_services_list[20][12];
static char s_bus_stops_list[37][30];

static Window *s_bus_stops_window;
static Window *s_services_menu_window;
static Window *s_service_detail_window;
static Window *s_error_screen_window;

static MenuLayer *s_services_menu_layer;
static MenuLayer *s_bus_stops_menu_layer;

static TextLayer *s_service_detail_text_layer;
static TextLayer *s_error_screen_text_layer;

static int s_bus_stop_list_message_counter = 0;
static int s_service_list_message_counter = 0; // for use with app message as a counter

// Sets up default variables for store
static void setUpStore() {
    // strcpy(s_bus_stops_list[0], "Loading...");
    // strcpy(s_services_list[0], "Loading...");
}

static int numberOfServices() {
    int arraySize = sizeof(s_services_list) / sizeof(s_services_list[0]);
    int counter = 0;
    for (int index = 0; index < arraySize; index++) {
        char *currentElement = s_services_list[index];
        if ((int)strlen(currentElement) > 0) {
            counter++;
        }
    }
    return counter;
}

static int numberOfBusStops() {
    int arraySize = sizeof(s_bus_stops_list) / sizeof(s_bus_stops_list[0]);
    int counter = 0;
    for (int index = 0; index < arraySize; index++) {
        char *currentElement = s_bus_stops_list[index];
        if ((int)strlen(currentElement) > 0) {
            counter++;
        }
    }
    return counter;
}

// helper method to parse string into an int and send it as an appmessage
static void sendAppMessageChar(int key, char *message) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    // parse the message to a int at the same time
    dict_write_uint8(iter, key, atoi(message));

    // Send the message!
    app_message_outbox_send();
}

// helper method to send an int as an appmessage
static void sendAppMessageInt(int key, int message) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    // parse the message to a int at the same time
    dict_write_uint8(iter, key, message);

    // Send the message!
    app_message_outbox_send();
}

// menu_layer_callbacks

// s_bus_stops_menu_layer callbacks;

static uint16_t callback_menu_layer_bus_stops_get_num_rows(struct MenuLayer* menu_layer, uint16_t section_index, void *callback_context) {
    
    return numberOfBusStops();
}

// callback to draw all the rows
static void callback_menu_layer_bus_stops_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    uint16_t row_index = cell_index->row;
    char* title = s_bus_stops_list[row_index];

    menu_cell_basic_draw(ctx, cell_layer, title, NULL, NULL);
}

// Whatp happens when the select button is pushed
static void callback_menu_layer_bus_stops_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bus stops menu select click");
    int currentBusStopIndex = cell_index->row;
    sendAppMessageInt(KEY_BUS_SERVICE_LIST_START, currentBusStopIndex);

    // push the service details window in
    window_stack_push(s_services_menu_window, true);
}

// s_services_menu_window callbacks


static uint16_t callback_menu_layer_services_get_num_rows(struct MenuLayer* menu_layer, uint16_t section_index, void *callback_context) {
    
    return numberOfServices();
}

// callback to draw all the rows
static void callback_menu_layer_services_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    uint16_t row_index = cell_index->row;
    char* title = s_services_list[row_index];

    menu_cell_basic_draw(ctx, cell_layer, title, NULL, NULL);
}

// Whatp happens when the select button is pushed
static void callback_menu_layer_services_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "menu layer services select click");
    // push the service details window in
    char *currentService = s_services_list[cell_index->row];

    sendAppMessageChar(KEY_BUS_SERVICE_DETAILS_START, currentService);
    window_stack_push(s_service_detail_window, true);
}

// AppMessage 

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {


    // read first item    
    Tuple *t = dict_read_first(iterator);

    while(t != NULL) {
        switch (t->key) {
            case KEY_BUS_STOP_LIST_START:
                s_bus_stop_list_message_counter = 0;
                break;
            case KEY_BUS_STOP_LIST_VALUE:
                snprintf(s_bus_stops_list[s_bus_stop_list_message_counter], sizeof(s_bus_stops_list[s_bus_stop_list_message_counter]), "%s", t->value->cstring);
                s_bus_stop_list_message_counter++;
                break;
            case KEY_BUS_STOP_LIST_END:
                // perform a check to see if the layer exists first before reloading
                if (s_bus_stops_menu_layer) {
                    menu_layer_reload_data(s_bus_stops_menu_layer);
                }
                break;

            case KEY_BUS_SERVICE_LIST_START:
                s_service_list_message_counter = 0;
                break;
            case KEY_BUS_SERVICE_LIST_VALUE:
                snprintf(s_services_list[s_service_list_message_counter], sizeof(s_services_list[s_service_list_message_counter]), "%s", t->value->cstring);
                s_service_list_message_counter++;
                break;
            case KEY_BUS_SERVICE_LIST_END:
                // perform a check to see if the layer exists first before reloading
                if (s_services_menu_layer) {
                    menu_layer_reload_data(s_services_menu_layer);
                }
                break;

            case KEY_BUS_SERVICE_DETAILS_VALUE:
                text_layer_set_text(s_service_detail_text_layer, t->value->cstring);
                break;

            case KEY_CONNECTION_ERROR:
                // #ifdef PBL_COLOR
                //     if (s_bus_stops_menu_layer) {
                //         menu_layer_set_normal_colors(s_bus_stops_menu_layer, GColorRed, GColorBlue);
                //         layer_mark_dirty(window_get_root_layer(s_bus_stops_window));
                //     }

                //     if (s_services_menu_layer) {
                //         menu_layer_set_normal_colors(s_services_menu_layer, GColorRed, GColorBlue);
                //         layer_mark_dirty(window_get_root_layer(s_services_menu_window));
                //     }
                // #endif

                window_stack_pop_all(false);
                window_stack_push(s_error_screen_window, true);

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

static void window_load_bus_stops_menu(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    s_bus_stops_menu_layer = menu_layer_create(layer_get_bounds(window_layer));

    menu_layer_set_callbacks(s_bus_stops_menu_layer, s_bus_stops_list, (MenuLayerCallbacks) {
        .get_num_rows = callback_menu_layer_bus_stops_get_num_rows,
        .draw_row = callback_menu_layer_bus_stops_draw_row,
        .select_click = callback_menu_layer_bus_stops_select_click
    });

    #ifdef PBL_COLOR
        menu_layer_set_normal_colors(s_bus_stops_menu_layer, GColorYellow, GColorBlue);
    #endif

    menu_layer_set_click_config_onto_window(s_bus_stops_menu_layer, s_bus_stops_window);

    layer_add_child(window_layer, menu_layer_get_layer(s_bus_stops_menu_layer));
}

static void window_unload_bus_stops_menu(Window *window) {
    menu_layer_destroy(s_bus_stops_menu_layer);
}


static void window_load_services_menu(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    s_services_menu_layer = menu_layer_create(layer_get_bounds(window_layer));

    menu_layer_set_callbacks(s_services_menu_layer, s_services_list, (MenuLayerCallbacks) {
        .get_num_rows = callback_menu_layer_services_get_num_rows,
        .draw_row = callback_menu_layer_services_draw_row,
        .select_click = callback_menu_layer_services_select_click
    });

    menu_layer_set_click_config_onto_window(s_services_menu_layer, s_services_menu_window);
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

    text_layer_set_font(s_service_detail_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text(s_service_detail_text_layer, "Loading...");
    layer_add_child(window_layer, text_layer_get_layer(s_service_detail_text_layer));
}

static void window_unload_service_details(Window *window) {
    text_layer_destroy(s_service_detail_text_layer);
}

static void window_load_error_screen(Window *window) {
    Layer *window_layer = window_get_root_layer(window);


    GRect bounds = layer_get_bounds(window_layer);

    // Create and Add to layer hierarchy:
    s_error_screen_text_layer = text_layer_create(bounds);


    text_layer_set_font(s_error_screen_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text(s_error_screen_text_layer, "Connection Error");
    layer_add_child(window_layer, text_layer_get_layer(s_error_screen_text_layer));
}

static void window_unload_error_screen(Window *window) {
    text_layer_destroy(s_error_screen_text_layer);
}

// init and deinit

static void init(void) {
    setUpStore();

    s_bus_stops_window = window_create();
    window_set_window_handlers(s_bus_stops_window, (WindowHandlers) {
        .load = window_load_bus_stops_menu,
        .unload = window_unload_bus_stops_menu,
    });
    
    


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

    s_error_screen_window = window_create();
    window_set_window_handlers(s_error_screen_window, (WindowHandlers) {
        .load = window_load_error_screen,
        .unload = window_unload_error_screen
    });

    // AppMessage
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

    window_stack_push(s_bus_stops_window, true);
}

static void deinit(void) {
    window_destroy(s_bus_stops_window);
    window_destroy(s_services_menu_window);
    window_destroy(s_service_detail_window);
}

int main(void) {
    init();

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_services_menu_window);

    app_event_loop();
    deinit();
}
