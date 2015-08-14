#include "details_window.h"

#define PLACEHOLDER_TIME_STRING "99:99"
#define CONTENT_X_PADDING 5
#define DETAILS_LAYER_FONT FONT_KEY_GOTHIC_14_BOLD   
#define NO_OF_DETAILS_TEXT_LAYERS 3

#ifdef PBL_COLOR

    #define SEAT_AVAIL_COLOR GColorMediumAquamarine
    #define STAND_AVAIL_COLOR GColorIcterine
    #define STAND_LIMITED_COLOR GColorMelon

#endif

#ifdef PBL_SDK_3
    static StatusBarLayer *s_status_bar;
#endif

static Window *s_details_window;
static TextLayer *s_time_text_layer;
static TextLayer *s_details_text_layers[NO_OF_DETAILS_TEXT_LAYERS];
static Layer *s_knob_graphic_layer;
static char *s_current_service;
static char *s_current_stop_id;
static bool content_loaded = false;

static BitmapLayer *s_layer_bus_icon;
static GBitmap *s_bitmap_bus_icon;

static BitmapLayer *s_layer_alert_icon;
static GBitmap *s_bitmap_alert_icon;


#ifdef PBL_COLOR
static void set_text_layer_color_for_load(TextLayer *text_layer, char load_char) {
    GColor load_color;
    if (load_char == '3') {
        load_color = SEAT_AVAIL_COLOR;
    } else if (load_char == '2') {
        load_color = STAND_AVAIL_COLOR;
    } else if (load_char == '1') {
        load_color = STAND_LIMITED_COLOR;
    } else {
        // ignore 
    }
    text_layer_set_text_color(text_layer, load_color);
}
#endif 

static void details_layers_load(GRect content_bounds) {

    Layer *window_layer = window_get_root_layer(s_details_window);

    // data store index
    // 0 - stop id
    // 1 - service no
    // 2 - next bus est. time
    // 3 - next bus seating
    // 4 - subsequent bus est. time
    // 5 - subsequent bus seating

    // text_layers
    // 0 - service_no
    // 1 - next bus est. time
    // 2 - sub bus est. time

    // TODO: put bus stop name as well

    GColor default_text_color;
    #ifdef PBL_COLOR
        default_text_color = COLOR_TEXT;
    #else
        default_text_color = NO_COLOR_TEXT;
    #endif

    // SERVICE NO
    GFont service_no_font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
    int16_t service_no_height = get_font_height(s_details_window, service_no_font);

    GRect service_no_bounds = GRect(0, APLITE_Y_OFFSET + 19, content_bounds.size.w, service_no_height);
    s_details_text_layers[0] = text_layer_create(service_no_bounds);

    text_layer_set_text(s_details_text_layers[0], details_list[1]);
    text_layer_set_text_color(s_details_text_layers[0], default_text_color);
    text_layer_set_font(s_details_text_layers[0], service_no_font);
    text_layer_set_text_alignment(s_details_text_layers[0], GTextAlignmentCenter);


    // NEXT BUS EST TIME
    
    GFont next_bus_time_font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);

    int16_t next_bus_time_height = get_font_height(s_details_window, next_bus_time_font);
    GRect next_bus_time_bounds = GRect(0, APLITE_Y_OFFSET + 108, content_bounds.size.w, next_bus_time_height);
    s_details_text_layers[1] = text_layer_create(next_bus_time_bounds);
    text_layer_set_text(s_details_text_layers[1], details_list[2]);
    text_layer_set_text_color(s_details_text_layers[1], default_text_color);
    text_layer_set_font(s_details_text_layers[1], next_bus_time_font);
    text_layer_set_text_alignment(s_details_text_layers[1], GTextAlignmentCenter);

    // SUB BUS EST TIME
    GFont sub_bus_time_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    int16_t sub_bus_time_height = get_font_height(s_details_window, sub_bus_time_font);
    GRect sub_bus_time_bounds = GRect(0, APLITE_Y_OFFSET + 142, content_bounds.size.w, sub_bus_time_height);
    s_details_text_layers[2] = text_layer_create(sub_bus_time_bounds);
    text_layer_set_text(s_details_text_layers[2], details_list[4]);
    text_layer_set_text_color(s_details_text_layers[2], default_text_color);
    text_layer_set_font(s_details_text_layers[2], sub_bus_time_font);
    text_layer_set_text_alignment(s_details_text_layers[2], GTextAlignmentCenter);
    
    #ifdef PBL_COLOR
        char next_bus_load = details_list[3][0];
        char sub_bus_load = details_list[5][0];
        set_text_layer_color_for_load(s_details_text_layers[1], next_bus_load);
        set_text_layer_color_for_load(s_details_text_layers[2], sub_bus_load);
    #endif


    for (int i = 0; i < NO_OF_DETAILS_TEXT_LAYERS; i++ ) {

        text_layer_set_background_color(s_details_text_layers[i], GColorClear);
        layer_add_child(window_layer, text_layer_get_layer(s_details_text_layers[i]));
    }

    // Icon

    s_bitmap_bus_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BUS_ICON);
    GRect bus_icon_bounds = GRect(34, APLITE_Y_OFFSET + 61, 80, 43);
    s_layer_bus_icon = bitmap_layer_create(bus_icon_bounds);
    bitmap_layer_set_bitmap(s_layer_bus_icon, s_bitmap_bus_icon);

    // The icon for the black is simply the color one inverted, and doing GCompOpAssignInverted will invert it back to 
    // the regular desired appearance
    #ifdef PBL_PLATFORM_APLITE
        bitmap_layer_set_compositing_mode(s_layer_bus_icon, GCompOpAssignInverted);
    #else
        bitmap_layer_set_compositing_mode(s_layer_bus_icon, GCompOpSet);
    #endif
    bitmap_layer_set_alignment(s_layer_bus_icon, GAlignTopLeft);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_bus_icon));

}
static void knob_graphic_update_callback(Layer *layer, GContext *ctx) {
    
    graphics_context_set_fill_color(ctx, GColorBlack);
    GPoint circle_center = GPoint(16, 16);
    graphics_fill_circle(ctx, circle_center, 16);
}

