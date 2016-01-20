#include <pebble.h>

static const GPathInfo HOUR_HANDLE_INFO = {
    .num_points = 4,
    .points = (GPoint []) {
        {-4, 0},
        {-4, -40},
        {4, -40},
        {4, 0}
    }
};

static const GPathInfo MINUTE_HANDLE_INFO = {
    .num_points = 4,
    .points = (GPoint []) {
        {-3, 0},
        {-3, -60},
        {3, -60},
        {3, 0}
    }
};

static const GPathInfo SECOND_HANDLE_INFO = {
    .num_points = 4,
    .points = (GPoint []) {
        {-2, 0},
        {-2, -60},
        {2, -60},
        {2, 0}
    }
};

static GPath *hour_handle;
static GPath *minute_handle;
static GPath *second_handle;

static Layer *time_layer;

static Window *window;

static void update_time_layer(Layer *layer, GContext *ctx) {
    graphics_context_set_fill_color(ctx, GColorPastelYellow);
    gpath_draw_filled(ctx, hour_handle);
    gpath_draw_filled(ctx, minute_handle);

    graphics_context_set_fill_color(ctx, GColorLimerick);
    gpath_draw_filled(ctx, second_handle);
}

static void update_handle_position(struct tm *t) {
    // analog watch does not show 24 hours
    int32_t hour = t->tm_hour % 12;

    // 12 hours have 720 minutes ⇒ 720 possible positions for hour handle
    int32_t hour_angle = TRIG_MAX_ANGLE * (hour * 60 + t->tm_min) / 720;
    gpath_rotate_to(hour_handle, hour_angle);

    // 60 possible positions for the minute handle
    int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
    gpath_rotate_to(minute_handle, minute_angle);

    // 60 possible positions for the second handle
    int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
    gpath_rotate_to(second_handle, second_angle);
}

static void timer_tick(struct tm *tick_time, TimeUnits units_changed) {
    update_handle_position(tick_time);
    layer_mark_dirty(time_layer);
}

static void window_load(Window *window) {
    window_set_background_color(window, GColorDarkCandyAppleRed);

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

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    GPoint center = grect_center_point(&bounds);

    hour_handle = gpath_create(&HOUR_HANDLE_INFO);
    gpath_move_to(hour_handle, center);

    minute_handle = gpath_create(&MINUTE_HANDLE_INFO);
    gpath_move_to(minute_handle, center);

    second_handle = gpath_create(&SECOND_HANDLE_INFO);
    gpath_move_to(second_handle, center);

    time_t now = time(NULL);
    struct tm *current_time = localtime(&now);
    update_handle_position(current_time);

    tick_timer_service_subscribe(SECOND_UNIT, timer_tick);
}

static void deinit(void) {
    tick_timer_service_unsubscribe();

    gpath_destroy(hour_handle);
    gpath_destroy(minute_handle);
    gpath_destroy(second_handle);

    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
