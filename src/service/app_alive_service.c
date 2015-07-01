// #define APP_ALIVE_SERVICE_REFRESH 300000 // 5 min (300000)
// #define LAST_APP_ALIVE_RESPONSE_THRESHOLD
// #include "app_alive_service.h"

// AppTimer *s_app_alive_service_timer;

// static void app_alive_callback() {
//     send_app_message_int(KEY_APP_ALIVE, 1);

//     if (get_time_since_last_app_alive_message() > APP_ALIVE_SERVICE_REFRESH/1000) {
//         window_stack_pop_all(false);
//         error_window_push((char*)"Lost connection with JS, reopen the app to continue");
//     } else {
//         s_app_alive_service_timer = app_timer_register(APP_ALIVE_SERVICE_REFRESH, app_alive_callback, NULL);
//     }
// }

// // starts service to send a message to the watch every 5 min
// void start_app_alive_service() {
//     send_app_message_int(KEY_APP_ALIVE, 1);
//     s_app_alive_service_timer = app_timer_register(APP_ALIVE_SERVICE_REFRESH, app_alive_callback, NULL);
// }

// void cancel_app_alive_service() {
//     app_timer_cancel(s_app_alive_service_timer);
// }
