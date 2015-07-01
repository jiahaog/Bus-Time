#include "bus_arrival_notifier.h"

#define NOTIFICATION_MESSAGE_BUFFER_SIZE 16

// Requests for a notification using the last entry in the details list buffer
static void request_for_notification(bool start) {
    // message format {set_or_cancel_notification}|{stop_id}|{service_no}
    
    // size 1 because string elements are simply chars
    char delimiter_buffer[1] = {MESSAGE_DELIMITER};
    char set_notification_token[2];  // need size 2 here somehow because of string terminator when doing snprintf
    
    // bool current_notification_state = notification_list_get_status(details_list[0], details_list[1]);

    if (start) {
        // set token to 1
        snprintf(set_notification_token, sizeof(set_notification_token), "%c", '1');
    } else {
        // set token to 0
        snprintf(set_notification_token, sizeof(set_notification_token), "%c", '0');
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

void start_notification_service() {
    request_for_notification(true);
}

void cancel_notification_service() {
    request_for_notification(false);
}
