#include "controller.h"

static int s_bus_stop_list_message_counter = 0;
static int s_service_list_message_counter = 0; 


static void handleError(int code) {
    if (code == 1) {
        // network error
        window_stack_pop_all(false);
        error_window_push((char*)"network failure");
    } else {
        // no services available
        window_stack_pop(false);
        error_window_push((char*)"No services available");
    }

    // // char message[] = "No services operational";
    // // error_window_push(message);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

    // read first item    
    Tuple *t = dict_read_first(iterator);

    char message_buffer[200];

    while(t != NULL) {
        switch (t->key) {
            case KEY_BUS_STOP_LIST_START:
                s_bus_stop_list_message_counter = 0;
                bus_stop_list_reset();
                break;
            case KEY_BUS_STOP_LIST_VALUE:
                // APP_LOG(APP_LOG_LEVEL_ERROR, "received service: %s", t->value->cstring);
                snprintf(bus_stops_list[s_bus_stop_list_message_counter], sizeof(bus_stops_list[s_bus_stop_list_message_counter]), "%s", t->value->cstring);
                s_bus_stop_list_message_counter++;
                break;
            case KEY_BUS_STOP_LIST_END:

                bus_stops_window_reload_menu();
                break;

            case KEY_BUS_SERVICE_LIST_START:
                s_service_list_message_counter = 0;
                // need to reset service list so that if the previous service list contains more elements than the received one, it will not 
                // be shown
                services_list_reset();
                break;
            case KEY_BUS_SERVICE_LIST_VALUE:
                snprintf(services_list[s_service_list_message_counter], sizeof(services_list[s_service_list_message_counter]), "%s", t->value->cstring);
                s_service_list_message_counter++;
                break;
            case KEY_BUS_SERVICE_LIST_END:

                services_window_reload_menu();
                break;

            case KEY_BUS_SERVICE_DETAILS_VALUE:

                snprintf(message_buffer, sizeof(message_buffer), "%s", t->value->cstring);
                details_window_set_text(message_buffer);
                APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting details window text!");
                break;

            case KEY_ERROR:

                handleError(t->value->int32);
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
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


void controller_init() {
    // AppMessage register callbacks
    app_message_register_inbox_received(inbox_received_callback);
    app_message_register_inbox_dropped(inbox_dropped_callback);
    app_message_register_outbox_failed(outbox_failed_callback);
    app_message_register_outbox_sent(outbox_sent_callback);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}
