#include "store.h"

char s_bus_stop_string_buffer[BUS_STOP_LIST_MAX_ELEMENTS][STRING_SIZE];
char s_service_list_string_buffer[SERVICE_LIST_MAX_ELEMENTS][STRING_SIZE];
char s_details_list_string_buffer[STRING_SIZE];
// char s_notifications_list_string_buffer[NOTIFICATION_LIST_MAX_ELEMENTS][STRING_SIZE];

int s_bus_stop_list_counter = 0;
int s_service_list_counter = 0;
// int s_notifications_list_counter = 0;

time_t s_last_app_alive_message_time;

// Sets up default variables for store
void store_init() {
    s_last_app_alive_message_time = time(NULL);
}

void bus_stop_list_reset() {
    s_bus_stop_list_counter = 0;
    memset(s_bus_stop_string_buffer, 0, sizeof s_bus_stop_string_buffer);
    memset(bus_stop_list, 0, sizeof bus_stop_list);   
}

void service_list_reset() {
    s_service_list_counter = 0;
    memset(s_service_list_string_buffer, 0, sizeof s_service_list_string_buffer);
    memset(service_list, 0, sizeof service_list);
}

void details_list_reset() {
    memset(s_details_list_string_buffer, 0, sizeof s_details_list_string_buffer);
    memset(details_list, 0, sizeof details_list);
}

void bus_stop_list_add_string(char *string) {
    snprintf(s_bus_stop_string_buffer[s_bus_stop_list_counter], sizeof(s_bus_stop_string_buffer[s_bus_stop_list_counter]), "%s", string);
    s_bus_stop_list_counter++;
}

void service_list_add_string(char *string) {
    snprintf(s_service_list_string_buffer[s_service_list_counter], sizeof(s_service_list_string_buffer[s_service_list_counter]), "%s", string);
    s_service_list_counter++;
}


// static void split_notifications_list_data() {

//     // normal splitting of data by delimiter into a buffer 
//     char splitted_buffer[NOTIFICATION_LIST_MESSAGE_PARTS][STRING_SIZE];
//     memset(splitted_buffer, 0, sizeof splitted_buffer);  // SOMEHOW THIS IS THE SOLUTION TO RANDOM CHARACTERS APPEARING IN THE BUFFER

//     char *current_string = s_notifications_list_string_buffer[s_notifications_list_counter];

//     // APP_LOG(APP_LOG_LEVEL_DEBUG, "SPLITTING STIRNG: %s", current_string);
//     int delimiters[NOTIFICATION_LIST_MESSAGE_PARTS - 1] = {};
//     int delimiter_counter = 0;
    
//     for (int i = 0; (unsigned)i < strlen(current_string); i++) {
        
//         if (current_string[i] == MESSAGE_DELIMITER) {
//             delimiters[delimiter_counter] = i;
//             delimiter_counter++;
//         }
//     }

//     delimiter_counter = 0;
//     int current_char_counter = 0;
//     for (int i = 0; (unsigned)i < strlen(current_string); i++) {
//         char currentChar = current_string[i];
        
//         if (i == delimiters[delimiter_counter]) {
//             delimiter_counter++;
//             current_char_counter = 0;
//             continue;
//         } else {
//             splitted_buffer[delimiter_counter][current_char_counter] = currentChar;
//             current_char_counter++;
//         }
//     }

//     char *status = splitted_buffer[0];
//     char *stop_id = splitted_buffer[1];
//     char *service_no = splitted_buffer[2];

//     bool notification_stored = false;

//     // first loop is to find exact old notification
//     for (int i = 0; i < NOTIFICATION_LIST_MAX_ELEMENTS; i++) {
//         char *current_stop_id = notifications_list[i][1];
//         char *current_service_no = notifications_list[i][2];
        
//         // APP_LOG(APP_LOG_LEVEL_DEBUG, "COMPARING |%s|%s|", stop_id, current_stop_id);
//         // APP_LOG(APP_LOG_LEVEL_DEBUG, "COMPARING |%s|%s|", service_no, current_service_no);

//         bool same_stop_id = strcmp(stop_id, current_stop_id) == 0;
//         bool same_service_no = strcmp(service_no, current_service_no) == 0;

//         if (same_stop_id && same_service_no) {
//             // APP_LOG(APP_LOG_LEVEL_DEBUG, "OLD NOTIFCATION IS FOUND");
//             snprintf(notifications_list[i][0], sizeof(notifications_list[i][0]), "%s", status);
//             notification_stored = true;
//             // APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTING STATUS TO %s", notifications_list[i][0]);
//             break;
//         } 
//     }

//     // if cannot find, we want to put it in
//     if (!notification_stored) {
//         // APP_LOG(APP_LOG_LEVEL_DEBUG, "OLD NOTIFCATION IS NOT FOUND");

//         // put the buffer into the store
//         snprintf(notifications_list[s_notifications_list_counter][0], sizeof(notifications_list[s_notifications_list_counter][0]), "%s", status);
//         snprintf(notifications_list[s_notifications_list_counter][1], sizeof(notifications_list[s_notifications_list_counter][1]), "%s", stop_id);
//         snprintf(notifications_list[s_notifications_list_counter][2], sizeof(notifications_list[s_notifications_list_counter][2]), "%s", service_no);
//         // APP_LOG(APP_LOG_LEVEL_DEBUG, "SETTING STATUS TO %s", notifications_list[s_notifications_list_counter][0]);
//         // increment the counter that marks the last notification
//         s_notifications_list_counter = s_notifications_list_counter + 1;
//         s_notifications_list_counter = s_notifications_list_counter % NOTIFICATION_LIST_MAX_ELEMENTS; // reset to 0 to override
//     }
// }

