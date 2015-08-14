#pragma once
#include <pebble.h>

#include "store.h"
#define NOTIFICATION_STORE_STRING_SIZE 16
#define NOTIFICATION_APP_MESSAGE_PARTS 2



void notification_store_add(char *stop_id, char *service_no, AppTimer *timer);
void notification_store_remove(char *stop_id, char *service_no);
AppTimer * notification_store_get(char *stop_id, char *service_no);
bool notification_exists(char *stop_id, char *service_no);
void print_notification_store();
void cancel_notification_timer_from_message(char *message);