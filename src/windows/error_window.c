#include "error_window.h"

#define ERROR_MESSAGE_SIZE 30

static Window *s_error_window;
static TextLayer *s_error_text_layer;

static BitmapLayer *s_warning_layer;
static GBitmap *s_warning_bitmap;


char s_error_message[ERROR_MESSAGE_SIZE] = "DEFAULT ERROR MESSAGE";


// static void error_window_click_handler(ClickRecognizerRef recognizer, void *context) {
//     // go back to previous window
    
// }

// static void click_config_provider(void *context) {
//   // Register the ClickHandlers
//   window_single_click_subscribe(BUTTON_ID_UP, error_window_click_handler);
//   window_single_click_subscribe(BUTTON_ID_SELECT, error_window_click_handler);
//   window_single_click_subscribe(BUTTON_ID_DOWN, error_window_click_handler);
// }

static void window_load(Window *window) {
    window_set_up(window);
    // window_set_click_config_provider(s_error_window, click_config_provider);
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_warning_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WARNING);
    GRect bitmap_bounds = gbitmap_get_bounds(s_warning_bitmap);

    s_warning_layer = bitmap_layer_create(GRect(10, 10, bitmap_bounds.size.w, bitmap_bounds.size.h));
    bitmap_layer_set_bitmap(s_warning_layer, s_warning_bitmap);

    #ifdef PBL_PLATFORM_APLITE
        bitmap_layer_set_compositing_mode(s_warning_layer, GCompOpAssignInverted);
    #else
        bitmap_layer_set_compositing_mode(s_warning_layer, GCompOpSet);
    #endif

    layer_add_child(window_layer, bitmap_layer_get_layer(s_warning_layer));

    // Create and Add to layer hierarchy:
    s_error_text_layer = text_layer_create(GRect(10, 10 + bitmap_bounds.size.h + 5, 124, 168 - (10 + bitmap_bounds.size.h + 10)));
    text_layer_set_up(s_error_text_layer);
    text_layer_set_font(s_error_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_error_text_layer, GTextAlignmentLeft);
    text_layer_set_text(s_error_text_layer, s_error_message);

    #ifdef PBL_COLOR
        window_set_background_color(window, GColorRed);
    #endif

    layer_add_child(window_layer, text_layer_get_layer(s_error_text_layer));

    vibes_short_pulse();
}

static void window_unload(Window *window) {
    bitmap_layer_destroy(s_warning_layer);
    gbitmap_destroy(s_warning_bitmap);
    text_layer_destroy(s_error_text_layer);
    window_destroy(window);

    s_error_window = NULL;
}

void error_window_push(char *message) {

    snprintf(s_error_message, sizeof(s_error_message), "%s", message);
    
    if (!s_error_window) {
        s_error_window = window_create();
        window_set_window_handlers(s_error_window, (WindowHandlers) {
            .load = window_load,
            .unload = window_unload,
        });
    }

    
    window_stack_push(s_error_window, true);

}
