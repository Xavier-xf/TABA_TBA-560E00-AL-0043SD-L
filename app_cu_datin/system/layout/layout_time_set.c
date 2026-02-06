#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"

static int get_year_max(void);
static int get_year_min(void);
static unsigned char get_current_month_max_day(int month);

#define YEAR_MAX get_year_max()
#define YEAR_MIN get_year_min()
#define MONTH_MAX 11
#define MONTH_MIN 0
#define DAY_MAX get_current_month_max_day(TimeSetClass.date.month) - 1
#define DAY_MIN 0
#define HOUR_MAX 23
#define HOUR_MIN 0
#define MINUTE_MAX 59
#define MINUTE_MIN 0

static int get_year_max(void)
{
	if (language_get() == language_english)
	{
		return 2037;
	}
	else
	{
		return 1415;
	}
}

static int get_year_min(void)
{
	if (language_get() == language_english)
	{
		return 1980;
	}
	else
	{
		return 1359;
	}
}

static bool public_bool_IsLeap(int i_year) // 计算波斯历的闰年
{
	int mod;
	mod = i_year % 33;
	if (i_year <= 1472 && i_year >= 1343)
	{
		if (mod == 1 || mod == 5 || mod == 9 || mod == 13 || mod == 17 || mod == 22 || mod == 26 || mod == 23)
			return true;
		else
			return false;
	}
	if (i_year <= 1342 && i_year >= 1244)
	{
		if (mod == 1 || mod == 5 || mod == 9 || mod == 13 || mod == 17 || mod == 22 || mod == 26 || mod == 23)
			return true;
		else
			return false;
	}
	return false;
}

static unsigned char get_current_month_max_day(int month)
{

	if (language_get() == language_english)
	{
		unsigned char leap_year[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		unsigned char nonleap_year[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		if (((TimeSetClass.date.year % 4 == 0) && (TimeSetClass.date.year % 100 != 0)) || TimeSetClass.date.year % 400 == 0)
		{
			return leap_year[month];
		}
		else
		{
			return nonleap_year[month];
		}
	}
	else
	{
		unsigned char leap_year[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 30};
		unsigned char nonleap_year[12] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 30, 30, 29};
		if (public_bool_IsLeap(TimeSetClass.date.year))
		{
			return leap_year[month];
		}
		else
		{
			return nonleap_year[month];
		}
	}
}

static void time_set_year_bracket_icon_display(void)
{
	icon left_icon;
	position left_pos = {{278, 23}, {12, 23}};
	gui_erase(&left_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == YEAR_SET_FOCUS)
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_FOCUS_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	else
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_ICON_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	icon_display(&left_icon);

	icon right_icon;
	position right_pos = {{363, 23}, {12, 23}};
	gui_erase(&right_pos, 0x00);

	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == YEAR_SET_FOCUS)
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_FOCUS_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	else
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_ICON_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	icon_display(&right_icon);
}

static void time_set_month_bracket_icon_display(void)
{
	icon left_icon;
	position left_pos = {{278, 59}, {12, 23}};
	gui_erase(&left_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == MONTH_SET_FOCUS)
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_FOCUS_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	else
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_ICON_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	icon_display(&left_icon);

	icon right_icon;
	position right_pos = {{363, 59}, {12, 23}};
	gui_erase(&right_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == MONTH_SET_FOCUS)
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_FOCUS_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	else
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_ICON_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	icon_display(&right_icon);
}

static void time_set_day_bracket_icon_display(void)
{
	icon left_icon;
	position left_pos = {{278, 97}, {12, 23}};
	gui_erase(&left_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == DAY_SET_FOCUS)
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_FOCUS_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	else
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_ICON_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	icon_display(&left_icon);

	icon right_icon;
	position right_pos = {{363, 97}, {12, 23}};
	gui_erase(&right_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == DAY_SET_FOCUS)
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_FOCUS_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	else
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_ICON_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	icon_display(&right_icon);
}

static void time_set_hour_bracket_icon_display(void)
{
	icon left_icon;
	position left_pos = {{278, 133}, {12, 23}};
	gui_erase(&left_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == HOUR_SET_FOCUS)
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_FOCUS_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	else
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_ICON_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	icon_display(&left_icon);

	icon right_icon;
	position right_pos = {{363, 133}, {12, 23}};
	gui_erase(&right_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == HOUR_SET_FOCUS)
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_FOCUS_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	else
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_ICON_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	icon_display(&right_icon);
}

