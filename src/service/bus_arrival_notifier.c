#include "bus_arrival_notifier.h"

#define CAT_MESSAGE_DELIMITER "|\0"
#define NOTIFICATION_POLL_INTERVAL 30000 // 30 sec


// Concatenates the stop_id and the service_no together with a delimiter
static char * get_notification_message(char *stop_id, char *service_no) {
    // message format {stop_id}|{service_no}
        
        // size 2 because string elements are simply chars and include the string
        // terminator
    char delimiter_buffer[2] = CAT_MESSAGE_DELIMITER;
    
    // create a buffer and concatenate the details together
    char *notification_message_buffer = "";

    strcpy(notification_message_buffer, stop_id);
    strcat(notification_message_buffer, delimiter_buffer);
    strcat(notification_message_buffer, service_no);

    return notification_message_buffer;
}

// Requests for a notification using the last entry in the details list buffer
static void request_for_notification(char *stop_id, char *service_no) {
    // Because we somehow cannot pass a string array in the callback context, we have to concatenate
    // the stop id and the service no into a single message and split it in the callback
    char *notification_message_buffer = get_notification_message(stop_id, service_no);
    send_app_message_string(KEY_BUS_NOTIFICATION, notification_message_buffer);
}

static void notification_list_callback(void* context) {

    char *callback_message = context;

    // split the context into the stop id and the service no
    char splitted_buffer[NOTIFICATION_APP_MESSAGE_PARTS][NOTIFICATION_STORE_STRING_SIZE];
    memset(splitted_buffer, 0, sizeof splitted_buffer);  // SOMEHOW THIS IS THE SOLUTION TO RANDOM CHARACTERS APPEARING IN THE BUFFER

    int delimiters[NOTIFICATION_APP_MESSAGE_PARTS - 1] = {};
    int delimiter_counter = 0;
    
    for (int i = 0; (unsigned)i < strlen(callback_message); i++) {
        
        if (callback_message[i] == MESSAGE_DELIMITER) {
            delimiters[delimiter_counter] = i;
            delimiter_counter++;
        }
    }

    delimiter_counter = 0;
    int current_char_counter = 0;
    for (int i = 0; (unsigned)i < strlen(callback_message); i++) {
        char currentChar = callback_message[i];
        
        if (i == delimiters[delimiter_counter]) {
            delimiter_counter++;
            current_char_counter = 0;
            continue;
        } else {
            splitted_buffer[delimiter_counter][current_char_counter] = currentChar;
            current_char_counter++;
        }
    }

    char *stop_id = splitted_buffer[0];
    char *service_no = splitted_buffer[1];

    // register and add it to the store first before sending the request, because we dont 
    // want the race contition where the bus notification is sent and the cancellation message is sent
    // before the timer is registered.
    AppTimer *timer = app_timer_register(NOTIFICATION_POLL_INTERVAL, notification_list_callback, context);
    notification_store_add(stop_id, service_no, timer);

    request_for_notification(stop_id, service_no);
}


void start_notification_service() {
    
    char *stop_id = details_list[0];
    char *service_no = details_list[1];
    
    request_for_notification(stop_id, service_no);

    AppTimer *timer = notification_store_get(stop_id, service_no);

    if (!timer) {

        char *notification_callback_context = get_notification_message(stop_id, service_no);
        AppTimer *new_timer = app_timer_register(NOTIFICATION_POLL_INTERVAL, notification_list_callback, notification_callback_context);
        notification_store_add(stop_id, service_no, new_timer);
    }
}

void cancel_notification_service() {
    char *stop_id = details_list[0];
    char *service_no = details_list[1];

    notification_store_remove(stop_id, service_no);
}
