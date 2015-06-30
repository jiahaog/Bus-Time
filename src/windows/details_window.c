#include "details_window.h"

#define PLACEHOLDER_TIME_STRING "99:99"
#define CONTENT_X_PADDING 5
// #define DETAILS_LAYER_HEIGHT 30
#define DETAILS_LAYER_FONT FONT_KEY_GOTHIC_14_BOLD   

#define NOTIFICATION_MESSAGE_BUFFER_SIZE 16
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
static ActionBarLayer *s_action_bar;

GBitmap *s_bitmap_alert_set;
GBitmap *s_bitmap_alert_cancel;
GBitmap *s_bitmap_bus_icon;

BitmapLayer *s_layer_bus_icon;

static void set_action_bar_notification_icon(bool show_set_icon) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTING NOTIFICAITON ICON %s", show_set_icon ? "SET" : "UNSET");
    if (show_set_icon) {
        #ifdef PBL_PLATFORM_APLITE
            action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_bitmap_alert_set);
        #else
            action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_SELECT, s_bitmap_alert_set, true);
        #endif
    } else {
        #ifdef PBL_PLATFORM_APLITE
            action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_bitmap_alert_cancel);
        #else
            action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_SELECT, s_bitmap_alert_cancel, true);
        #endif
    }
}

static void register_or_cancel_notification(bool register_notification) {

    if (register_notification) {
        // send app message to register notification
    } else {
        // send app message to cancel notification
    }
}

static void toggle_notification_click_handler(ClickRecognizerRef recognizer, void *context) {
    // message format {set_or_cancel_notification}|{stop_id}|{service_no}
    
    // size 1 because string elements are simply chars
    char delimiter_buffer[1] = {MESSAGE_DELIMITER};
    char set_notification_token[2];  // need size 2 here somehow because of string terminator when doing snprintf
    
    bool current_notification_state = notification_list_get_status(details_list[0], details_list[1]);

    if (current_notification_state) {
        // notification is alreadyset
        // set token to 0
        snprintf(set_notification_token, sizeof(set_notification_token), "%c", '0');
    } else {
        // notification is not already set
        // set token to 1
        snprintf(set_notification_token, sizeof(set_notification_token), "%c", '1');
    }

    // gets these details from the store
    char *stop_id = details_list[0];
    char *service_no = details_list[1];

    // create a buffer and concatenate the details together
    char notification_message_buffer[NOTIFICATION_MESSAGE_BUFFER_SIZE]; 
    strcpy(notification_message_buffer, set_notification_token);
    strcat(notification_message_buffer, delimiter_buffer);
    strcat(notification_message_buffer, stop_id);
    strcat(notification_message_buffer, delimiter_buffer);
    strcat(notification_message_buffer, service_no);

    send_app_message_string(KEY_BUS_NOTIFICATION, notification_message_buffer);
}

static void action_bar_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) toggle_notification_click_handler);
}

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

    // SERVICE NO

    GColor default_text_color;
    #ifdef PBL_COLOR
        default_text_color = COLOR_TEXT;
    #else
        default_text_color = NO_COLOR_TEXT;
    #endif

    GFont service_no_font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);
    int16_t service_no_height = get_font_height(s_details_window, service_no_font);

    GRect service_no_bounds = GRect(0, 16, content_bounds.size.w, service_no_height);
    s_details_text_layers[0] = text_layer_create(service_no_bounds);

    text_layer_set_text(s_details_text_layers[0], details_list[1]);
    text_layer_set_text_color(s_details_text_layers[0], default_text_color);
    text_layer_set_font(s_details_text_layers[0], service_no_font);
    text_layer_set_text_alignment(s_details_text_layers[0], GTextAlignmentCenter);


    // NEXT BUS EST TIME
    
    GFont next_bus_time_font = fonts_get_system_font(FONT_KEY_BITHAM_30_BLACK);

    int16_t next_bus_time_height = get_font_height(s_details_window, next_bus_time_font);
    GRect next_bus_time_bounds = GRect(0, 108, content_bounds.size.w, next_bus_time_height);
    s_details_text_layers[1] = text_layer_create(next_bus_time_bounds);
    text_layer_set_text(s_details_text_layers[1], details_list[2]);
    text_layer_set_font(s_details_text_layers[1], next_bus_time_font);
    text_layer_set_text_alignment(s_details_text_layers[1], GTextAlignmentCenter);

    // SUB BUS EST TIME
    GFont sub_bus_time_font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
    int16_t sub_bus_time_height = get_font_height(s_details_window, sub_bus_time_font);
    GRect sub_bus_time_bounds = GRect(0, 149, content_bounds.size.w, sub_bus_time_height);
    s_details_text_layers[2] = text_layer_create(sub_bus_time_bounds);
    text_layer_set_text(s_details_text_layers[2], details_list[4]);
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
    static int16_t bus_icon_origin_x = 34;
    static int16_t bus_icon_origin_y = 45;
    static int16_t bus_icon_size_w = 80;
    static int16_t bus_icon_size_h = 43;
    // GRect bus_icon_bounds = GRect(content_bounds.origin.x + bus_icon_origin_x, content_bounds.origin.y + bus_icon_origin_y, bus_icon_size_w, bus_icon_size_h);
    GRect bus_icon_bounds = GRect(34, 61, 80, 43);
    s_layer_bus_icon = bitmap_layer_create(bus_icon_bounds);
    bitmap_layer_set_bitmap(s_layer_bus_icon, s_bitmap_bus_icon);
    bitmap_layer_set_compositing_mode(s_layer_bus_icon, GCompOpSet);
    bitmap_layer_set_alignment(s_layer_bus_icon, GAlignTopLeft);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_layer_bus_icon));

}

