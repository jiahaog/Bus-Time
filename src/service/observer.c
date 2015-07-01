#include "observer.h"

#define OBSERVER_POLL_INTERVAL 60000

AppTimer *s_observer_timer = NULL;


static void watch_bus_services_list_callback(void *data) {

    send_app_message_string(KEY_BUS_SERVICE_LIST_START, (char *)data);
    s_observer_timer = app_timer_register(OBSERVER_POLL_INTERVAL, watch_bus_services_list_callback, data);
}

void watch_bus_services_list(char *bus_stop_id) {

    if (s_observer_timer) {
        app_timer_cancel(s_observer_timer);
    }
    send_app_message_string(KEY_BUS_SERVICE_LIST_START, bus_stop_id);

    s_observer_timer = app_timer_register(OBSERVER_POLL_INTERVAL, watch_bus_services_list_callback, bus_stop_id);
}


static void watch_bus_service_details_callback(void *data) {

    send_app_message_string(KEY_BUS_SERVICE_DETAILS_START, (char *)data);
    s_observer_timer = app_timer_register(OBSERVER_POLL_INTERVAL, watch_bus_service_details_callback, data);
}

void watch_bus_service_details(char *service_no) {
    if (s_observer_timer) {
        app_timer_cancel(s_observer_timer);
    }
    send_app_message_string(KEY_BUS_SERVICE_DETAILS_START, service_no);

    s_observer_timer = app_timer_register(OBSERVER_POLL_INTERVAL, watch_bus_service_details_callback, service_no);
}


