#include "services_window.h"


static Window *s_services_window;
static MenuLayer *s_services_menu_layer;


static uint16_t callback_menu_layer_get_num_rows(struct MenuLayer* menu_layer, uint16_t section_index, void *callback_context) {
    // int numberOfRows = numberOfServices();
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "NO OF ROWS : %i", numberOfRows);
    return numberOfServices();
}

// callback to draw all the rows
static void callback_menu_layer_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    uint16_t row_index = cell_index->row;
    char* title = services_list[row_index];

    menu_cell_basic_draw(ctx, cell_layer, title, NULL, NULL);
}

// Whatp happens when the select button is pushed
static void callback_menu_layer_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {

    // only act on the button click if the number of rows is more than zero
    if (numberOfServices() > 0) {
        char *currentService = services_list[cell_index->row];
        send_app_message_string(KEY_BUS_SERVICE_DETAILS_START, currentService);
        details_window_push();
    }
    
}

static void window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    s_services_menu_layer = menu_layer_create(layer_get_bounds(window_layer));

    menu_layer_set_callbacks(s_services_menu_layer, services_list, (MenuLayerCallbacks) {
        .get_num_rows = callback_menu_layer_get_num_rows,
        .draw_row = callback_menu_layer_draw_row,
        .select_click = callback_menu_layer_select_click
    });

    menu_layer_set_click_config_onto_window(s_services_menu_layer, s_services_window);

    layer_add_child(window_layer, menu_layer_get_layer(s_services_menu_layer));
}

static void window_unload(Window *window) {
    menu_layer_destroy(s_services_menu_layer);
    window_destroy(window);
    s_services_window = NULL;
}

void services_window_push() {

    if (!s_services_window) {
        s_services_window = window_create();
        window_set_window_handlers(s_services_window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
        });
    }

    window_stack_push(s_services_window, true);
}

void services_window_reload() {
    if (s_services_menu_layer) {
        menu_layer_reload_data(s_services_menu_layer);
    }
}

