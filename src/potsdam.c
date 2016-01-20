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

static void timer_tick(struct tm *tick_time, TimeUnits units_changed) {
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
