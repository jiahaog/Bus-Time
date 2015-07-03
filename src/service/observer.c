#include "observer.h"

#define OBSERVER_POLL_INTERVAL 60000

static AppTimer *s_observer_timer = NULL; // only one observer at one time
static char *s_last_watched_service_no = NULL;

static int s_current_observed_window = NO_WINDOW;

static void watch_bus_services_list_callback(void *data) {

    send_app_message_string(KEY_BUS_SERVICE_LIST_START, (char *)data);
    s_observer_timer = app_timer_register(OBSERVER_POLL_INTERVAL, watch_bus_services_list_callback, data);
}

void watch_bus_services_list(char *bus_stop_id) {
    s_current_observed_window = BUS_STOPS_WINDOW;
    if (s_observer_timer) {
        app_timer_cancel(s_observer_timer);
        s_observer_timer = NULL;
    }
    send_app_message_string(KEY_BUS_SERVICE_LIST_START, bus_stop_id);

    s_observer_timer = app_timer_register(OBSERVER_POLL_INTERVAL, watch_bus_services_list_callback, bus_stop_id);
}


static void watch_bus_service_details_callback(void *data) {

    send_app_message_string(KEY_BUS_SERVICE_DETAILS_START, (char *)data);
    s_observer_timer = app_timer_register(OBSERVER_POLL_INTERVAL, watch_bus_service_details_callback, data);
}

void watch_bus_service_details(char *service_no) {
    s_current_observed_window = DETAILS_WINDOW;
    if (s_observer_timer) {
        app_timer_cancel(s_observer_timer);
        s_observer_timer = NULL;
    }
    send_app_message_string(KEY_BUS_SERVICE_DETAILS_START, service_no);

    s_observer_timer = app_timer_register(OBSERVER_POLL_INTERVAL, watch_bus_service_details_callback, service_no);
    s_last_watched_service_no = service_no;
}

int watchingWhichWindow() {
    return s_current_observed_window;
}

void watch_last_bus_service_details() {
    watch_bus_service_details(s_last_watched_service_no);
}

