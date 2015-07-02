#include "bus_arrival_notifier.h"

#define NOTIFICATION_MESSAGE_BUFFER_SIZE 16
#define CAT_MESSAGE_DELIMITER "|\0"
#define NOTIFICATION_POLL_INTERVAL 30000 // 30 sec

// Requests for a notification using the last entry in the details list buffer
static void request_for_notification(char *stop_id, char *service_no) {
    // message format {stop_id}|{service_no}
    
    // size 2 because string elements are simply chars and include the string
    // terminator
    char delimiter_buffer[2] = CAT_MESSAGE_DELIMITER;
    
    // create a buffer and concatenate the details together
    char notification_message_buffer[NOTIFICATION_MESSAGE_BUFFER_SIZE];

    strcpy(notification_message_buffer, stop_id);
    strcat(notification_message_buffer, delimiter_buffer);
    strcat(notification_message_buffer, service_no);

    send_app_message_string(KEY_BUS_NOTIFICATION, notification_message_buffer);
}

static void notification_list_callback(void *data) {
    char **callback_context = (char**)data;

    char *stop_id = callback_context[0];
    char *service_no = callback_context[1];

    notification_store_remove(stop_id, service_no);

    AppTimer *timer = app_timer_register(NOTIFICATION_POLL_INTERVAL, notification_list_callback, callback_context);
    notification_store_add(stop_id, service_no, timer);

    request_for_notification(stop_id, service_no);
}


void start_notification_service() {
    
    char *stop_id = details_list[0];
    char *service_no = details_list[1];

    
    request_for_notification(stop_id, service_no);

    char callback_context[2][NOTIFICATION_STORE_STRING_SIZE];
    snprintf(callback_context[0], sizeof(callback_context[0]), "%s", stop_id);
    snprintf(callback_context[1], sizeof(callback_context[1]), "%s", service_no);
    AppTimer *timer = notification_store_get(stop_id, service_no);
    if (!timer) {
        AppTimer *new_timer = app_timer_register(NOTIFICATION_POLL_INTERVAL, notification_list_callback, callback_context);
        notification_store_add(stop_id, service_no, new_timer);
    }

    print_notification_store();
}

void cancel_notification_service() {
    char *stop_id = details_list[0];
    char *service_no = details_list[1];

    notification_store_remove(stop_id, service_no);
    print_notification_store();
}
