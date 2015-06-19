#pragma once

#include <pebble.h>

#include "../controller/app_message_helper.h"
#include "../model/store.h"
#include "../windows/error_window.h"

void start_app_alive_service();
void cancel_app_alive_service();
