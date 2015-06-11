#include <pebble.h>

#include "model/store.h"
#include "windows/bus_stops_window.h"


static void init(void) {
    setUpStore();
    bus_stops_window_push();
}

static void deinit(void) {

}


int main(void) {
    init();

    app_event_loop();
    deinit();
}