static void time_set_minute_bracket_icon_display(void)
{
	icon left_icon;
	position left_pos = {{278, 168}, {12, 23}};
	gui_erase(&left_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == MINUTE_SET_FOCUS)
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_FOCUS_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	else
	{
		resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LEFT_ICON_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	icon_display(&left_icon);

	icon right_icon;
	position right_pos = {{363, 168}, {12, 23}};
	gui_erase(&right_pos, 0x00);
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && TimeSetClass.cur_focus.main == MINUTE_SET_FOCUS)
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_FOCUS_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	else
	{
		resource right_res = resource_get(ROM_R_IMG_SYSTEM_SET_RIGHT_ICON_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	icon_display(&right_icon);
}

static void time_set_year_font_display(void)
{
	position pos = {{86, 20}, {100, 50}};
	text time_set;

	text_init(&time_set, &pos, 20);
	time_set.align = LEFT_TOP;
	text_display(&time_set, font_str(STR_TIME_SET_YEAR));
}

static void time_set_month_font_display(void)
{
	position pos = {{86, 57}, {100, 50}};
	text time_set;

	text_init(&time_set, &pos, 20);
	time_set.align = LEFT_TOP;
	text_display(&time_set, font_str(STR_TIME_SET_MONTH));
}

static void time_set_day_font_display(void)
{
	position pos = {{86, 95}, {100, 50}};
	text time_set;

	text_init(&time_set, &pos, 20);
	time_set.align = LEFT_TOP;
	text_display(&time_set, font_str(STR_TIME_SET_DAY));
}

static void time_set_hour_font_display(void)
{
	position pos = {{86, 131}, {100, 50}};
	text time_set;

	text_init(&time_set, &pos, 20);
	time_set.align = LEFT_TOP;
	text_display(&time_set, font_str(STR_TIME_SET_HOUR));
}

static void time_set_minute_font_display(void)
{
	position pos = {{86, 162}, {100, 50}};
	text time_set;

	text_init(&time_set, &pos, 20);
	time_set.align = LEFT_TOP;
	text_display(&time_set, font_str(STR_TIME_SET_MINUTE));
}

static void time_set_year_data_font_display(void)
{
	position pos = {{290, 12}, {70, 50}};
	text text_date;
	char string[5];

	gui_erase(&pos, 0x00000000);
	text_init(&text_date, &pos, 20);
	text_date.align = CENTER_MIDDLE;
	sprintf(string, "%04d", TimeSetClass.date.year);
	text_display(&text_date, string);
}

static void time_set_month_data_font_display(void)
{
	position pos = {{290, 50}, {70, 50}};
	text text_date;
	char string[3];

	gui_erase(&pos, 0x00000000);
	text_init(&text_date, &pos, 20);
	text_date.align = CENTER_MIDDLE;
	sprintf(string, "%02d", TimeSetClass.date.month + 1);
	text_display(&text_date, string);
}

static void time_set_day_data_font_display(void)
{
	position pos = {{290, 87}, {70, 50}};
	text text_date;
	char string[3];

	gui_erase(&pos, 0x00000000);
	text_init(&text_date, &pos, 20);
	text_date.align = CENTER_MIDDLE;
	sprintf(string, "%02d", TimeSetClass.date.day + 1);
	text_display(&text_date, string);
}

static void time_set_hour_data_font_display(void)
{
	position pos = {{290, 125}, {70, 50}};
	text text_date;
	char string[3];

	gui_erase(&pos, 0x00000000);
	text_init(&text_date, &pos, 20);
	text_date.align = CENTER_MIDDLE;
	sprintf(string, "%02d", TimeSetClass.date.hour);
	text_display(&text_date, string);
}

static void time_set_minute_data_font_display(void)
{

	position pos = {{290, 168}, {70, 30}};
	text text_date;
	char string[3];

	gui_erase(&pos, 0x00000000);
	text_init(&text_date, &pos, 20);
	text_date.align = CENTER_MIDDLE;
	sprintf(string, "%02d", TimeSetClass.date.minute);
	text_display(&text_date, string);
}

static void time_set_icon_display(void)
{
	taba_btn_display();
	time_set_year_bracket_icon_display();
	time_set_month_bracket_icon_display();
	time_set_day_bracket_icon_display();
	time_set_hour_bracket_icon_display();
	time_set_minute_bracket_icon_display();
}

static void time_set_date_font_display(void)
{
	time_set_year_data_font_display();
	time_set_month_data_font_display();
	time_set_day_data_font_display();
	time_set_hour_data_font_display();
	time_set_minute_data_font_display();
}

static void time_set_font_display(void)
{
	time_set_year_font_display();
	time_set_month_font_display();
	time_set_day_font_display();
	time_set_hour_font_display();
	time_set_minute_font_display();

	time_set_date_font_display();
}

static void clear_prev_time_set_focus(void)
{

	position pos = {{382, 18}, {32, 32}};
	pos.point.y = 18 + 37 * TimeSetClass.cur_focus.main;
	gui_erase(&pos, 0x00);
}

static void display_current_time_set_focus(void)
{
	icon focus;
	position pos = {{382, 18}, {32, 32}};
	if (TimeSetClass.cur_focus.layer == TIME_SET_MAIN_LAYER)
	{
		pos.point.y = 18 + 37 * TimeSetClass.cur_focus.main;
		resource res = resource_get(ROM_R_IMG_SET_PAGE_FOCUS_PNG);
		icon_init(&focus, &pos, &res);
		icon_display(&focus);
	}
	TimeSetClass.widget_show.icon();
}

static void time_set_focus_display(void)
{

	clear_prev_time_set_focus();
	display_current_time_set_focus();
}

static void goto_prev_time_set_focus(void)
{

	(TimeSetClass.cur_focus.main == YEAR_SET_FOCUS) ? (TimeSetClass.cur_focus.main = MINUTE_SET_FOCUS) : (TimeSetClass.cur_focus.main == MONTH_SET_FOCUS) ? (TimeSetClass.cur_focus.main = YEAR_SET_FOCUS)
																									 : (TimeSetClass.cur_focus.main == DAY_SET_FOCUS)	  ? (TimeSetClass.cur_focus.main = MONTH_SET_FOCUS)
																									 : (TimeSetClass.cur_focus.main == HOUR_SET_FOCUS)	  ? (TimeSetClass.cur_focus.main = DAY_SET_FOCUS)
																																						  : (TimeSetClass.cur_focus.main = HOUR_SET_FOCUS);
}

static void goto_next_time_set_focus(void)
{

	(TimeSetClass.cur_focus.main == YEAR_SET_FOCUS) ? (TimeSetClass.cur_focus.main = MONTH_SET_FOCUS) : (TimeSetClass.cur_focus.main == MONTH_SET_FOCUS) ? (TimeSetClass.cur_focus.main = DAY_SET_FOCUS)
																									: (TimeSetClass.cur_focus.main == DAY_SET_FOCUS)	 ? (TimeSetClass.cur_focus.main = HOUR_SET_FOCUS)
																									: (TimeSetClass.cur_focus.main == HOUR_SET_FOCUS)	 ? (TimeSetClass.cur_focus.main = MINUTE_SET_FOCUS)
																																						 : (TimeSetClass.cur_focus.main = YEAR_SET_FOCUS);
}

static void time_set_current_date_adjust(struct ak_date *date)
{
	if (date->year > YEAR_MAX)
		date->year = YEAR_MIN;
	if (date->year < YEAR_MIN)
		date->year = YEAR_MAX;
	if (date->month > MONTH_MAX)
		date->month = MONTH_MIN;
	if (date->month < MONTH_MIN)
		date->month = MONTH_MAX;
	if (date->day > DAY_MAX)
		date->day = DAY_MIN;
	if (date->day < DAY_MIN)
		date->day = DAY_MAX;
	if (date->hour > HOUR_MAX)
		date->hour = HOUR_MIN;
	if (date->hour < HOUR_MIN)
		date->hour = HOUR_MAX;
	if (date->minute > MINUTE_MAX)
		date->minute = MINUTE_MIN;
	if (date->minute < MINUTE_MIN)
		date->minute = MINUTE_MAX;
}

static void current_month_max_day_adjust(void)
{
	if (TimeSetClass.cur_focus.main == DAY_SET_FOCUS)
		return;
	if (TimeSetClass.date.day > DAY_MAX)
		TimeSetClass.date.day = DAY_MAX;
}

static void time_set_current_date(TIME_SET arg)
{
	if (arg == TIME_ADD)
	{
		(*TimeSetClass.date_array[TimeSetClass.cur_focus.main])++;
	}
	else
	{
		(*TimeSetClass.date_array[TimeSetClass.cur_focus.main])--;
	}
	current_month_max_day_adjust();
	time_set_current_date_adjust(&TimeSetClass.date);
	time_set_date_font_display();
}

static void time_set_key_up_up(void)
{
	if (TimeSetClass.cur_focus.layer == TIME_SET_MAIN_LAYER)
	{
		clear_prev_time_set_focus();
		goto_prev_time_set_focus();
		display_current_time_set_focus();
	}
	else
	{
		time_set_current_date(TIME_ADD);
	}
}

static void time_set_key_up_long(void)
{
	static char long_press_count = 0;
	long_press_count++;
	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && long_press_count > 5)
	{
		long_press_count = 0;
		time_set_current_date(TIME_ADD);
	}
}

static void time_set_key_down_long(void)
{
	static char long_press_count = 0;
	long_press_count++;

	if (TimeSetClass.cur_focus.layer == TIME_SET_ADJUST_LAYER && long_press_count > 5)
	{
		long_press_count = 0;
		time_set_current_date(TIME_SUB);
	}
}

static void time_set_key_down_up(void)
{
	if (TimeSetClass.cur_focus.layer == TIME_SET_MAIN_LAYER)
	{
		clear_prev_time_set_focus();
		goto_next_time_set_focus();
		display_current_time_set_focus();
	}
	else
	{
		time_set_current_date(TIME_SUB);
	}
}

static void time_set_key_star_up(void)
{
	if (TimeSetClass.cur_focus.layer == TIME_SET_MAIN_LAYER)
	{
		os_layout_goto(&layout_system_set);
	}
	else
	{
		TimeSetClass.cur_focus.layer = TIME_SET_MAIN_LAYER;
		display_current_time_set_focus();
	}
}

static void time_set_key_ring_up(void)
{

	clear_prev_time_set_focus();
	if (TimeSetClass.cur_focus.layer == TIME_SET_MAIN_LAYER)
	{
		TimeSetClass.cur_focus.layer = TIME_SET_ADJUST_LAYER;
	}
	else
	{
		TimeSetClass.cur_focus.layer = TIME_SET_MAIN_LAYER;
	}
	display_current_time_set_focus();
}

static void time_set_key_register(void)
{

	key_touch_handle_register(KEY_INDEX_UP, KEY_STATE_DOWN, time_set_key_up_up);
	key_touch_handle_register(KEY_INDEX_UP, KEY_STATE_LONG_DOWN, time_set_key_up_long);
	key_touch_handle_register(KEY_INDEX_DOWN, KEY_STATE_DOWN, time_set_key_down_up);
	key_touch_handle_register(KEY_INDEX_DOWN, KEY_STATE_LONG_DOWN, time_set_key_down_long);
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, time_set_key_star_up);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, time_set_key_ring_up);
}

