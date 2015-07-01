#pragma once

#include <pebble.h>
#include "../model/store.h"
#include "details_window.h"
#include "view_setup_helper.h"
#include "../controller/app_message_helper.h"
#include "../animations/loading_animation.h"
#include "../service/observer.h"

void services_window_push(char *bus_stop_name, char *bus_stop_id);
void services_window_reload_menu();