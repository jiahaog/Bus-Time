#pragma once

#include <pebble.h>

#include "../model/store.h"

#include "../windows/bus_stops_window.h"
#include "../windows/services_window.h"
#include "../windows/error_window.h"
#include "../service/observer.h"

#include "app_message_helper.h"

bool controller_init();
