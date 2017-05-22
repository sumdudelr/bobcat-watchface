#include <pebble.h>

static Window *my_window;
static TextLayer *date_layer;
static TextLayer *time_layer;
static TextLayer *steps_layer;
static BitmapLayer *logo_layer;
static GBitmap *logo;
static GFont time_font;
static GRect bounds;

void update_time() {
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	static char time_buffer[8];
	strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	
	text_layer_set_text(time_layer, time_buffer);
}

void update_date() {
	time_t temp = time(NULL);
	struct tm *tick_date = localtime(&temp);
	
	static char date_buffer[9];
	strftime(date_buffer, sizeof(date_buffer), "%m-%d-%y", tick_date);
	
	text_layer_set_text(date_layer, date_buffer);
}

void update_steps() {
	static char steps_buffer[30] = "Steps: /0";
	unsigned short int steps_day = health_service_sum_today(HealthMetricStepCount);
	unsigned short int steps_avg = health_service_sum_averaged(HealthMetricStepCount, time_start_of_today(), time(NULL), HealthServiceTimeScopeDaily);
	snprintf(steps_buffer, 30, "Steps: %hu\nAvg: %hu", steps_day, steps_avg);
	text_layer_set_text(steps_layer, steps_buffer);
}

void window_update() {
	Layer *window_layer = window_get_root_layer(my_window);
	bounds = layer_get_unobstructed_bounds(window_layer);
	
	GRect date_rect = GRect(45, 0, bounds.size.w - 45, 40);
	layer_set_frame(text_layer_get_layer(date_layer), date_rect);
	text_layer_set_background_color(date_layer, GColorDarkGreen);
	text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_text(date_layer, " ");
	
		
	layer_set_frame(bitmap_layer_get_layer(logo_layer), GRect(0, 0, 45, 40));
	bitmap_layer_set_background_color(logo_layer, GColorDarkGreen);
	bitmap_layer_set_compositing_mode(logo_layer, GCompOpSet);
	bitmap_layer_set_bitmap(logo_layer, logo);
	
	
	layer_set_frame(text_layer_get_layer(time_layer), GRect(0, bounds.size.h / 2 - 25, bounds.size.w, bounds.size.h / 2 + 5));
	text_layer_set_background_color(time_layer, GColorClear);
	text_layer_set_text_color(time_layer, GColorDarkGreen);
	text_layer_set_font(time_layer, time_font);
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
	text_layer_set_text(time_layer, " ");
	
	
	layer_set_frame(text_layer_get_layer(steps_layer), GRect(0, bounds.size.h - 40, bounds.size.w, 40));
	text_layer_set_background_color(steps_layer, GColorDarkGreen);
	text_layer_set_text_color(steps_layer, GColorWhite);
	text_layer_set_font(steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(steps_layer, GTextAlignmentCenter);
	text_layer_set_text(steps_layer, " ");
	
	update_date();
	update_time();
	update_steps();
}

void unobstructed_change(AnimationProgress progress, void *context) {
	window_update();
}

void main_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	bounds = layer_get_unobstructed_bounds(window_layer);
	
	time_font = fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS);
	logo = gbitmap_create_with_resource(RESOURCE_ID_OHIO_LOGO_SHRUNK);
	
	date_layer = text_layer_create(GRect(45, 0, bounds.size.w - 45, 40));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
	logo_layer = bitmap_layer_create(GRect(0, 0, 45, 40));
	layer_add_child(window_layer, bitmap_layer_get_layer(logo_layer));
	time_layer = text_layer_create(GRect(0, bounds.size.h / 2 - 20, bounds.size.w, bounds.size.h / 2 + 20));
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	steps_layer = text_layer_create(GRect(0, bounds.size.h - 40, bounds.size.w, bounds.size.h - 100));
	layer_add_child(window_layer, text_layer_get_layer(steps_layer));
	
	UnobstructedAreaHandlers handlers = {
    	.change = window_update
	};
	unobstructed_area_service_subscribe(handlers, NULL);
	
	window_update();
}

void main_window_unload(Window *window) {
	text_layer_destroy(date_layer);
	bitmap_layer_destroy(logo_layer);
	text_layer_destroy(time_layer);
	text_layer_destroy(steps_layer);
	unobstructed_area_service_unsubscribe();
	gbitmap_destroy(logo);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	update_date();
	update_time();
	update_steps();
}

void handle_init(void) {
	my_window = window_create();
	window_set_background_color(my_window, GColorWhite);
	window_set_window_handlers(my_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	window_stack_push(my_window, true);
	update_date();
	update_time();
	update_steps();
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void handle_deinit(void) {
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
