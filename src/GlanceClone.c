#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1 

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_weather_layer;

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

  // Weather
  s_weather_layer = text_layer_create(GRect(0, 12, 140, 30));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));

  // update the time and date on load
  update_time_date();
}

static void main_window_unload(Window *window) {
  // Destroy Time
  text_layer_destroy(s_time_layer);

  // Destroy Date
  text_layer_destroy(s_date_layer);

  // Destroy Weather
  text_layer_destroy(s_weather_layer);

  // Destroy Background Image
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);


}

static void minute_tick_handler(struct tm *tick_time, TimeUnits units_changer) {
  // Functions to run ever minute
  update_time_date();
 
  // Update Weather every 30 mins
  if(tick_time->tm_min % 30 == 0) {
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    dict_write_uint8(iter, 0, 0);
    app_message_outbox_send();
  }
 
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];

  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

  // Assemble string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Meesage Dropped");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed");
}

static void outbox_send_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success");
}


static void init(void) {
  // Create main window
  s_main_window = window_create();
  window_set_fullscreen(s_main_window, true);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  
  const bool animated = true;
  window_stack_push(s_main_window, animated);

  tick_timer_service_subscribe(MINUTE_UNIT, minute_tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_send_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
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
