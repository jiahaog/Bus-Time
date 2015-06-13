#include "services_window.h"

#define SERVICES_CELL_H1_TOP_MARGIN 5
#define SERVICES_CELL_H2_TOP_MARGIN 10

#define SERVICES_CELL_X_PADDING 5

static Window *s_services_window;
static MenuLayer *s_services_menu_layer;
static TextLayer *s_loading_text_layer; 


static uint16_t callback_menu_layer_get_num_rows(struct MenuLayer* menu_layer, uint16_t section_index, void *callback_context) {
    // int numberOfRows = numberOfServices();
    // APP_LOG(APP_LOG_LEVEL_DEBUG, "NO OF ROWS : %i", numberOfRows);
    return get_service_list_count();
}

// callback to draw all the rows
static void callback_menu_layer_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    // this guard is needed to prevent crashing when switching back and forth quickly
    if (ctx && cell_layer && cell_index) {
        graphics_context_set_text_color(ctx, CELL_TEXT_COLOR);

        uint16_t row_index = cell_index->row;
        char* service_no = service_list[row_index][0];
        char* arrival_time = service_list[row_index][1];

        GRect cell_bounds = layer_get_bounds(cell_layer);

        int16_t cell_layer_width = cell_bounds.size.w - 2*SERVICES_CELL_X_PADDING;

        GRect title_bounds = GRect(cell_bounds.origin.x + SERVICES_CELL_X_PADDING, cell_bounds.origin.y + SERVICES_CELL_H1_TOP_MARGIN, cell_layer_width, cell_bounds.size.h);
        graphics_draw_text(ctx, service_no, fonts_get_system_font(CELL_H1_FONT), title_bounds, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
        
        GRect arrival_time_bounds = GRect(cell_bounds.origin.x, cell_bounds.origin.y + SERVICES_CELL_H2_TOP_MARGIN, cell_layer_width, cell_bounds.size.h);
        graphics_draw_text(ctx, arrival_time, fonts_get_system_font(CELL_H2_FONT), arrival_time_bounds, GTextOverflowModeFill, GTextAlignmentRight, NULL);

        // GRect subtitle_bounds = GRect(cell_bounds.origin.x + CELL_LEFT_MARGIN, title_bounds.origin.y + title_bounds.size.h, cell_bounds.size.w, s_cell_h2_height);
        
        // GRect stop_id_bounds = GRect(cell_bounds.origin.x + CELL_LEFT_MARGIN, subtitle_bounds.origin.y + subtitle_bounds.size.h, cell_bounds.size.w, s_cell_h2_height);
        // graphics_draw_text(ctx, stop_id, fonts_get_system_font(CELL_H2_FONT), stop_id_bounds, GTextOverflowModeFill, GTextAlignmentLeft, NULL);
    }



}

// Whatp happens when the select button is pushed
static void callback_menu_layer_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {

    // only act on the button click if the number of rows is more than zero
    if (get_service_list_count() > 0) {
        char *currentService = service_list[cell_index->row][0];
        send_app_message_string(KEY_BUS_SERVICE_DETAILS_START, currentService);
        details_window_push();
    }
    
}

static void menu_load() {

    Layer *window_layer = window_get_root_layer(s_services_window);
    s_services_menu_layer = menu_layer_create(layer_get_bounds(window_layer));

    menu_layer_set_callbacks(s_services_menu_layer, service_list, (MenuLayerCallbacks) {
        .get_num_rows = callback_menu_layer_get_num_rows,
        .draw_row = callback_menu_layer_draw_row,
        .select_click = callback_menu_layer_select_click
    });

    menu_layer_set_up(s_services_menu_layer);

    menu_layer_set_click_config_onto_window(s_services_menu_layer, s_services_window);

    layer_add_child(window_layer, menu_layer_get_layer(s_services_menu_layer));
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
    // reset the service list on unload so the wrong service list won't be displayed then changed immediately on load
    service_list_reset();

    menu_layer_destroy(s_services_menu_layer);
    window_destroy(window);
    s_services_window = NULL;
    s_services_menu_layer = NULL;
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

void services_window_reload_menu() {
    if (s_services_menu_layer) {
        menu_layer_reload_data(s_services_menu_layer);
    } else {
        menu_load();
    }
}

