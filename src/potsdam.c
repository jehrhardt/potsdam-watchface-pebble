#include <pebble.h>

int32_t hour_angle;
int32_t minute_angle;
int32_t second_angle;

static Layer *time_layer;

static Window *window;

static void update_time_layer(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    GPoint center = grect_center_point(&bounds);

    GPoint hour_hand = {
        .x = (sin_lookup(hour_angle) * 40 / TRIG_MAX_RATIO) + center.x,
        .y = (-cos_lookup(hour_angle) * 40 / TRIG_MAX_RATIO) + center.y
    };

    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_stroke_width(ctx, 6);
    graphics_draw_line(ctx, hour_hand, center);

    GPoint minute_hand = {
        .x = (sin_lookup(minute_angle) * 60 / TRIG_MAX_RATIO) + center.x,
        .y = (-cos_lookup(minute_angle) * 60 / TRIG_MAX_RATIO) + center.y
    };

    graphics_context_set_stroke_width(ctx, 4);
    graphics_draw_line(ctx, minute_hand, center);

    GPoint second_hand = {
        .x = (sin_lookup(second_angle) * 65 / TRIG_MAX_RATIO) + center.x,
        .y = (-cos_lookup(second_angle) * 65 / TRIG_MAX_RATIO) + center.y
    };

    graphics_context_set_fill_color(ctx, GColorRed);
    graphics_fill_circle(ctx, second_hand, 4);
}

static void update_handle_position(struct tm *t) {
    // analog watch does not show 24 hours
    int32_t hour = t->tm_hour % 12;

    // 12 hours have 720 minutes ⇒ 720 possible positions for hour handle
    hour_angle = TRIG_MAX_ANGLE * (hour * 60 + t->tm_min) / 720;

    // 60 possible positions for the minute handle
    minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;

    // 60 possible positions for the second handle
    second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
}

static void timer_tick(struct tm *tick_time, TimeUnits units_changed) {
    update_handle_position(tick_time);
    layer_mark_dirty(time_layer);
}

static void window_load(Window *window) {
    window_set_background_color(window, GColorBlack);

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    time_layer = layer_create(bounds);
    layer_set_update_proc(time_layer, update_time_layer);
    layer_add_child(window_layer, time_layer);
}

static void window_unload(Window *window) {
    layer_destroy(time_layer);
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

    tick_timer_service_subscribe(SECOND_UNIT, timer_tick);
}

static void deinit(void) {
    tick_timer_service_unsubscribe();

    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
