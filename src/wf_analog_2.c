#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "resource_ids.auto.h"

#include "string.h"
#include "stdlib.h"

#include "wf_analog_2.h"

#define MY_UUID { 0xE5, 0x58, 0x71, 0x7B, 0xEE, 0xCD, 0x45, 0x1B, 0x82, 0x67, 0x57, 0xF8, 0xF5, 0xEF, 0x06, 0x8D }

/*
	History
	0.1 initial version
	0.2	added menu icon

	TODO
	- Inneren Layer für Datum und Wochentag.
 */

PBL_APP_INFO(MY_UUID,
	"Analog 2", "MichaPoe",
	0, 2, /* App version */
	RESOURCE_ID_IMAGE_MENU_ICON,
	APP_INFO_WATCH_FACE);

static struct SimpleAnalogData {
	Layer simple_bg_layer;
	Layer hourmarks_layer;
	GPoint hourmarks[HOURMARKCNT];

	Layer hands_layer;
	GPath hour_arrow;
	GPath minute_arrow;

	Layer date_layer;
	TextLayer weekday_label;
	TextLayer daynum_label;
	char daynum_buffer[7];

	Window window;
} s_data;

static GPoint computemark(int16_t diameter, int16_t width, int16_t height, int16_t cnt, int16_t n) {
	int16_t radius = diameter >> 1;
	int16_t centerx = (width - diameter) / 2;
	int16_t centery = (height - diameter) / 2;
	int16_t t = (cnt >> 2) - n;
	if (t < 0) t += cnt;
	int16_t angle = TRIG_MAX_ANGLE*t/cnt;
	int16_t x = (int16_t)( sin_lookup(angle) * (int32_t)radius / TRIG_MAX_RATIO) + radius + centerx;
	int16_t y = (int16_t)(-cos_lookup(angle) * (int32_t)radius / TRIG_MAX_RATIO) + radius + centery;

	return GPoint(x, y);
}

static void buildmarks(GPoint marks[], int16_t width, int16_t height, int16_t markdiameter, int16_t markradius, int16_t cnt) {
	for (int16_t n = 0; n < cnt; ++n) {
		marks[n] = computemark(markdiameter, width, height, cnt, n);
	}
}

static void bg_update_proc(Layer* me, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, me->bounds, 0, GCornerNone);
}

static void hourmarks_update_proc(Layer* me, GContext* ctx) {
	graphics_context_set_fill_color(ctx, GColorBlack);
	graphics_fill_rect(ctx, me->bounds, 0, GCornerNone);

#ifdef SHOWHOURMARKS
	graphics_context_set_fill_color(ctx, GColorWhite);
	for (int h = 0; h < HOURMARKCNT; ++h) {
		graphics_fill_circle(ctx, s_data.hourmarks[h], HOURMARKRADIUS);
	}
#endif
}

static void hands_update_proc_arrow(Layer* me, GContext* ctx) {
	PblTm t;
	get_time(&t);

	// minute/hour hand
	graphics_context_set_fill_color(ctx, GColorWhite);
	graphics_context_set_stroke_color(ctx, GColorBlack);

#ifdef DEBUG
	int time = t.tm_min * 60 + t.tm_sec;
	int h = time / 60;
	int m = time % 60;
#else
	int h = t.tm_hour;
	int m = t.tm_min;
#endif

	// hour hand
	gpath_rotate_to(&s_data.hour_arrow, (TRIG_MAX_ANGLE * (((h % 12) * 6) + (m / 10))) / (12 * 6));
	gpath_draw_filled(ctx, &s_data.hour_arrow);
	gpath_draw_outline(ctx, &s_data.hour_arrow);

	const GPoint center = grect_center_point(&me->bounds);

	// border of dot in the middle
	graphics_context_set_fill_color(ctx, GColorWhite);
	//graphics_draw_circle(ctx, GPoint(me->bounds.size.w / 2, me->bounds.size.h / 2), CENTERCIRCLEDIAMETER + 2);
	graphics_draw_circle(ctx, center, CENTERCIRCLEDIAMETER + 1);

	// minute hand
	gpath_rotate_to(&s_data.minute_arrow, TRIG_MAX_ANGLE * m / 60);
	gpath_draw_filled(ctx, &s_data.minute_arrow);
	gpath_draw_outline(ctx, &s_data.minute_arrow);

	// dot in the middle
	//graphics_fill_circle(ctx, GPoint(me->bounds.size.w / 2, me->bounds.size.h / 2), CENTERCIRCLEDIAMETER);
	graphics_fill_circle(ctx, center, CENTERCIRCLEDIAMETER);

	// hole in the middle
	graphics_context_set_fill_color(ctx, GColorBlack);
	//graphics_fill_circle(ctx, GPoint(me->bounds.size.w / 2, me->bounds.size.h / 2), CENTERHOLEDIAMETER);
	graphics_fill_circle(ctx, center, CENTERHOLEDIAMETER);
}

static void date_update_proc(Layer* me, GContext* ctx) {
	(void) me;
	(void) ctx;

	PblTm t;
	get_time(&t);

	text_layer_set_text(&s_data.weekday_label, GERMAN_DAYS[t.tm_wday]);

#ifdef SHOWMONTH
	string_format_time(s_data.daynum_buffer, sizeof(s_data.daynum_buffer), "%d.%m.", &t);
#else
	#ifdef DAY2DIGITS
		string_format_time(s_data.daynum_buffer, sizeof(s_data.daynum_buffer), "%d", &t);
	#else
		string_format_time(s_data.daynum_buffer, sizeof(s_data.daynum_buffer), "%e", &t);
	#endif
#endif
	text_layer_set_text(&s_data.daynum_label, s_data.daynum_buffer);
	//%V
}