static void details_layers_unload() {
    for (int i = 0; i < NO_OF_DETAILS_TEXT_LAYERS; i++) {
        text_layer_destroy(s_details_text_layers[i]);
    }

    gbitmap_destroy(s_bitmap_bus_icon);
    bitmap_layer_destroy(s_layer_bus_icon);
}

// static void action_bar_load() {
    // s_action_bar = action_bar_layer_create();
    // action_bar_layer_add_to_window(s_action_bar, s_details_window);
    // action_bar_layer_set_click_config_provider(s_action_bar, action_bar_click_config_provider);

    // #ifdef PBL_PLATFORM_BASALT
    //     action_bar_layer_set_background_color(s_action_bar, COLOR_SECONDARY);
    // #endif

    // s_bitmap_alert_set = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ALERT_SET);
    // s_bitmap_alert_cancel = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ALERT_CANCEL);
// }

static void content_load() {

    Layer *window_layer = window_get_root_layer(s_details_window);
    GRect window_bounds = layer_get_bounds(window_layer);

    #ifdef PBL_SDK_3
        // subtract the height of the Status bar 
        GRect content_bounds = GRect(window_bounds.origin.x, window_bounds.origin.y + STATUS_BAR_LAYER_HEIGHT, window_bounds.size.w, window_bounds.size.h - STATUS_BAR_LAYER_HEIGHT);
    #else
        GRect content_bounds = window_bounds;
    #endif

    // action_bar_load();
    
    details_layers_load(content_bounds);
}

static void window_load(Window *window) {
    // always initialise to false 
    // TODO: check from js side to see if notification is on before setting

    window_set_up(window);
    // Layer *window_layer = window_get_root_layer(window);
    // GRect window_bounds = layer_get_bounds(window_layer);

    #ifdef PBL_PLATFORM_APLITE
        // GRect content_bounds = window_bounds;
        
    #else
        create_loading_animation(window);        
        // GRect content_bounds = GRect(window_bounds.origin.x, window_bounds.origin.y, window_bounds.size.w - ACTION_BAR_WIDTH, window_bounds.size.h);

    #endif


    #ifdef PBL_SDK_3
        Layer *window_layer = window_get_root_layer(s_details_window);
        s_status_bar = status_bar_layer_create();
        status_bar_layer_set_up(s_status_bar);
        layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
    #endif
}

static void window_unload(Window *window) {
    details_layers_unload();
    window_destroy(window);
    s_details_window = NULL;

    // action_bar_layer_destroy(s_action_bar);
    // s_action_bar = NULL;

    gbitmap_destroy(s_bitmap_alert_set);
    gbitmap_destroy(s_bitmap_alert_cancel);

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
    // going back to the services list
    // tell js that we are done with the service details
    send_app_message_int(KEY_BUS_SERVICE_DETAILS_END, 1);

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

void details_window_reload_details() {
    #ifdef PBL_PLATFORM_BASALT
        destroy_loading_animation();
    #endif

    if (!s_action_bar) {
        // first load of content & action bar
        // content will load together with action
        // action_bar_load();
        content_load();
    } else {
        // not first load of content, so layers are initialized and we can execute functions on them
        for (int i = 0; i < NO_OF_DETAILS_TEXT_LAYERS; i++) {
            layer_mark_dirty(text_layer_get_layer(s_details_text_layers[i]));
        }        
    }

    // bool current_notification_state = notification_list_get_status(details_list[0], details_list[1]);
    // set_action_bar_notification_icon(!current_notification_state);
}
