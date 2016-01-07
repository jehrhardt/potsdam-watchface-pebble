#include <pebble.h>

static Window *window;
static TextLayer *s_time_layer;

static void update_time(struct tm *time) {
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", time);

  text_layer_set_text(s_time_layer, s_buffer);
}

static void timer_tick(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static void window_load(Window *window) {
  window_set_background_color(window, GColorDarkCandyAppleRed);
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  GRect text_bounds = GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50);
  s_time_layer = text_layer_create(text_bounds);

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
}

static void init(void) {
  window = window_create();

  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  window_stack_push(window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, timer_tick);
  time_t temp = time(NULL);
  struct tm *time = localtime(&temp);
  update_time(time);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
