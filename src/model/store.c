#include "store.h"

#define MESSAGE_DELIMITER ','

char s_bus_stop_string_buffer[30][50];
int s_bus_stop_list_counter = 0;

// Sets up default variables for store
void store_init() {
    // strcpy(bus_stops_list[0], "Stop 1");
}

void services_list_reset() {
    memset(services_list, 0, sizeof services_list);
}

void bus_stop_list_reset() {
    s_bus_stop_list_counter = 0;
    memset(s_bus_stop_string_buffer, 0, sizeof s_bus_stop_string_buffer);
    memset(bus_stop_list, 0, sizeof bus_stop_list);   
}

void bus_stop_list_add_string(char *string) {
    snprintf(s_bus_stop_string_buffer[s_bus_stop_list_counter], sizeof(s_bus_stop_string_buffer[s_bus_stop_list_counter]), "%s", string);
    s_bus_stop_list_counter++;
}

int numberOfServices() {
    int arraySize = sizeof(services_list) / sizeof(services_list[0]);
    int counter = 0;
    for (int index = 0; index < arraySize; index++) {
        char *currentElement = services_list[index];
        if ((int)strlen(currentElement) > 0) {
            counter++;
        }
    }
    return counter;
}

int get_bus_stop_list_count() {
    // int arraySize = sizeof(bus_stop_string_buffer) / sizeof(bus_stop_string_buffer[0]);
    // int counter = 0;
    // for (int index = 0; index < arraySize; index++) {
    //     char *currentElement = bus_stop_string_buffer[index];
    //     if ((int)strlen(currentElement) > 0) {
    //         counter++;
    //     }
    // }
    // return counter;
    return s_bus_stop_list_counter;
}

void split_bus_stop_data() {

    for (int j = 0; j < get_bus_stop_list_count(); j++) {
        char *currentBusStopString = s_bus_stop_string_buffer[j];
        
        
        int delimiters[NO_OF_BUS_STOP_LIST_MESSAGE_PARTS - 1];
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