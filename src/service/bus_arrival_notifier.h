#pragma once

#include <pebble.h>
#include "../controller/app_message_helper.h"
#include "../model/store.h"
#include "../model/notification_store.h"


void start_notification_service();
void cancel_notification_service();