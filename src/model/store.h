#pragma once

#include <pebble.h>


char services_list[20][12];
char bus_stops_list[37][30];

void setUpStore();
int numberOfServices();
int numberOfBusStops();