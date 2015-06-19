#pragma once

#include <pebble.h>


enum {
    KEY_BUS_SERVICE_LIST_START = 0,
    KEY_BUS_SERVICE_LIST_VALUE = 1,
    KEY_BUS_SERVICE_LIST_END = 2,

    KEY_BUS_SERVICE_DETAILS_START = 3,
    KEY_BUS_SERVICE_DETAILS_VALUE = 4,
    KEY_BUS_SERVICE_DETAILS_END = 5,

    KEY_BUS_STOP_LIST_START = 6,
    KEY_BUS_STOP_LIST_VALUE = 7,
    KEY_BUS_STOP_LIST_END = 8,

    KEY_ERROR = 9,

    KEY_BUS_NOTIFICATION = 10,
    KEY_APP_ALIVE =  11
};


void send_app_message_string(int key, char *message);
void send_app_message_int(int key, int message);