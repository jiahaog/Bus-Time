#include "bus_stops_window.h"


static Window *s_bus_stops_window;
static MenuLayer *s_bus_stops_menu_layer;
static TextLayer *s_loading_text_layer; 

static uint16_t callback_menu_layer_get_num_rows(struct MenuLayer* menu_layer, uint16_t section_index, void *callback_context) {
    
    return numberOfBusStops();
}

// callback to draw all the rows
static void callback_menu_layer_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    uint16_t row_index = cell_index->row;
    char* title = bus_stop_list[row_index][0];
    char* subtitle = bus_stop_list[row_index][1];

    menu_cell_basic_draw(ctx, cell_layer, title, subtitle, NULL);

}

// Whatp happens when the select button is pushed
static void callback_menu_layer_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {

    
    // only act on the button click if the number of rows is more than zero
    if (numberOfBusStops() > 0) {
        int currentBusStopIndex = cell_index->row;
        send_app_message_int(KEY_BUS_SERVICE_LIST_START, currentBusStopIndex);
        services_window_push();
    }
}

static void menu_load() {
    Layer *window_layer = window_get_root_layer(s_bus_stops_window);
    s_bus_stops_menu_layer = menu_layer_create(layer_get_bounds(window_layer));

    menu_layer_set_callbacks(s_bus_stops_menu_layer, bus_stops_list, (MenuLayerCallbacks) {
        .get_num_rows = callback_menu_layer_get_num_rows,
        .draw_row = callback_menu_layer_draw_row,
        .select_click = callback_menu_layer_select_click
    });

    menu_layer_set_up(s_bus_stops_menu_layer);

    menu_layer_set_click_config_onto_window(s_bus_stops_menu_layer, s_bus_stops_window);

    layer_add_child(window_layer, menu_layer_get_layer(s_bus_stops_menu_layer));
}

static void window_load(Window *window) {
    window_set_up(window);
    Layer *window_layer = window_get_root_layer(window);   
    GRect bounds = layer_get_bounds(window_layer);

    // Create and Add to layer hierarchy:
    s_loading_text_layer = text_layer_create(bounds);
    text_layer_set_up(s_loading_text_layer);
    
    text_layer_set_text(s_loading_text_layer, "Loading...");
    layer_add_child(window_layer, text_layer_get_layer(s_loading_text_layer));
}

static void window_unload(Window *window) {
    menu_layer_destroy(s_bus_stops_menu_layer);
    window_destroy(window);
    s_bus_stops_menu_layer = NULL;
    s_bus_stops_window = NULL;
    
}

void bus_stops_window_push() {

    if (!s_bus_stops_window) {
        s_bus_stops_window = window_create();
        window_set_window_handlers(s_bus_stops_window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
        });
    }

    window_stack_push(s_bus_stops_window, true);
}


void bus_stops_window_reload_menu() {
    if (s_bus_stops_menu_layer) {
        menu_layer_reload_data(s_bus_stops_menu_layer);
    } else {
        menu_load();
    }
}


