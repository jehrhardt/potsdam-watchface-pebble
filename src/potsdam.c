#include <pebble.h>

static Window *window;

static void timer_tick(struct tm *tick_time, TimeUnits units_changed) {
}

static void window_load(Window *window) {
    window_set_background_color(window, GColorDarkCandyAppleRed);
}

static void window_unload(Window *window) {
}

static void init(void) {
    window = window_create();

    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });

    window_stack_push(window, true);
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
