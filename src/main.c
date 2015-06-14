#include <pebble.h>

#include "model/store.h"
#include "windows/bus_stops_window.h"
#include "controller/controller.h"

static void init(void) {
    store_init();
    if (controller_init()) {
        bus_stops_window_push();
    }
}

static void deinit(void) {

} 

int main(void) {
    init();

    app_event_loop();
    deinit();
}
