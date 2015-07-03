#pragma once

#include <pebble.h>

#include "../controller/app_message_helper.h"
#include "../windows/window_index.h"

void watch_bus_services_list(char *bus_stop_id);
void watch_bus_service_details(char *service_no);
void watch_last_bus_service_details();

int watchingWhichWindow();
