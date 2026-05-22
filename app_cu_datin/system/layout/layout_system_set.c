#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "gpio_control.h"

/* 语言 */
static void system_set_lang_icon_display(void)
{
	icon sys_icon;
	position pos = {{62, 33}, {34, 34}};
	resource res = resource_get(ROM_R_IMG_SYSTEM_SET_LANGUAGE_ICON_PNG);
	icon_init(&sys_icon, &pos, &res);
	icon_display(&sys_icon);
}

static void system_set_lang_bracket_icon_display(void)
{
	icon left_icon;
	position left_pos = {{262, 43}, {6, 16}};
	gui_erase(&left_pos, 0x00);
	if (SystemSetClass.cur_focus.layer == SYSTEM_SET_LANG_LAYER)
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
	position right_pos = {{373, 43}, {6, 16}};
	gui_erase(&right_pos, 0x00);
	if (SystemSetClass.cur_focus.layer == SYSTEM_SET_LANG_LAYER)
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

static void system_set_time_icon_display(void)
{
	icon time_icon;
	position pos = {{62, 90}, {34, 34}};
	resource res = resource_get(ROM_R_IMG_SYSTEM_SET_TIME_ICON_PNG);
	icon_init(&time_icon, &pos, &res);
	icon_display(&time_icon);
}

/* **************************************************/
static void system_set_reset_icon_display(void)
{
	icon floor_icon;
	position pos = {{62, 147}, {34, 34}};
	resource res = resource_get(ROM_R_IMG_SYSTEM_SET_RESET_ICON_PNG);
	icon_init(&floor_icon, &pos, &res);
	icon_display(&floor_icon);
}
/* *****************************************************/

static void system_set_icon_display(void)
{
	taba_btn_display();
	system_set_lang_icon_display();
	system_set_lang_bracket_icon_display();
	system_set_time_icon_display();
	system_set_reset_icon_display();
}

static void system_set_lang_set_font_display(void)
{
	position pos = {{112, 35}, {120, 25}};
	text lang_set;

	gui_erase(&pos, 0x00);
	text_init(&lang_set, &pos, 20);
	lang_set.align = LEFT_TOP;
	text_display(&lang_set, font_str(STR_SYSTEM_SET_LANG_SET));
}

static void system_set_lang_font_display(void)
{
	position pos = {{292, 41}, {62, 25}};
	text lang;

	gui_erase(&pos, 0x00);
	text_init(&lang, &pos, 18);
	text_display(&lang, font_str(STR_SYSTEM_SET_LANG));
}
static void system_set_time_set_font_display(void)
{
	position pos = {{112, 92}, {150, 25}};
	text time_set;

	gui_erase(&pos, 0x00);
	text_init(&time_set, &pos, 20);
	time_set.align = LEFT_TOP;
	text_display(&time_set, font_str(STR_SYSTEM_SET_TIME_SET));
}

/*********************************************************/
static void system_set_reset_font_display(void)
{
	position pos = {{112, 149}, {150, 25}};
	text floor_set;

	gui_erase(&pos, 0x00);
	text_init(&floor_set, &pos, 20);
	floor_set.align = LEFT_TOP;
	text_display(&floor_set, font_str(STR_SYSTEM_SET_RESET));
}
/*********************************************************/

static void system_set_font_display(void)
{
	system_set_lang_set_font_display();
	system_set_lang_font_display();
	system_set_time_set_font_display();
	system_set_reset_font_display();
}

static void system_set_logo_refresh(void)
{
	position pos = {{0, 193}, {480, 79}};
	gui_erase(&pos, 0x00);
	taba_btn_display();
}

static void system_set_focus_icon_display(void)
{
	// -------------------------- 左侧图标处理 --------------------------
	icon left_icon;
	position left_pos = {{115, 134}, {125, 43}}; // 左侧区域坐标（
	gui_erase(&left_pos, 0x00);					 // 清屏

	if (SystemSetClass.sure == SYSTEM_SET_SURE_YES_FOUCUS)
	{
		// 选中左侧（yes）：加载左侧焦点图
		resource res = resource_get(ROM_R_IMG_CARD_MANEAGE_LEFT_FOCUS_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	else
	{
		// 选中右侧（no）：加载左侧默认图
		resource res = resource_get(ROM_R_IMG_CARD_MANEAGE_LEFT_ICON_PNG);
		icon_init(&left_icon, &left_pos, &res);
	}
	icon_display(&left_icon);

	// -------------------------- 右侧图标处理 --------------------------
	icon right_icon;
	position right_pos = {{240, 134}, {125, 43}}; // 右侧区域坐标
	gui_erase(&right_pos, 0x00);				  // 清屏

	if (SystemSetClass.sure == SYSTEM_SET_SURE_YES_FOUCUS)
	{
		// 选中左侧（yes）：加载右侧默认图
		resource right_res = resource_get(ROM_R_IMG_CARD_MANEAGE_RIGHT_ICON_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	else
	{
		// 选中右侧（no）：加载右侧焦点图
		resource right_res = resource_get(ROM_R_IMG_CARD_MANEAGE_RIGHT_FOCUS_PNG);
		icon_init(&right_icon, &right_pos, &right_res);
	}
	icon_display(&right_icon);
}

static void system_set_dialog_box_init(void)
{
	icon box;
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 0};

	position box_pos = {{115, 59}, {250, 118}};
	resource box_res = resource_get(ROM_R_IMG_CARD_MANEAGE_POP_UP_PNG);
	icon_init(&box, &box_pos, &box_res);

	position text1_pos = {{165, 80}, {155, 30}};
	text_init(&font.text1, &text1_pos, 20);
	font.string1 = font_str(STR_SYSTEM_SET_RESET);

	position text2_pos = {{152, 140}, {40, 30}};
	text_init(&font.text2, &text2_pos, 20);
	font.text2.font_color = 0xFFFF0000;
	font.string2 = font_str(STR_CARD_MANAGE_YES);

	position text3_pos = {{290, 140}, {40, 30}};
	text_init(&font.text3, &text3_pos, 20);
	font.text3.font_color = 0xFFFFFFFF;
	font.string3 = font_str(STR_CARD_MANAGE_NO);

	widget_dialog_box_init(SystemSetClass.dialog_box,
						   &box,
						   &font,
						   &cursor);
}

static void clear_prev_system_set_focus(void)
{

	position pos = {{418, 34}, {32, 32}};
	pos.point.y = 34 + 57 * SystemSetClass.cur_focus.main;
	gui_erase(&pos, 0x00);
}

static void display_current_system_set_focus(void)
{
	icon focus;
	position pos = {{418, 34}, {32, 32}};
	if (SystemSetClass.cur_focus.layer == SYSTEM_SET_MAIN_LAYER)
	{
		pos.point.y = 34 + 57 * SystemSetClass.cur_focus.main;
		resource res = resource_get(ROM_R_IMG_SET_PAGE_FOCUS_PNG);
		icon_init(&focus, &pos, &res);
		icon_display(&focus);
	}
	system_set_lang_bracket_icon_display();
}

static void system_set_focus_display(void)
{
	clear_prev_system_set_focus();
	display_current_system_set_focus();
}

static void ClearScreen(void)
{
	position pos = {{0, 0}, {480, 195}};
	gui_erase(&pos, 0x00);
}

static void system_set_dialog_display(void)
{
	if (SystemSetClass.cur_focus.layer == SYSTEM_SET_RESET_LAYER)
	{
		ClearScreen();
		widget_dialog_box_display(SystemSetClass.dialog_box);
		system_set_focus_icon_display();

		STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
		position text2_pos = {{152, 140}, {40, 30}};
		text_init(&font.text2, &text2_pos, 20);
		font.text2.font_color = 0xFFFF0000;
		font.string2 = font_str(STR_CARD_MANAGE_YES);

		position text3_pos = {{290, 140}, {40, 30}};
		text_init(&font.text3, &text3_pos, 20);
		font.text3.font_color = 0xFFFFFFFF;
		font.string3 = font_str(STR_CARD_MANAGE_NO);
		text_display(&font.text2, font.string2);
		text_display(&font.text3, font.string3);
	}
}

static void goto_prev_system_set_main_focus(void)
{

	(SystemSetClass.cur_focus.main == LANGUAGE_SET_FOCUS) ? (SystemSetClass.cur_focus.main = SYSTEM_SET_RESET_FOCUS) : (SystemSetClass.cur_focus.main == TIME_SET_FOCUS)	   ? (SystemSetClass.cur_focus.main = LANGUAGE_SET_FOCUS)
																												   : (SystemSetClass.cur_focus.main == SYSTEM_SET_RESET_FOCUS) ? (SystemSetClass.cur_focus.main = TIME_SET_FOCUS)
																																											   : (SystemSetClass.cur_focus.main = LANGUAGE_SET_FOCUS);
}

static void goto_next_system_set_main_focus(void)
{

	(SystemSetClass.cur_focus.main == LANGUAGE_SET_FOCUS) ? (SystemSetClass.cur_focus.main = TIME_SET_FOCUS) : (SystemSetClass.cur_focus.main == TIME_SET_FOCUS)	   ? (SystemSetClass.cur_focus.main = SYSTEM_SET_RESET_FOCUS)
																										   : (SystemSetClass.cur_focus.main == SYSTEM_SET_RESET_FOCUS) ? (SystemSetClass.cur_focus.main = LANGUAGE_SET_FOCUS)
																																									   : (SystemSetClass.cur_focus.main = LANGUAGE_SET_FOCUS);
}

/* key [up] */
static void system_set_key_up_up(void)
{

	if (SystemSetClass.cur_focus.layer == SYSTEM_SET_MAIN_LAYER)
	{
		clear_prev_system_set_focus();
		goto_prev_system_set_main_focus();
		display_current_system_set_focus();
	}
	else if (SystemSetClass.cur_focus.layer == SYSTEM_SET_LANG_LAYER)
	{
		if (language_get() == language_english)
		{
			language_set(language_persian);
		}
		else
		{
			language_set(language_english);
		}
		font_file_reload();
		SystemSetClass.widget_show.font();
		system_set_logo_refresh();
	}
	else if (SystemSetClass.cur_focus.layer == SYSTEM_SET_RESET_LAYER)
	{
		(SystemSetClass.sure == SYSTEM_SET_SURE_NO_FOUCUS) ? (SystemSetClass.sure = SYSTEM_SET_SURE_YES_FOUCUS) : (SystemSetClass.sure = SYSTEM_SET_SURE_NO_FOUCUS);
		SystemSetClass.widget_show.dialog_box();
	}
}

/* key [down] */
static void system_set_key_down_up(void)
{

	if (SystemSetClass.cur_focus.layer == SYSTEM_SET_MAIN_LAYER)
	{
		clear_prev_system_set_focus();
		goto_next_system_set_main_focus();
		display_current_system_set_focus();
	}
	else if (SystemSetClass.cur_focus.layer == SYSTEM_SET_LANG_LAYER)
	{
		if (language_get() == language_english)
		{
			language_set(language_persian);
		}
		else
		{
			language_set(language_english);
		}
		font_file_reload();
		SystemSetClass.widget_show.font();
		system_set_logo_refresh();
	}
	else if (SystemSetClass.cur_focus.layer == SYSTEM_SET_RESET_LAYER)
	{
		(SystemSetClass.sure == SYSTEM_SET_SURE_NO_FOUCUS) ? (SystemSetClass.sure = SYSTEM_SET_SURE_YES_FOUCUS) : (SystemSetClass.sure = SYSTEM_SET_SURE_NO_FOUCUS);
		SystemSetClass.widget_show.dialog_box();
	}
}

/* key [*] */
static void system_set_key_star_up(void)
{
	if (SystemSetClass.cur_focus.layer == SYSTEM_SET_MAIN_LAYER)
	{
		SystemSetClass.cur_focus.main = LANGUAGE_SET_FOCUS;
		os_layout_goto(&layout_settings);
	}
	else if (SystemSetClass.cur_focus.layer == SYSTEM_SET_RESET_LAYER)
	{
		SystemSetClass.sure = SYSTEM_SET_SURE_YES_FOUCUS;
		SystemSetClass.cur_focus.layer = SYSTEM_SET_MAIN_LAYER;
		ClearScreen();
		SystemSetClass.widget_show.icon();
		SystemSetClass.widget_show.font();
		SystemSetClass.widget_show.focus();
	}
	else
	{
		clear_prev_system_set_focus();
		if (SystemSetClass.cur_focus.layer == SYSTEM_SET_LANG_LAYER)
		{
			SystemSetClass.cur_focus.layer = SYSTEM_SET_MAIN_LAYER;
		}
		display_current_system_set_focus();
	}
}

/* key ring */
static void system_set_key_ring_up(void)
{
	clear_prev_system_set_focus();
	if (SystemSetClass.cur_focus.layer == SYSTEM_SET_MAIN_LAYER)
	{
		if (SystemSetClass.cur_focus.main == TIME_SET_FOCUS)
		{
			os_layout_goto(&layout_time_set);
		}
		else if (SystemSetClass.cur_focus.main == SYSTEM_SET_RESET_FOCUS)
		{
			SystemSetClass.cur_focus.layer = SYSTEM_SET_RESET_LAYER;
			ClearScreen();
			system_set_dialog_box_init();
			SystemSetClass.widget_show.dialog_box();
		}
		else
		{

			if (SystemSetClass.cur_focus.main == LANGUAGE_SET_FOCUS)
			{
				SystemSetClass.cur_focus.layer = SYSTEM_SET_LANG_LAYER;
			}
			display_current_system_set_focus();
		}
	}
	else
	{
		if (SystemSetClass.cur_focus.layer == SYSTEM_SET_LANG_LAYER)
		{
			LOG_BLUE("lag -> main \n");
			set_int_conf(LANGUAGE, language_get());
			user_data_save();
			SystemSetClass.cur_focus.layer = SYSTEM_SET_MAIN_LAYER;
			display_current_system_set_focus();
		}
		else if (SystemSetClass.cur_focus.layer == SYSTEM_SET_RESET_LAYER)
		{
			if (SystemSetClass.sure == SYSTEM_SET_SURE_YES_FOUCUS)
			{
				message = RESET_SYSTEM;
				os_layout_goto(&layout_info);
			}
			else
			{
				SystemSetClass.sure = SYSTEM_SET_SURE_YES_FOUCUS;
				SystemSetClass.cur_focus.layer = SYSTEM_SET_MAIN_LAYER;
				ClearScreen();
				SystemSetClass.widget_show.icon();
				SystemSetClass.widget_show.font();
				SystemSetClass.widget_show.focus();
			}
		}
	}
}

static void system_set_key_register(void)
{

	key_touch_handle_register(KEY_INDEX_UP, KEY_STATE_DOWN, system_set_key_up_up);
	key_touch_handle_register(KEY_INDEX_DOWN, KEY_STATE_DOWN, system_set_key_down_up);
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, system_set_key_star_up);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, system_set_key_ring_up);
}

static void layout_system_set_init(void)
{
	language_set(get_int_conf(LANGUAGE));
}

static void layout_system_set_enter(void)
{
	LOG_WHITE(">>> enter layout system set \n\r");

	SystemSetClass.widget_show.icon();
	SystemSetClass.widget_show.font();
	SystemSetClass.widget_show.focus();
	SystemSetClass.key_register();
}

static void layout_system_set_quit(void)
{
	LOG_WHITE(">>> quit layout system set \n\r");

	set_int_conf(LANGUAGE, language_get());
}

static void layout_system_set_timer(void)
{
}
STR_WidgetDialogBox SystemSetDialgBox;
STR_SystemSetClass SystemSetClass = {
	{system_set_icon_display, system_set_font_display, system_set_focus_display, system_set_dialog_display, NULL},
	&SystemSetDialgBox,
	system_set_key_register,
	{LANGUAGE_SET_FOCUS, SYSTEM_SET_MAIN_LAYER},
	SYSTEM_SET_SURE_YES_FOUCUS};

layout layout_system_set = {
	.init = layout_system_set_init,
	.enter = layout_system_set_enter,
	.quit = layout_system_set_quit,
	.timer = layout_system_set_timer};
