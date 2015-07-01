#include "action_menu_window.h"


static Window *s_action_menu_window;
static SimpleMenuLayer *s_menu_layer;

static SimpleMenuItem s_menu_items[2];
static SimpleMenuSection s_menu_sections;

static void menu_layer_select_click(int index, void *context) {
    // set up notification here
    window_stack_pop(true);
}

static void set_up_menu_items() {

    s_menu_items[0] = (SimpleMenuItem) {
        .title = "Set Alert",
        .callback = menu_layer_select_click
    };

    s_menu_items[1] = (SimpleMenuItem) {
        .title = "Cancel Alert",
        .callback = menu_layer_select_click
    };

    s_menu_sections = (SimpleMenuSection) {
        .items = s_menu_items,
        .num_items = 2
    };
}

static void window_load(Window *window) {
    window_set_up(window);
    Layer *window_layer = window_get_root_layer(window);   
    GRect bounds = layer_get_bounds(window_layer);

    set_up_menu_items();
    s_menu_layer = simple_menu_layer_create(bounds, window, &s_menu_sections, 1, NULL);

    menu_layer_set_up(simple_menu_layer_get_menu_layer(s_menu_layer));
    layer_add_child(window_layer, simple_menu_layer_get_layer(s_menu_layer));
}

static void window_unload(Window *window) {

    simple_menu_layer_destroy(s_menu_layer);
    window_destroy(window);

}


void action_menu_window_push() {

    s_action_menu_window = window_create();
    window_set_window_handlers(s_action_menu_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    window_stack_push(s_action_menu_window, true);
}