void handle_init(AppContextRef ctx) {
	(void)ctx;

	window_init(&s_data.window, "MichaPoe analog #2");
	window_stack_push(&s_data.window, true);
	// If you neglect to call this, all `resource_get_handle()` requests will return NULL.
	resource_init_current_app(&WF_ANALOG_2);
	
	// Fonts
#ifdef DATEFONT
	GFont font = fonts_get_system_font(DATEFONT);
#else
	#ifdef DATEFONTLOAD
		GFont font = fonts_load_custom_font(resource_get_handle(DATEFONTLOAD));
	#endif
#endif

	// resolution: 144 × 168
	int16_t hourmarkheight = HOURMARKDIAMETER + HOURMARKRADIUS * 2 + 2;
	int16_t hourmarkwidth = hourmarkheight;
	int16_t hourmarkwidthspace = 144 - hourmarkwidth;
	int16_t datelayerwidth = 144 - DATEHSPACE * 2;

	// init layers
	layer_init(&s_data.simple_bg_layer, s_data.window.layer.frame);
	s_data.simple_bg_layer.update_proc = &bg_update_proc;
	layer_add_child(&s_data.window.layer, &s_data.simple_bg_layer);

#ifdef DATEONTOP
	GRect hourmarkshandrect = GRect(2, 168 - hourmarkheight, 140, hourmarkheight);
#else
	GRect hourmarkshandrect = GRect(2, 0, 140, hourmarkheight);
#endif

	layer_init(&s_data.hourmarks_layer, s_data.simple_bg_layer.frame);
	layer_set_bounds(&s_data.hourmarks_layer, hourmarkshandrect);
	s_data.hourmarks_layer.update_proc = &hourmarks_update_proc;
	layer_add_child(&s_data.window.layer, &s_data.hourmarks_layer);

	layer_init(&s_data.hands_layer, s_data.simple_bg_layer.frame);
	layer_set_bounds(&s_data.hands_layer, hourmarkshandrect);
	s_data.hands_layer.update_proc = &hands_update_proc_arrow;
	layer_add_child(&s_data.window.layer, &s_data.hands_layer);

	// date_layer
#ifdef DATEONTOP
	GRect daterect = GRect(0, 0, datelayerwidth, DATEFONTSIZE);
#else
	GRect daterect = GRect(0, 168 - DATEFONTSIZE, datelayerwidth, DATEFONTSIZE);
#endif
	layer_init(&s_data.date_layer, daterect);
	s_data.date_layer.update_proc = &date_update_proc;
	layer_add_child(&s_data.window.layer, &s_data.date_layer);

	// buffers
	s_data.daynum_buffer[0] = '\0';

	// weekday_layer
	text_layer_init(&s_data.weekday_label, GRect(0, 0, datelayerwidth / 2, DATEFONTSIZE));
	text_layer_set_text(&s_data.weekday_label, "\0");
	text_layer_set_background_color(&s_data.weekday_label, GColorBlack);
	text_layer_set_text_color(&s_data.weekday_label, GColorWhite);
#ifdef CENTERDATE
	text_layer_set_text_alignment(&s_data.weekday_label, GTextAlignmentRight);
#else
	#ifdef CENTERDATEEACH
		text_layer_set_text_alignment(&s_data.weekday_label, GTextAlignmentCenter);
	#endif
#endif
	text_layer_set_font(&s_data.weekday_label, font);
	layer_add_child(&s_data.date_layer, &s_data.weekday_label.layer);

	// daynum_layer
	text_layer_init(&s_data.daynum_label, GRect(datelayerwidth / 2, 0, datelayerwidth / 2, DATEFONTSIZE));
	text_layer_set_text(&s_data.daynum_label, s_data.daynum_buffer);
	text_layer_set_background_color(&s_data.daynum_label, GColorBlack);
	text_layer_set_text_color(&s_data.daynum_label, GColorWhite);
#ifdef CENTERDATE
#else
	#ifdef CENTERDATEEACH
		text_layer_set_text_alignment(&s_data.daynum_label, GTextAlignmentCenter);
	#else
		text_layer_set_text_alignment(&s_data.daynum_label, GTextAlignmentRight);
	#endif
#endif
	text_layer_set_font(&s_data.daynum_label, font);
	layer_add_child(&s_data.date_layer, &s_data.daynum_label.layer);

	// compute hourmarks
	buildmarks(s_data.hourmarks, 144, hourmarkheight, HOURMARKDIAMETER, HOURMARKRADIUS, HOURMARKCNT);

	// init hand paths
	gpath_init(&s_data.hour_arrow, &HOUR_HAND_POINTS);
	gpath_init(&s_data.minute_arrow, &MINUTE_HAND_POINTS);
	
	const GPoint center = grect_center_point(&s_data.hands_layer.bounds);
	gpath_move_to(&s_data.hour_arrow, center);
	gpath_move_to(&s_data.minute_arrow, center);
}

static void handle_minute_tick(AppContextRef ctx, PebbleTickEvent* t) {
	(void) t;
	layer_mark_dirty(&s_data.hands_layer);
	layer_mark_dirty(&s_data.date_layer);
}

void pbl_main(void *params) {
	PebbleAppHandlers handlers = {
		.init_handler = &handle_init,
		.tick_info = {
			.tick_handler = &handle_minute_tick,
#ifdef DEBUG
			.tick_units = SECOND_UNIT
#else
			.tick_units = SECOND_UNIT
#endif
		}
	};
	app_event_loop(params, &handlers);
}
