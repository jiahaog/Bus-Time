#pragma once

#include <pebble.h>

// #include "../model/app_message_keys.c"
#include "../model/store.h"

#include "../windows/bus_stops_window.h"
#include "../windows/services_window.h"
#include "../windows/error_window.h"
#include "../service/observer.h"
#include "../service/app_alive_service.h"

#include "app_message_helper.h"

bool controller_init();
