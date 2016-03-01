#include <pebble.h>
#include "potsdam.h"

static GPath *ticks[NUM_TICKS];

static int32_t hour_angle;
static int32_t minute_angle;
static int32_t second_angle;

static Layer *time_layer;
static Layer *background_layer;

static Window *window;

static GPoint position(int32_t angle, int32_t length, GPoint shift) {
    return (GPoint) {
        .x = (sin_lookup(angle) * length / TRIG_MAX_RATIO) + shift.x,
        .y = (-cos_lookup(angle) * length / TRIG_MAX_RATIO) + shift.y
    };
}

static void draw_time_layer(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    GPoint hour_hand = position(hour_angle, 45, center);

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 6);
    graphics_draw_line(ctx, hour_hand, center);

    GPoint minute_hand = position(minute_angle, 65, center);

    graphics_context_set_stroke_width(ctx, 4);
    graphics_draw_line(ctx, minute_hand, center);

    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_fill_circle(ctx, center, 5);
}

static void draw_background_layer(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 1);

    for (int i = 0; i < NUM_TICKS; ++i) {
        gpath_draw_outline(ctx, ticks[i]);
    }
}

static void update_handle_position(struct tm *t) {
    // analog watch does not show 24 hours
    int32_t hour = t->tm_hour % 12;

    // 12 hours have 720 minutes ⇒ 720 possible positions for hour handle
    hour_angle = TRIG_MAX_ANGLE * (hour * 60 + t->tm_min) / 720;

    // 60 possible positions for the minute handle
    minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
}

static void timer_tick(struct tm *tick_time, TimeUnits units_changed) {
    update_handle_position(tick_time);
    layer_mark_dirty(time_layer);
}

static void window_load(Window *window) {
    window_set_background_color(window, GColorBlack);

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    background_layer = layer_create(bounds);
    layer_set_update_proc(background_layer, draw_background_layer);
    layer_add_child(window_layer, background_layer);

    time_layer = layer_create(bounds);
    layer_set_update_proc(time_layer, draw_time_layer);
    layer_add_child(window_layer, time_layer);
}

static void window_unload(Window *window) {
    layer_destroy(time_layer);
    layer_destroy(background_layer);
}

static void init(void) {
    window = window_create();

    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    window_stack_push(window, true);

    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    update_handle_position(current_time);

    for (int i = 0; i < NUM_TICKS; ++i) {
        ticks[i] = gpath_create(&BACKGROUND_TICKS[i]);
    }

    tick_timer_service_subscribe(MINUTE_UNIT, timer_tick);
}

static void deinit(void) {
    tick_timer_service_unsubscribe();

    for(int i = 0; i < NUM_TICKS; ++i) {
        gpath_destroy(ticks[i]);
    }

    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
