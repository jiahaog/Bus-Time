#pragma once
#include <pebble.h>

#define NOTIFICATION_STORE_STRING_SIZE 16


void notification_store_add(char *stop_id, char *service_no, AppTimer *timer);
void notification_store_remove(char *stop_id, char *service_no);
AppTimer *notification_store_get(char *stop_id, char *service_no);
void print_notification_store();
