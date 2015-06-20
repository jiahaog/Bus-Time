#include "details_window.h"

#define PLACEHOLDER_TIME_STRING "00:00"
#define CONTENT_X_PADDING 5
// #define DETAILS_LAYER_HEIGHT 30
#define DETAILS_LAYER_FONT FONT_KEY_GOTHIC_14_BOLD   
#define TIME_LAYER_FONT FONT_KEY_GOTHIC_28
#define NOTIFICATION_MESSAGE_BUFFER_SIZE 16

static Window *s_details_window;
static TextLayer *s_time_text_layer;
static TextLayer *s_details_text_layers[DETAILS_LIST_MESSAGE_PARTS];
static ActionBarLayer *s_action_bar;

GBitmap *s_bitmap_alert_set;
GBitmap *s_bitmap_alert_cancel;

static void update_time() {
    time_t time_now = time(NULL);
    struct tm *tick_time = localtime(&time_now);

    static char time_string_buffer[] = PLACEHOLDER_TIME_STRING;

    if (clock_is_24h_style() == true) {
        strftime(time_string_buffer, sizeof(PLACEHOLDER_TIME_STRING), "%H:%M", tick_time);
    } else {
        strftime(time_string_buffer, sizeof(PLACEHOLDER_TIME_STRING), "%I:%M", tick_time);
    }

    text_layer_set_text(s_time_text_layer, time_string_buffer);
}

static void time_tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
}

static void set_action_bar_notification_icon(bool show_set_icon) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTING NOTIFICAITON ICON %s", show_set_icon ? "true" : "false");
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


static void details_layers_load(GRect content_bounds) {

    Layer *window_layer = window_get_root_layer(s_details_window);
   
    int16_t content_width = content_bounds.size.w - 2*CONTENT_X_PADDING;
    int16_t content_height = get_font_height(s_details_window, fonts_get_system_font(DETAILS_LAYER_FONT));
    int16_t details_top_margin = 50;

    for (int i = 0; i < DETAILS_LIST_MESSAGE_PARTS; i++ ) {

        GRect current_details_bounds = GRect(CONTENT_X_PADDING, details_top_margin + content_bounds.origin.y + content_height * i, content_width, content_height);
        s_details_text_layers[i] = text_layer_create(current_details_bounds);
        text_layer_set_up(s_details_text_layers[i]);
        text_layer_set_font(s_details_text_layers[i], fonts_get_system_font(DETAILS_LAYER_FONT));
        text_layer_set_text(s_details_text_layers[i], details_list[i]);
        layer_add_child(window_layer, text_layer_get_layer(s_details_text_layers[i]));
    }
}

static void time_layer_load(GRect content_bounds) {
    Layer *window_layer = window_get_root_layer(s_details_window);

    int16_t time_layer_height = get_font_height(s_details_window, fonts_get_system_font(TIME_LAYER_FONT));
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Time layer height: %i", time_layer_height);
    GRect time_layer_bounds = GRect(content_bounds.origin.x, content_bounds.origin.y, content_bounds.size.w, time_layer_height);

    s_time_text_layer = text_layer_create(time_layer_bounds);
    text_layer_set_up(s_time_text_layer);
    text_layer_set_font(s_time_text_layer, fonts_get_system_font(TIME_LAYER_FONT));

    layer_add_child(window_layer, text_layer_get_layer(s_time_text_layer));
    update_time();
    tick_timer_service_subscribe(MINUTE_UNIT, time_tick_handler);
}

static void details_layers_unload() {
    for (int i = 0; i < DETAILS_LIST_MESSAGE_PARTS; i++) {
        text_layer_destroy(s_details_text_layers[i]);
    }
    text_layer_destroy(s_time_text_layer);
}

static void action_bar_load() {
    s_action_bar = action_bar_layer_create();
    action_bar_layer_add_to_window(s_action_bar, s_details_window);
    action_bar_layer_set_click_config_provider(s_action_bar, action_bar_click_config_provider);

    #ifdef PBL_PLATFORM_BASALT
        action_bar_layer_set_background_color(s_action_bar, COLOR_SECONDARY);
    #endif

    s_bitmap_alert_set = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ALERT_SET);
    s_bitmap_alert_cancel = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ALERT_CANCEL);
}

static void content_load() {

    Layer *window_layer = window_get_root_layer(s_details_window);
    GRect window_bounds = layer_get_bounds(window_layer);

    #ifdef PBL_PLATFORM_APLITE
        GRect content_bounds = window_bounds;
    #else
        // subtract the width of the action bar away
        GRect content_bounds = GRect(window_bounds.origin.x, window_bounds.origin.y, window_bounds.size.w - ACTION_BAR_WIDTH, window_bounds.size.h);
    #endif

    action_bar_load();
    details_layers_load(content_bounds);
    time_layer_load(content_bounds);
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

    // details_layers_load(window, content_bounds);
}

static void window_unload(Window *window) {
    details_layers_unload();
    window_destroy(window);
    s_details_window = NULL;

    action_bar_layer_destroy(s_action_bar);
    s_action_bar = NULL;

    gbitmap_destroy(s_bitmap_alert_set);
    gbitmap_destroy(s_bitmap_alert_cancel);

    #ifdef PBL_PLATFORM_APLITE
        // todo 
    #else
        details_list_reset();
        destroy_loading_animation();
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
        for (int i = 0; i < DETAILS_LIST_MESSAGE_PARTS; i++) {
            layer_mark_dirty(text_layer_get_layer(s_details_text_layers[i]));
        }        
    }

    bool current_notification_state = notification_list_get_status(details_list[0], details_list[1]);
    set_action_bar_notification_icon(!current_notification_state);
}
