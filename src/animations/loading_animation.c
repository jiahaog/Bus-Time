#include "loading_animation.h"

#ifdef PBL_PLATFORM_BASALT

static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;

static void timer_handler(void *context) {
    uint32_t next_delay;
    
    if (gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay)) {
        bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
        layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));

        app_timer_register(next_delay, timer_handler, NULL);
    
    } else {
        gbitmap_sequence_restart(s_sequence);
    }

}


static void load_sequence() {
    if (s_sequence) {
        gbitmap_sequence_destroy(s_sequence);
        s_sequence = NULL;
    } 

    if (s_bitmap) {
        gbitmap_destroy(s_bitmap);
        s_bitmap = NULL;
    }


    s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIMATION_LOADING);
    s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

    app_timer_register(1, timer_handler, NULL);

}

void create_loading_animation(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_bitmap_layer = bitmap_layer_create(bounds);
    
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

    APP_LOG(APP_LOG_LEVEL_DEBUG, "created loading");

    load_sequence();
}


void destroy_loading_animation() {
    bitmap_layer_destroy(s_bitmap_layer);
}

#endif
