#include "services_window.h"

#define SERVICES_CELL_H1_TOP_MARGIN 5
#define SERVICES_CELL_H2_TOP_MARGIN 5
#define SERVICES_CELL_H2_FONT FONT_KEY_GOTHIC_24
#define SERVICES_CELL_X_PADDING 5

#define MENU_LAYER_HEADER_HEIGHT 30
#define MENU_LAYER_HEADER_FONT FONT_KEY_GOTHIC_18_BOLD
#define BUS_STOP_NAME_BUFFER_SIZE 30

static Window *s_services_window;
static MenuLayer *s_services_menu_layer;
static char s_bus_stop_name[BUS_STOP_NAME_BUFFER_SIZE] = "<placehodler>";
static char *s_bus_stop_id;

#ifdef PBL_SDK_3
    static StatusBarLayer *s_status_bar_layer;
#endif

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
        graphics_draw_text(ctx, service_no, fonts_get_system_font(CELL_H1_FONT), title_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        
        GRect arrival_time_bounds = GRect(cell_bounds.origin.x, cell_bounds.origin.y + SERVICES_CELL_H2_TOP_MARGIN, cell_layer_width, cell_bounds.size.h);
        graphics_draw_text(ctx, arrival_time, fonts_get_system_font(SERVICES_CELL_H2_FONT), arrival_time_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentRight, NULL);
    }
}

static int16_t callback_menu_layer_get_header_height(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return MENU_LAYER_HEADER_HEIGHT;
}
static void callback_menu_layer_draw_header(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
    GRect cell_bounds = layer_get_bounds(cell_layer);
    int16_t cell_layer_width = cell_bounds.size.w - 2*SERVICES_CELL_X_PADDING;
    GRect header_bounds = GRect(cell_bounds.origin.x + SERVICES_CELL_X_PADDING, cell_bounds.origin.y, cell_layer_width, cell_bounds.size.h);
    graphics_draw_text(ctx, s_bus_stop_name, fonts_get_system_font(MENU_LAYER_HEADER_FONT), header_bounds, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

// Whatp happens when the select button is pushed
static void callback_menu_layer_select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {

    // only act on the button click if the number of rows is more than zero
    if (get_service_list_count() > 0) {
        char *currentService = service_list[cell_index->row][0];
        send_app_message_string(KEY_BUS_SERVICE_DETAILS_START, currentService);
        details_window_push(currentService);
    }
}

static void menu_load() {
    Layer *window_layer = window_get_root_layer(s_services_window);

    GRect window_bounds = layer_get_bounds(window_layer);

    #ifdef PBL_PLATFORM_APLITE
        GRect content_bounds = window_bounds;
    #else
        GRect content_bounds = window_with_status_bar_content_bounds(window_layer, s_status_bar_layer);
    #endif

    s_services_menu_layer = menu_layer_create(content_bounds);  
    menu_layer_set_callbacks(s_services_menu_layer, service_list, (MenuLayerCallbacks) {
        .get_num_rows = callback_menu_layer_get_num_rows,
        .draw_row = callback_menu_layer_draw_row,
        .select_click = callback_menu_layer_select_click,
        .get_header_height = callback_menu_layer_get_header_height,
        .draw_header = callback_menu_layer_draw_header
    });

    menu_layer_set_up(s_services_menu_layer);

    menu_layer_set_click_config_onto_window(s_services_menu_layer, s_services_window);

    layer_add_child(window_layer, menu_layer_get_layer(s_services_menu_layer));
}

static void window_load(Window *window) {
    window_set_up(window);
    Layer *window_layer = window_get_root_layer(window);   
    GRect bounds = layer_get_bounds(window_layer);

    #ifdef PBL_PLATFORM_APLITE
        
    #else
        create_loading_animation(window);
    #endif

    #ifdef PBL_SDK_3
        s_status_bar_layer = status_bar_layer_create();
        status_bar_layer_set_up(s_status_bar_layer);
        layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar_layer));
    #endif

}
static void window_appear(Window *window) {
    watch_bus_services_list(s_bus_stop_id);
}

static void window_unload(Window *window) {
    // reset the service list on unload so the wrong service list won't be displayed then changed immediately on load
    service_list_reset();

    menu_layer_destroy(s_services_menu_layer);
    window_destroy(window);
    s_services_window = NULL;
    s_services_menu_layer = NULL;

    #ifdef PBL_PLATFORM_BASALT
        destroy_loading_animation();
    #endif

    #ifdef PBL_SDK_3
        status_bar_layer_destroy(s_status_bar_layer);
        s_status_bar_layer = NULL;
    #endif

    // going back to the bus stop list
    // tell js that we are done with the services list
    send_app_message_int(KEY_BUS_SERVICE_LIST_END, 1);

}

void services_window_push(char *bus_stop_name, char *bus_stop_id) {
    s_bus_stop_id = bus_stop_id;

    snprintf(s_bus_stop_name, sizeof(s_bus_stop_name), "%s", bus_stop_name);
    if (!s_services_window) {
        s_services_window = window_create();
        window_set_window_handlers(s_services_window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
            .appear = window_appear
        });
    }

    window_stack_push(s_services_window, true);
}

void services_window_reload_menu() {
    if (s_services_menu_layer) {
        menu_layer_reload_data(s_services_menu_layer);
    } else {
        // Guard here to prevent the menu from loading if this method is called after the window has been unloaded
        if (s_services_window) {
            menu_load();
            #ifdef PBL_PLATFORM_BASALT
                destroy_loading_animation();
            #endif
        }
    }
}