// void notifications_list_add_string(char *string) {
//     snprintf(s_notifications_list_string_buffer[s_notifications_list_counter], sizeof(s_notifications_list_string_buffer[s_notifications_list_counter]), "%s", string);
//     split_notifications_list_data();
// }

// // returns true if the notification is active
// bool notification_list_get_status(char *stop_id, char *service_no) {
//     // APP_LOG(APP_LOG_LEVEL_DEBUG, "TRYING TO FIND %s, %s", stop_id, service_no);
//     for (int i = 0; i < NOTIFICATION_LIST_MAX_ELEMENTS; i++) {
//         char *current_status = notifications_list[i][0];
//         char *current_stop_id = notifications_list[i][1];
//         char *current_service_no = notifications_list[i][2];
//         // APP_LOG(APP_LOG_LEVEL_DEBUG, "CURRENT: %s %s, %s", current_status, current_stop_id, current_service_no);
//         if (strcmp(stop_id, current_stop_id) == 0 && strcmp(service_no, current_service_no) == 0) {
//             // APP_LOG(APP_LOG_LEVEL_DEBUG, "FOUNDD: |%c|", current_status[0]);
//             if (current_status[0] == '1') {
//                 // APP_LOG(APP_LOG_LEVEL_DEBUG, "NOTIFICATION STATUS TRUE");
//                 return true;
//             } else {
//                 // APP_LOG(APP_LOG_LEVEL_DEBUG, "NOTIFICATION STATUS FALSE");
//                 return false;
//             }
//         }
//     }
//     return false;
// }

static void split_details_list_data() {

    // reset the details list
    memset(details_list, 0, sizeof details_list);

    char *current_string = s_details_list_string_buffer;

    int delimiters[DETAILS_LIST_MESSAGE_PARTS - 1] = {};
    int delimiter_counter = 0;
    
    for (int i = 0; (unsigned)i < strlen(current_string); i++) {
        
        if (current_string[i] == MESSAGE_DELIMITER) {
            delimiters[delimiter_counter] = i;
            delimiter_counter++;
        }
    }

    delimiter_counter = 0;
    int current_char_counter = 0;
    for (int i = 0; (unsigned)i < strlen(current_string); i++) {
        char currentChar = current_string[i];
        
        if (i == delimiters[delimiter_counter]) {
            delimiter_counter++;
            current_char_counter = 0;
            continue;
        } else {
            details_list[delimiter_counter][current_char_counter] = currentChar;
            current_char_counter++;
        }
    }
}

void details_list_add_string(char *string) {
    snprintf(s_details_list_string_buffer, sizeof(s_details_list_string_buffer), "%s", string);
    split_details_list_data();
}


int get_service_list_count() {
    return s_service_list_counter;
}

int get_bus_stop_list_count() {
    return s_bus_stop_list_counter;
}

int get_details_list_buffer_length() {
    return strlen(s_details_list_string_buffer);
}

void split_bus_stop_data() {

    for (int j = 0; j < get_bus_stop_list_count(); j++) {
        char *currentBusStopString = s_bus_stop_string_buffer[j];

        int delimiters[BUS_STOP_LIST_MESSAGE_PARTS - 1] = {};
        int delimiter_counter = 0;
        
        for (int i = 0; (unsigned)i < strlen(currentBusStopString); i++) {
            
            if (currentBusStopString[i] == MESSAGE_DELIMITER) {
                delimiters[delimiter_counter] = i;
                delimiter_counter++;
            }
        }
    
        delimiter_counter = 0;
        int current_char_counter = 0;
        for (int i = 0; (unsigned)i < strlen(currentBusStopString); i++) {
            char currentChar = currentBusStopString[i];
            
            if (i == delimiters[delimiter_counter]) {
                delimiter_counter++;
                current_char_counter = 0;
                continue;
            } else {
                bus_stop_list[j][delimiter_counter][current_char_counter] = currentChar;
                current_char_counter++;
            }
        }

    }
}

void split_service_list_data() {

    for (int j = 0; j < get_service_list_count(); j++) {
        char *current_string = s_service_list_string_buffer[j];

        int delimiters[SERVICE_LIST_MESSAGE_PARTS - 1] = {};
        int delimiter_counter = 0;
        
        for (int i = 0; (unsigned)i < strlen(current_string); i++) {
            
            if (current_string[i] == MESSAGE_DELIMITER) {
                delimiters[delimiter_counter] = i;
                delimiter_counter++;
            }
        }
    
        delimiter_counter = 0;
        int current_char_counter = 0;
        for (int i = 0; (unsigned)i < strlen(current_string); i++) {
            char currentChar = current_string[i];
            
            if (i == delimiters[delimiter_counter]) {
                delimiter_counter++;
                current_char_counter = 0;
                continue;
            } else {
                service_list[j][delimiter_counter][current_char_counter] = currentChar;
                current_char_counter++;
            }
        }
    }
}

void save_last_app_alive_message_time() {
    s_last_app_alive_message_time = time(NULL);  // seconds
}

// returns in seconds
time_t get_time_since_last_app_alive_message() {
    time_t current_time = time(NULL);
    return current_time - s_last_app_alive_message_time;
}