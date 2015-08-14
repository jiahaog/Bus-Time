#include "notification_store.h"

#define NOTIFICATION_STORE_MAX_ELEMENTS 4

static char s_notification_store[NOTIFICATION_STORE_MAX_ELEMENTS][2][NOTIFICATION_STORE_STRING_SIZE];
static AppTimer *s_notification_app_timer_store[NOTIFICATION_STORE_MAX_ELEMENTS];
static int s_notifications_counter = -1;

void notification_store_add(char *stop_id, char *service_no, AppTimer *timer) {
    // check if an existing stop id and service no is in the array and set it to the timer
    for (int i = 0; i < NOTIFICATION_STORE_MAX_ELEMENTS; i++) {
        char *current_stop_id = s_notification_store[i][0];
        char *current_service_no = s_notification_store[i][1];

        if (strcmp(stop_id, current_stop_id) == 0 && strcmp(service_no, current_service_no) == 0) {
            s_notification_app_timer_store[i] = timer;
            return;
        }
    }

    s_notifications_counter++; // counter starts at -1

    // Check for rolling back to the start of the list, and if so we want to 
    // cancel the timer already there
    if (s_notifications_counter >= NOTIFICATION_STORE_MAX_ELEMENTS) {
        s_notifications_counter = s_notifications_counter % NOTIFICATION_STORE_MAX_ELEMENTS;

        AppTimer *timer_to_override = s_notification_app_timer_store[s_notifications_counter];
        if (timer_to_override) {
            app_timer_cancel(timer_to_override);
            s_notification_app_timer_store[s_notifications_counter] = NULL;
        }
    }

    snprintf(s_notification_store[s_notifications_counter][0], sizeof(s_notification_store[s_notifications_counter][0]), "%s", stop_id);
    snprintf(s_notification_store[s_notifications_counter][1], sizeof(s_notification_store[s_notifications_counter][1]), "%s", service_no);

    s_notification_app_timer_store[s_notifications_counter] = timer;
}


void notification_store_remove(char *stop_id, char *service_no) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Cancelling notification timer: %s, %s", stop_id, service_no);
    for (int i = 0; i < NOTIFICATION_STORE_MAX_ELEMENTS; i++) {
        char *current_stop_id = s_notification_store[i][0];
        char *current_service_no = s_notification_store[i][1];

        if (strcmp(stop_id, current_stop_id) == 0 && strcmp(service_no, current_service_no) == 0) {

            // clear the stop id in the spot, so when get() is called the invariant is maintained
            snprintf(s_notification_store[i][0], sizeof(s_notification_store[i][0]), "111");
            snprintf(s_notification_store[i][1], sizeof(s_notification_store[i][1]), "aaa");
            app_timer_cancel(s_notification_app_timer_store[i]);
            s_notification_app_timer_store[i] = NULL;
        }
    }
}

AppTimer * notification_store_get(char *stop_id, char *service_no) {
    for (int i = 0; i < NOTIFICATION_STORE_MAX_ELEMENTS; i++) {
        char *current_stop_id = s_notification_store[i][0];
        char *current_service_no = s_notification_store[i][1];

        // invariant: if the string matches, the app timer returned has to be valid
        if (strcmp(stop_id, current_stop_id) == 0 && strcmp(service_no, current_service_no) == 0) {
            return s_notification_app_timer_store[i];
        }
    }

    return NULL;
}

void print_notification_store() {
    for (int i = 0; i < NOTIFICATION_STORE_MAX_ELEMENTS; i++) {
        char *current_stop_id = s_notification_store[i][0];
        char *current_service_no = s_notification_store[i][1];

        APP_LOG(APP_LOG_LEVEL_DEBUG, "_%s_%s_", current_stop_id, current_service_no);
    }
}


void cancel_notification_timer_from_message(char *message) {
    // normal splitting of data by delimiter into a buffer 
    char splitted_buffer[NOTIFICATION_APP_MESSAGE_PARTS][NOTIFICATION_STORE_STRING_SIZE];
    memset(splitted_buffer, 0, sizeof splitted_buffer);  // SOMEHOW THIS IS THE SOLUTION TO RANDOM CHARACTERS APPEARING IN THE BUFFER


    // APP_LOG(APP_LOG_LEVEL_DEBUG, "SPLITTING STIRNG: %s", current_string);
    int delimiters[NOTIFICATION_APP_MESSAGE_PARTS - 1] = {};
    int delimiter_counter = 0;
    
    for (int i = 0; (unsigned)i < strlen(message); i++) {
        
        if (message[i] == MESSAGE_DELIMITER) {
            delimiters[delimiter_counter] = i;
            delimiter_counter++;
        }
    }

    delimiter_counter = 0;
    int current_char_counter = 0;
    for (int i = 0; (unsigned)i < strlen(message); i++) {
        char currentChar = message[i];
        
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

    notification_store_remove(stop_id, service_no);
}





