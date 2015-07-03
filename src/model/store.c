#include "store.h"

char s_bus_stop_string_buffer[BUS_STOP_LIST_MAX_ELEMENTS][STRING_SIZE];
char s_service_list_string_buffer[SERVICE_LIST_MAX_ELEMENTS][STRING_SIZE];
char s_details_list_string_buffer[STRING_SIZE];

int s_bus_stop_list_counter = 0;
int s_service_list_counter = 0;

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