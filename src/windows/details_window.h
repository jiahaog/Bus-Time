#pragma once

#include <pebble.h>
#include "view_setup_helper.h"
#include "../animations/loading_animation.h"
#include "../controller/app_message_helper.h"
#include "../model/store.h"
#include "../service/observer.h"

#ifdef PBL_PLATFORM_APLITE
    #include "windows/action_menu_window.h"
#else
    #include "windows/alert_action_menu.h"
#endif

void details_window_push(char *stop_id, char *current_service);
void details_window_set_text(char *message);
void details_window_reload_details();
void hide_alert_icon();