static void time_set_data_init(void)
{

	ak_get_localdate(&TimeSetClass.date);
	if (language_get() == language_persian)
	{
		long date_array[3];
		gregorian_to_jalali(TimeSetClass.date.year, TimeSetClass.date.month + 1, TimeSetClass.date.day + 1, date_array);
		TimeSetClass.date.year = date_array[0];
		TimeSetClass.date.month = date_array[1] - 1;
		TimeSetClass.date.day = date_array[2] - 1;
	}
}

static void layout_time_set_enter(void)
{
	LOG_WHITE(">>> enter layout time set \n\r");

	TimeSetClass.data_init();
	TimeSetClass.widget_show.icon();
	TimeSetClass.widget_show.font();
	TimeSetClass.widget_show.focus();
	TimeSetClass.key_register();
}

static void layout_time_set_quit(void)
{

	LOG_WHITE(">>> quit layout time set \n\r");

	if (language_get() == language_persian)
	{
		long date_array[3];
		jalali_to_gregorian(TimeSetClass.date.year, TimeSetClass.date.month + 1, TimeSetClass.date.day + 1, date_array);
		TimeSetClass.date.year = date_array[0];
		TimeSetClass.date.month = date_array[1] - 1;
		TimeSetClass.date.day = date_array[2] - 1;
	}
	date_set(&TimeSetClass.date);
	TimeSetClass.cur_focus.main = YEAR_SET_FOCUS;
	TimeSetClass.cur_focus.layer = TIME_SET_MAIN_LAYER;
}

static int *DatePointerArray[TOTAL_TIME_SET_FOCUS] = {
	&TimeSetClass.date.year,
	&TimeSetClass.date.month,
	&TimeSetClass.date.day,
	&TimeSetClass.date.hour,
	&TimeSetClass.date.minute,
};

STR_TimeSetClass TimeSetClass = {
	{time_set_icon_display, time_set_font_display, time_set_focus_display, NULL, NULL},
	time_set_key_register,
	time_set_data_init,
	{YEAR_SET_FOCUS, TIME_SET_MAIN_LAYER},
	{2021, 01, 01, 00, 00, 00, 00},
	DatePointerArray,
};

layout layout_time_set = {
	.enter = layout_time_set_enter,
	.quit = layout_time_set_quit,
};
