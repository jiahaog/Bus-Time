#pragma once

#include <pebble.h>


char services_list[20][12];
char bus_stops_list[37][30];

void store_init();
void services_list_reset();
void bus_stop_list_reset();

int numberOfServices();
int numberOfBusStops();