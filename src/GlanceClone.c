#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer;


static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;


// Updates Time and Date  
static void update_time_date() {

  // Pulls time from system 
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  static char s_time_text[] = "00:00";
  static char s_date_text[] = "XxxxxXxx 00";

  strftime(s_date_text, sizeof(s_date_text), "%a, %b %d", tick_time);
  text_layer_set_text(s_date_layer, s_date_text);

  char *time_format;
  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }
  strftime(s_time_text, sizeof(s_time_text), time_format, tick_time);

  if (!clock_is_24h_style() && (s_time_text[0] == '0')) {
    memmove(s_time_text, &s_time_text[1], sizeof(s_time_text) - 1);
  }
  text_layer_set_text(s_time_layer, s_time_text);
}


static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  //Background
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Time
  s_time_layer = text_layer_create(GRect(12, 111, 130, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(s_time_layer,GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Date
  s_date_layer = text_layer_create(GRect(50, 35, 90, 20));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  update_time_date();
}

static void main_window_unload(Window *window) {

  // Destroy Time
  text_layer_destroy(s_time_layer);

  // Destroy Date
  text_layer_destroy(s_date_layer);

  // Destroy Background Image
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);


}

static void minute_tick_handler(struct tm *tick_time, TimeUnits units_changer) {
  // Functions to run ever minute
  update_time_date();
 
}


static void init(void) {
  // Create main window
  s_main_window = window_create();
  window_set_fullscreen(s_main_window, true);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  
  window_stack_push(s_main_window, true);

  // subscibers
  tick_timer_service_subscribe(MINUTE_UNIT, minute_tick_handler);

}

static void deinit(void) {
  window_destroy(s_main_window);

  tick_timer_service_unsubscribe();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_main_window);

  app_event_loop();
  deinit();
}
