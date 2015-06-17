#include "details_window.h"

#define LOADING_MESSAGE "Loading..."

static Window *s_details_window;
static TextLayer *s_details_text_layer;
static ActionBarLayer *s_action_bar;

static bool s_notification_on = false;

#ifdef PBL_PLATFORM_APLITE
    char s_details_message[100] = LOADING_MESSAGE;
#else 
    char s_details_message[100];
    static StatusBarLayer *s_status_bar_layer;
#endif

GBitmap *s_bitmap_alert_set;
GBitmap *s_bitmap_alert_cancel;


static void set_action_bar_icon(bool show_set_icon) {
    if (show_set_icon) {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing set icon");
        #ifdef PBL_PLATFORM_APLITE
            action_bar_layer_set_icon(s_action_bar, BUTTON_ID_SELECT, s_bitmap_alert_set);
        #else
            action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_SELECT, s_bitmap_alert_set, true);
        #endif
    } else {
         APP_LOG(APP_LOG_LEVEL_DEBUG, "Showing cancel icon");
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

static void toggle_notification() {

    if (!s_notification_on) {
        register_or_cancel_notification(s_notification_on); // false for cancel
        s_notification_on = true;
        set_action_bar_icon(s_notification_on); // true to show set_alert icon
    } else {
        register_or_cancel_notification(s_notification_on); // true for register notification
        s_notification_on = false;
        set_action_bar_icon(s_notification_on); // false to show cancel icon
    }
}

void toggle_notification_click_handler(ClickRecognizerRef recognizer, void *context) {
    toggle_notification();
}

static void action_bar_click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, (ClickHandler) toggle_notification_click_handler);
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


    toggle_notification();
    
}

static void window_load(Window *window) {
    window_set_up(window);
    Layer *window_layer = window_get_root_layer(window);
    GRect window_bounds = layer_get_bounds(window_layer);

    #ifdef PBL_PLATFORM_APLITE
        GRect content_bounds = window_bounds;
        
    #else
        if (strlen(s_details_message) == 0) {
            create_loading_animation(window);
        }


        s_status_bar_layer = status_bar_layer_create();
        status_bar_layer_set_up(s_status_bar_layer);
        layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar_layer));

        GRect content_bounds = window_with_status_bar_content_bounds(window_layer, s_status_bar_layer);
    #endif

    s_details_text_layer = text_layer_create(content_bounds);
    text_layer_set_up(s_details_text_layer);
    text_layer_set_text(s_details_text_layer, s_details_message);
    layer_add_child(window_layer, text_layer_get_layer(s_details_text_layer));

}

static void window_unload(Window *window) {
    text_layer_destroy(s_details_text_layer);
    window_destroy(window);
    s_details_window = NULL;

    action_bar_layer_destroy(s_action_bar);
    s_action_bar = NULL;
    gbitmap_destroy(s_bitmap_alert_set);
    gbitmap_destroy(s_bitmap_alert_cancel);

    #ifdef PBL_PLATFORM_APLITE
        snprintf(s_details_message, sizeof(s_details_message), LOADING_MESSAGE);
    #else
        memset(s_details_message, 0, sizeof s_details_message);
        destroy_loading_animation();
        status_bar_layer_destroy(s_status_bar_layer);
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

void details_window_set_text(char *message) {
    #ifdef PBL_PLATFORM_BASALT
        destroy_loading_animation();
    #endif

    snprintf(s_details_message, sizeof(s_details_message), "%s", message);
    layer_mark_dirty(text_layer_get_layer(s_details_text_layer));

    if (strcmp(s_details_message,LOADING_MESSAGE) != 0 ) {

        if (!s_action_bar) {
            action_bar_load();
        }
    }

}
