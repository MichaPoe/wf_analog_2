#pragma once
#define WF_ANALOG_H

// #define DEBUG

#define SHOWHOURMARKS

// resolution: 144 × 168
#define HOURMARKCNT 12
#define HOURMARKDIAMETER 128
#define HOURMARKRADIUS 4
// dot in the middle
#define CENTERCIRCLEDIAMETER 8
#define CENTERHOLEDIAMETER 4
// handles
#define WITHNEEDLE
#define HOURHANDWIDTH 6
#define HOURHANDLENGTH 38
#define MINUTEHANDWIDTH 5
#define MINUTEHANDLENGTH 55

// date settings
#define DATEHSPACE 2
//#define DATEFONT FONT_KEY_GOTHAM_30_BLACK
//#define DATEFONTLOAD RESOURCE_ID_FONT_PORSCHE_CONDENSED_28
#define DATEFONTLOAD RESOURCE_ID_FONT_DIGITAL_30
#define DATEFONTSIZE 30

//#define SHOWMONTH
//#define DATEONTOP // geht derzeit nicht, weil die Zeiger dann zu tief sind
//#define DAY2DIGITS
//#define CENTERDATE
//#define CENTERDATEEACH

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a < _b ? _a : _b; })
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
      _a > _b ? _a : _b; })
#define sgn0(a) \
   ({ __typeof__ (a) _a = (a); \
      _a < 0 ? -1 : 1; })

static const char *GERMAN_DAYS[] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa" };

static const GPathInfo HOUR_HAND_POINTS = {
#ifdef WITHNEEDLE
	7,
#else
	5,
#endif
	(GPoint []) {
		{ -HOURHANDWIDTH,  0},
		{ -HOURHANDWIDTH, -HOURHANDLENGTH},
#ifdef WITHNEEDLE
		{             -1, -HOURHANDLENGTH - HOURHANDWIDTH / 2},
		{              1, -HOURHANDLENGTH - HOURHANDWIDTH / 2},
#else
#endif
		{  HOURHANDWIDTH, -HOURHANDLENGTH},
		{  HOURHANDWIDTH,  0},
		{ -HOURHANDWIDTH,  0}
	}
};

static const GPathInfo MINUTE_HAND_POINTS = {
#ifdef WITHNEEDLE
	7,
#else
	5,
#endif
	(GPoint []) {
		{ -MINUTEHANDWIDTH,  0},
		{ -MINUTEHANDWIDTH, -MINUTEHANDLENGTH},
#ifdef WITHNEEDLE
		{               -1, -MINUTEHANDLENGTH - MINUTEHANDWIDTH / 2},
		{                1, -MINUTEHANDLENGTH - MINUTEHANDWIDTH / 2},
#else
#endif
		{  MINUTEHANDWIDTH, -MINUTEHANDLENGTH},
		{  MINUTEHANDWIDTH,  0},
		{ -MINUTEHANDWIDTH,  0}
	}
};

