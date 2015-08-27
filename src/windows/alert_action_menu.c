#include "alert_action_menu.h"

#ifdef PBL_PLATFORM_BASALT

    static ActionMenu *s_action_menu;
    static ActionMenuLevel *s_root_level;

    static void action_performed_callback(ActionMenu *action_menu, const ActionMenuItem *action, void *context) {
        int whatToDo = (int)action_menu_item_get_action_data(action);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "VAL: %i", whatToDo);

        if (whatToDo == 0) {
            start_notification_service();
        } else if (whatToDo == 1) {
            cancel_notification_service();
        } else {
            APP_LOG(APP_LOG_LEVEL_ERROR, "ERROR: Invalid action menu number");
        }
    }

    void open_alert_action_menu() {

        ActionMenuConfig config = (ActionMenuConfig) {
          .root_level = s_root_level,
          .colors = {
            .background = COLOR_BACKGROUND,
            .foreground = GColorBlack,
          },
          .align = ActionMenuAlignCenter
        };

        s_action_menu = action_menu_open(&config);
    }

    void init_alert_action_menu() {

        s_root_level = action_menu_level_create(2);

        action_menu_level_add_action(s_root_level, "Set Alert", action_performed_callback, (void *)0);
        action_menu_level_add_action(s_root_level, "Cancel Alert", action_performed_callback, (void *)1);
    }

    void unload_alert_action_menu() {
        action_menu_hierarchy_destroy(s_root_level, NULL, NULL);
    }

#endif

