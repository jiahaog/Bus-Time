#pragma once

#include <pebble.h>
#include "view_setup_helper.h"
#include "../service/bus_arrival_notifier.h"

// used for PBL BASALT for action menu, in setting alerts for bus arrival

void open_alert_action_menu();
void init_alert_action_menu();
void unload_alert_action_menu();