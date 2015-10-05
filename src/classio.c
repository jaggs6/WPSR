#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_time_mode;
static Layer *s_canvas;

static int minute;
static int hour;

static int digit_to_int(char ch)
{
  return ch - '0';
}

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00:00";

  strftime(s_time_text, sizeof(s_time_text), "%T", tick_time);
  
  // night mode
  int textlen = strlen(s_time_text); /* possibly you've saved the length previously */
  minute = digit_to_int(s_time_text[textlen - 1]);
  hour = digit_to_int(s_time_text[textlen - 5])*10 + digit_to_int(s_time_text[textlen - 4]);
  
  // hour = (digit_to_int(s_time_text[textlen - 2])*10 + minute) % 24;
  
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "hour %d", hour);
  
  
  if(hour > 1 && hour < 9)
  { 
    text_layer_set_text(s_time_mode, "Sleep");
    s_time_text[textlen - 1] = 0;
    s_time_text[textlen - 2] = 0;
  }
  else if (hour == 9)
  {
    text_layer_set_text(s_time_mode, "Repeat");
  }
  else if (hour >= 10 && hour < 18)
  {
    text_layer_set_text(s_time_mode, "Work");
  }
  else
  {
    text_layer_set_text(s_time_mode, "Play");
    s_time_text[textlen - 1] = 0;
  }
  
  text_layer_set_text(s_time_layer, s_time_text);
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  hour = (22 + hour) % 8;
  if(hour < 3){
    graphics_context_set_fill_color(ctx, GColorDarkGreen);     
  }
  else if (hour < 6) {
    graphics_context_set_fill_color(ctx, GColorBlue);     
  }
  else if (hour < 7) {
    graphics_context_set_fill_color(ctx, GColorOrange);     
  }
  else { 
    graphics_context_set_fill_color(ctx, GColorRed);
  }
  
  int height = (hour * (bounds.size.h / 8));
  graphics_fill_rect(ctx, GRect(0, height, bounds.size.w, bounds.size.h), 0, 0);

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "height %d", height);

  //APP_LOG(APP_LOG_LEVEL_DEBUG, "Layer dirty");
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  s_time_layer = text_layer_create(GRect(10, 54, bounds.size.w, 114));
  s_time_mode = text_layer_create(GRect(((bounds.size.w * 1) / 4), ((bounds.size.h * 3) / 4), bounds.size.w, 114));
  
  text_layer_set_text_color(s_time_mode, GColorWhite);
  text_layer_set_background_color(s_time_mode, GColorClear);
  text_layer_set_font(s_time_mode, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
  
  s_canvas = layer_create(bounds);
  layer_set_update_proc(s_canvas, layer_update_proc);
  
  layer_add_child(window_layer, s_canvas);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_mode));
  
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_time_mode);
  layer_destroy(s_canvas);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
