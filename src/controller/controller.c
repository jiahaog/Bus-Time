#include "controller.h"

static int s_bus_stop_list_message_counter = 0;
static int s_service_list_message_counter = 0; 


static void handleError(int code) {
    if (code == 1) {
        // network error
        window_stack_pop_all(false);
        error_window_push((char*)"network failure");
    } else if (code == 2) {
        // no services available
        window_stack_pop(false); // only pop the current window, not all
        error_window_push((char*)"No services available");
    } else if (code == 3) {
        // phone is not connected
        window_stack_pop_all(false);
        error_window_push((char*)"Phone is not connected");
    } else if (code == 4) {
        window_stack_pop_all(false);
        error_window_push((char*)"Failed to get location");
    } else if (code == 5) {
        window_stack_pop_all(false);
        error_window_push((char*)"Reopen the app to continue");
    } else {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error code %i not recognized", code);
    }
}

static void bluetooth_event_callback(bool connected) {
    if (!connected) {
        handleError(3);
    }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

    // read first item    
    Tuple *t = dict_read_first(iterator);

    while(t != NULL) {
        switch (t->key) {
            case KEY_BUS_STOP_LIST_START:
                s_bus_stop_list_message_counter = 0;
                bus_stop_list_reset();
                break;
            case KEY_BUS_STOP_LIST_VALUE:
                // APP_LOG(APP_LOG_LEVEL_ERROR, "received service: %s", t->value->cstring);
                bus_stop_list_add_string(t->value->cstring);
                s_bus_stop_list_message_counter++;
                break;
            case KEY_BUS_STOP_LIST_END:
                split_bus_stop_data();
                bus_stops_window_reload_menu();
                break;

            case KEY_BUS_SERVICE_LIST_START:
                s_service_list_message_counter = 0;
                // need to reset service list so that if the previous service list contains more elements than the received one, it will not 
                // be shown
                service_list_reset();
                break;
            case KEY_BUS_SERVICE_LIST_VALUE:
                service_list_add_string(t->value->cstring);
                s_service_list_message_counter++;
                break;
            case KEY_BUS_SERVICE_LIST_END:
                split_service_list_data();
                services_window_reload_menu();
                break;

            case KEY_BUS_SERVICE_DETAILS_VALUE:
                details_list_add_string(t->value->cstring);
                details_window_reload_details();
                break;

            case KEY_BUS_NOTIFICATION:
                notifications_list_add_string(t->value->cstring);
                details_window_reload_details();
                break;

            case KEY_ERROR:
                handleError(t->value->int32);
                break;

            case KEY_APP_ALIVE:
                save_last_app_alive_message_time();
                break;

            default:
                APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
                break;
        }

        t = dict_read_next(iterator);
    }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
    APP_LOG(APP_LOG_LEVEL_ERROR, "%i", reason);
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    // APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

bool controller_init() {
    // AppMessage register callbacks
    if (!bluetooth_connection_service_peek()) {
        handleError(3);
        return false;
    } else {
        app_message_register_inbox_received(inbox_received_callback);
        app_message_register_inbox_dropped(inbox_dropped_callback);
        app_message_register_outbox_failed(outbox_failed_callback);
        app_message_register_outbox_sent(outbox_sent_callback);
        app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

        bluetooth_connection_service_subscribe(bluetooth_event_callback);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Watch inbox opened and callbacks registered");

        // psleep(400);  // we need to sleep before messages can be sent, as outbox takes some time to open
        start_app_alive_service();
        return true;
    }
}
