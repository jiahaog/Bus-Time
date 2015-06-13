#pragma once
#define NO_OF_BUS_STOP_LIST_MESSAGE_PARTS 3

#include <pebble.h>


char services_list[20][12];


char bus_stop_list[30][NO_OF_BUS_STOP_LIST_MESSAGE_PARTS][50];


void store_init();
void services_list_reset();
void bus_stop_list_reset();

void bus_stop_list_add_string(char *string);
int get_bus_stop_list_count();

void split_bus_stop_data();



int numberOfServices();
int numberOfBusStops();