#pragma once

#include <pebble.h>

char services_list[20][12];

char bus_stop_string_buffer[30][50];
char bus_stop_list[30][2][50];


void store_init();
void services_list_reset();
void bus_stop_list_reset();

void split_bus_stop_data();

int numberOfServices();
int numberOfBusStops();