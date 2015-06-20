#include "bus_stops_window.h"

#define BUS_STOP_CELL_H1_TOP_MARGIN -5
#define BUS_STOP_CELL_H2_FONT FONT_KEY_GOTHIC_18
#define BUS_STOP_CELL_HEIGHT 60

#define BUS_STOP_CELL_X_PADDING 5


static Window *s_bus_stops_window;
static MenuLayer *s_bus_stops_menu_layer;
static TextLayer *s_loading_text_layer; 

static int16_t s_cell_h1_height = 0;
static int16_t s_cell_h2_height = 0;

#ifdef PBL_PLATFORM_BASALT
    static StatusBarLayer *s_status_bar_layer;
#endif

static uint16_t callback_menu_layer_get_num_rows(struct MenuLayer* menu_layer, uint16_t section_index, void *callback_context) {
    
    return get_bus_stop_list_count();
}

static int16_t callback_menu_layer_get_cell_height(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {

    return BUS_STOP_CELL_HEIGHT;
}

// callback to draw all the rows
static void callback_menu_layer_draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {

    // this guard is needed to prevent crashing when switching back and forth quickly
    if (ctx && cell_layer && cell_index) {
        graphics_context_set_text_color(ctx, CELL_TEXT_COLOR);

        uint16_t row_index = cell_index->row;
        char* title = bus_stop_list[row_index][0];
        char* subtitle = bus_stop_list[row_index][1];
        char* stop_id = bus_stop_list[row_index][2];

        GRect cell_bounds = layer_get_bounds(cell_layer);

        int16_t cell_layer_width = cell_bounds.size.w - 2*BUS_STOP_CELL_X_PADDING;

        GRect title_bounds = GRect(cell_bounds.origin.x + BUS_STOP_CELL_X_PADDING, cell_bounds.origin.y + BUS_STOP_CELL_H1_TOP_MARGIN, cell_layer_width, s_cell_h1_height);
        graphics_draw_text(ctx, title, fonts_get_system_font(CELL_H1_FONT), title_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        
        GRect subtitle_bounds = GRect(cell_bounds.origin.x + BUS_STOP_CELL_X_PADDING, title_bounds.origin.y + title_bounds.size.h, cell_layer_width, s_cell_h2_height);
        graphics_draw_text(ctx, subtitle, fonts_get_system_font(BUS_STOP_CELL_H2_FONT), subtitle_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        
        GRect stop_id_bounds = GRect(cell_bounds.origin.x + BUS_STOP_CELL_X_PADDING, subtitle_bounds.origin.y + subtitle_bounds.size.h, cell_layer_width, s_cell_h2_height);
        graphics_draw_text(ctx, stop_id, fonts_get_system_font(BUS_STOP_CELL_H2_FONT), stop_id_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
}

// Whatp happens when the select button is pushed
static void callback_menu_layer_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {

    // only act on the button click if the number of rows is more than zero
    if (get_bus_stop_list_count() > 0) {
        uint16_t row_index = cell_index->row;
        char* bus_stop_name = bus_stop_list[row_index][0];
        char* bus_stop_id = bus_stop_list[row_index][2];
        services_window_push(bus_stop_name);
        send_app_message_string(KEY_BUS_SERVICE_LIST_START, bus_stop_id);
    }
}

static void menu_load() {
    s_cell_h1_height = get_font_height(s_bus_stops_window, fonts_get_system_font(CELL_H1_FONT));
    s_cell_h2_height = get_font_height(s_bus_stops_window, fonts_get_system_font(BUS_STOP_CELL_H2_FONT));

    Layer *window_layer = window_get_root_layer(s_bus_stops_window);

    GRect window_bounds = layer_get_bounds(window_layer);

    #ifdef PBL_PLATFORM_APLITE
        GRect content_bounds = window_bounds;
    #else
        GRect content_bounds = window_with_status_bar_content_bounds(window_layer, s_status_bar_layer);
    #endif

    s_bus_stops_menu_layer = menu_layer_create(content_bounds);  
    menu_layer_set_callbacks(s_bus_stops_menu_layer, bus_stop_list, (MenuLayerCallbacks) {
        .get_num_rows = callback_menu_layer_get_num_rows,
        .draw_row = callback_menu_layer_draw_row,
        .select_click = callback_menu_layer_select_click,
        .get_cell_height = callback_menu_layer_get_cell_height
    });

    menu_layer_set_up(s_bus_stops_menu_layer);

    menu_layer_set_click_config_onto_window(s_bus_stops_menu_layer, s_bus_stops_window);

    layer_add_child(window_layer, menu_layer_get_layer(s_bus_stops_menu_layer));
}

static void window_load(Window *window) {
    window_set_up(window);
    Layer *window_layer = window_get_root_layer(window);   
    GRect bounds = layer_get_bounds(window_layer);

    // // Create and Add to layer hierarchy:
    #ifdef PBL_PLATFORM_APLITE
        s_loading_text_layer = text_layer_create(bounds);
        text_layer_set_up(s_loading_text_layer);
        text_layer_set_text(s_loading_text_layer, "Loading...");
        layer_add_child(window_layer, text_layer_get_layer(s_loading_text_layer));
    #else
        create_loading_animation(window);
        s_status_bar_layer = status_bar_layer_create();
        status_bar_layer_set_up(s_status_bar_layer);
        layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar_layer));
    #endif
}

static void window_unload(Window *window) {
    menu_layer_destroy(s_bus_stops_menu_layer);
    s_bus_stops_menu_layer = NULL;

    window_destroy(window);
    s_bus_stops_window = NULL;

    #ifdef PBL_PLATFORM_BASALT
        destroy_loading_animation();
        status_bar_layer_destroy(s_status_bar_layer);
        s_status_bar_layer = NULL;
    #endif
    
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
        if (s_bus_stops_window) {
            menu_load();

            #ifdef PBL_PLATFORM_BASALT
                destroy_loading_animation();
            #endif
        }
    }
}


