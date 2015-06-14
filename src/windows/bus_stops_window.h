#pragma once

#include <pebble.h>
#include "../model/store.h"
#include "services_window.h"
#include "view_setup_helper.h"
#include "../controller/app_message_helper.h"
#include "../animations/loading_animation.h"

void bus_stops_window_push();
void bus_stops_window_reload_menu();