static void draw_circle() {

    Layer *window_layer = window_get_root_layer(s_details_window);
    s_knob_graphic_layer = layer_create(GRect(139, APLITE_Y_OFFSET + 68, 32, 32));
    layer_set_update_proc(s_knob_graphic_layer, knob_graphic_update_callback);
    layer_add_child(window_layer, s_knob_graphic_layer);
}

static void details_layers_unload() {
    for (int i = 0; i < NO_OF_DETAILS_TEXT_LAYERS; i++) {
        text_layer_destroy(s_details_text_layers[i]);
    }

    gbitmap_destroy(s_bitmap_bus_icon);
    bitmap_layer_destroy(s_layer_bus_icon);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    action_menu_window_push();
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void draw_alert_icon() {
    if (!s_layer_alert_icon) {
        Layer *window_layer = window_get_root_layer(s_details_window);
        GRect window_bounds = layer_get_bounds(window_layer);

        s_bitmap_alert_icon = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ALERT);
        GRect alert_bounds = gbitmap_get_bounds(s_bitmap_alert_icon);
        s_layer_alert_icon = bitmap_layer_create(GRect(6, APLITE_Y_OFFSET + 21, alert_bounds.size.w, alert_bounds.size.h));
        bitmap_layer_set_bitmap(s_layer_alert_icon, s_bitmap_alert_icon);
        bitmap_layer_set_compositing_mode(s_layer_alert_icon, GCompOpSet);

        layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_alert_icon));
    } else {
        layer_set_hidden(bitmap_layer_get_layer(s_layer_alert_icon), false);
    }
}

// hides the alert button
// not a static method because we want this to be called by the controller when
// notifications are cancelled
void hide_alert_icon() {
    if (s_layer_alert_icon) {
        layer_set_hidden(bitmap_layer_get_layer(s_layer_alert_icon), true);
    }
}

// helper method to draw the alert symbol depending on whether a notification exists
static void manage_alert_display() {
    if (notification_exists(s_current_stop_id, s_current_service)) {
        draw_alert_icon();
    } else {
        hide_alert_icon();
    }
}

static void content_load() {

    Layer *window_layer = window_get_root_layer(s_details_window);
    GRect window_bounds = layer_get_bounds(window_layer);

    #ifdef PBL_SDK_3
        // subtract the height of the Status bar 
        GRect content_bounds = GRect(window_bounds.origin.x, window_bounds.origin.y + STATUS_BAR_LAYER_HEIGHT, window_bounds.size.w, window_bounds.size.h - STATUS_BAR_LAYER_HEIGHT);
    #else
        GRect content_bounds = window_bounds;
    #endif

    details_layers_load(content_bounds);

    // only allow the menu to be called when the details have been loaded
    window_set_click_config_provider(s_details_window, click_config_provider);

    draw_circle();
    manage_alert_display();
    content_loaded = true;
}

static void content_unload() {
    details_layers_unload();
    layer_destroy(s_knob_graphic_layer);
    
    if (s_layer_alert_icon) {
        gbitmap_destroy(s_bitmap_alert_icon);
        bitmap_layer_destroy(s_layer_alert_icon);
        
        // have to set to null, because this file is not an object and the variables
        // will stay the same even after the window is destroyed. Sigh. Will 
        // cause double free exception
        s_layer_alert_icon = NULL;
        s_bitmap_alert_icon = NULL;
    }
    
    content_loaded = false;
}

static void window_load(Window *window) {

    window_set_up(window);
    
    #ifdef PBL_PLATFORM_APLITE
        
    #else
        create_loading_animation(window);        
    #endif

    #ifdef PBL_SDK_3
        Layer *window_layer = window_get_root_layer(s_details_window);
        s_status_bar = status_bar_layer_create();
        status_bar_layer_set_up(s_status_bar);
        layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
    #endif
}

static void window_appear(Window *window) {
    watch_bus_service_details(s_current_service);

    if (content_loaded) {
        manage_alert_display();
    }
}

static void window_unload(Window *window) {
    content_unload();

    #ifdef PBL_PLATFORM_APLITE
        // todo 
    #else
        details_list_reset();
        destroy_loading_animation();
    #endif

    #ifdef PBL_SDK_3
        status_bar_layer_destroy(s_status_bar);
        s_status_bar = NULL;
    #endif
        
    window_destroy(window);
    s_details_window = NULL;

}

void details_window_push(char *current_stop_id, char *current_service) {
    s_current_stop_id = current_stop_id;
    s_current_service = current_service;

    if (!s_details_window) {
        s_details_window = window_create();
        window_set_window_handlers(s_details_window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
            .appear = window_appear
        });
    }

    window_stack_push(s_details_window, true);
}

void details_window_reload_details() {

    // guard here to not do anything if the window is not loaded
    // this prevents errors when switching between menus fast when the controller
    // receives an app message to reload the details while the window is already closed
    if (!s_details_window) {
        return;
    }

    #ifdef PBL_PLATFORM_BASALT
        destroy_loading_animation();
    #endif

    if (!content_loaded) {
        content_load();
    } else {
        // not first load of content, so layers are initialized and we can execute functions on them
        for (int i = 0; i < NO_OF_DETAILS_TEXT_LAYERS; i++) {
            layer_mark_dirty(text_layer_get_layer(s_details_text_layers[i]));
        }        
    }
}
