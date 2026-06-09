#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"

/*********************************************************************************************************
 * 函 数 名 : set_system_set_icon_display
 * 功能说明 : 显示系统设置选项的图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 图标位置为(29,29)，尺寸34x34，资源为系统设置图标PNG
 *********************************************************************************************************/
static void set_system_set_icon_display(void)
{
	icon sys_icon;
	position pos = {{29, 29}, {34, 34}};
	resource res = resource_get(ROM_R_IMG_SET_PAGE_SYSTEM_SET_ICON_PNG);
	icon_init(&sys_icon, &pos, &res);
	icon_display(&sys_icon);
}

/*********************************************************************************************************
 * 函 数 名 : set_unlock_set_icon_display
 * 功能说明 : 显示解锁设置选项的图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 图标位置为(29,86)，尺寸34x34，资源为解锁设置图标PNG
 *********************************************************************************************************/
static void set_unlock_set_icon_display(void)
{
	icon unlock_icon;
	position pos = {{29, 86}, {34, 34}};
	resource res = resource_get(ROM_R_IMG_SET_PAGE_UNLOCK_ICON_PNG);
	icon_init(&unlock_icon, &pos, &res);
	icon_display(&unlock_icon);
}

/*********************************************************************************************************
 * 函 数 名 : set_password_set_icon_display
 * 功能说明 : 显示密码设置选项的图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 图标位置为(29,143)，尺寸34x34，资源为密码设置图标PNG
 *********************************************************************************************************/
static void set_password_set_icon_display(void)
{
	icon password_icon;
	position pos = {{29, 143}, {34, 34}};
	resource res = resource_get(ROM_R_IMG_SET_PAGE_PASSWORD_ICON_PNG);
	icon_init(&password_icon, &pos, &res);
	icon_display(&password_icon);
}

/*********************************************************************************************************
 * 函 数 名 : set_floor_set_icon_display
 * 功能说明 : 显示楼层/单元地址设置选项的图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 图标位置为(296,30)，尺寸34x34，资源为楼层设置图标PNG
 *********************************************************************************************************/
static void set_floor_set_icon_display(void)
{
	icon floor_icon;
	position pos = {{290, 30}, {34, 34}};
	resource res = resource_get(ROM_R_IMG_SET_PAGE_FLOOR_ICON_PNG);
	icon_init(&floor_icon, &pos, &res);
	icon_display(&floor_icon);
}

/*********************************************************************************************************
 * 函 数 名 : set_card_set_icon_display
 * 功能说明 : 显示卡片管理设置选项的图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 图标位置为(296,90)，尺寸34x34，资源为卡片设置图标PNG
 *********************************************************************************************************/
static void set_card_set_icon_display(void)
{
	icon card_icon;
	position pos = {{290, 90}, {34, 34}};
	resource res = resource_get(ROM_R_IMG_SET_PAGE_CARD_ICON_PNG);
	icon_init(&card_icon, &pos, &res);
	icon_display(&card_icon);
}

/*********************************************************************************************************
 * 函 数 名 : settings_icon_display
 * 功能说明 : 设置界面所有图标显示的入口函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 依次显示taba按钮、系统设置、解锁设置、密码设置、楼层设置、卡片设置图标
 *********************************************************************************************************/
static void settings_icon_display(void)
{
	taba_btn_display();
	set_system_set_icon_display();
	set_unlock_set_icon_display();
	set_password_set_icon_display();
	set_floor_set_icon_display();
	set_card_set_icon_display();
}

/*********************************************************************************************************
 * 函 数 名 : set_system_set_font_display
 * 功能说明 : 显示系统设置选项的文字提示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(69,35)，尺寸140x30，字体大小20，左上对齐
 *********************************************************************************************************/
static void set_system_set_font_display(void)
{
	position pos = {{69, 35}, {140, 30}};
	text system_set;

	text_init(&system_set, &pos, 20);
	system_set.align = LEFT_TOP;
	text_display(&system_set, font_str(STR_SET_SYSTEM_SET));
}

/*********************************************************************************************************
 * 函 数 名 : set_unlock_set_font_display
 * 功能说明 : 显示解锁设置选项的文字提示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(69,92)，尺寸155x30，字体大小20，左上对齐
 *********************************************************************************************************/
static void set_unlock_set_font_display(void)
{
	position pos = {{69, 92}, {155, 30}};
	text unlock_set;

	text_init(&unlock_set, &pos, 20);
	unlock_set.align = LEFT_TOP;
	text_display(&unlock_set, font_str(STR_SET_UNLOCK_SET));
}

/*********************************************************************************************************
 * 函 数 名 : set_password_set_font_display
 * 功能说明 : 显示密码设置选项的文字提示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(69,149)，尺寸155x30，字体大小20，左上对齐
 *********************************************************************************************************/
static void set_password_set_font_display(void)
{
	position pos = {{69, 149}, {155, 30}};
	text password_set;

	text_init(&password_set, &pos, 20);
	password_set.align = LEFT_TOP;
	text_display(&password_set, font_str(STR_SET_PASSWORD_SET));
}

/*********************************************************************************************************
 * 函 数 名 : set_floor_set_font_display
 * 功能说明 : 显示楼层/单元地址配置选项的文字提示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(336,36)，尺寸70x30，字体大小20，左上对齐
 *********************************************************************************************************/
static void set_floor_set_font_display(void)
{
	position pos = {{330, 36}, {70, 30}};
	text floor_set;

	text_init(&floor_set, &pos, 20);
	floor_set.align = LEFT_TOP;
	text_display(&floor_set, font_str(STR_SET_CONFIGURE_UNIT_ADDR));
}

/*********************************************************************************************************
 * 函 数 名 : set_card_set_font_display
 * 功能说明 : 显示卡片管理选项的文字提示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(336,96)，尺寸50x30，字体大小20，左上对齐
 *********************************************************************************************************/
static void set_card_set_font_display(void)
{
	position pos = {{330, 96}, {56, 30}};
	text card_set;
	int text_size = 20;
	align card_set_align = LEFT_TOP;

	if (language_get() == language_persian)
	{
		position focus_pos = {{425, 90}, {32, 32}};

		pos = (position){{300, 96}, {120, 30}};
		text_size = 18;
		card_set_align = RIGHT_MIDDLE;
		if (pos.point.x + pos.vector.width > focus_pos.point.x)
		{
			pos.vector.width = focus_pos.point.x - pos.point.x;
		}
	}

	text_init(&card_set, &pos, text_size);
	card_set.align = card_set_align;
	text_display(&card_set, font_str(STR_SET_CARD_MANAGEMENT));
}

/*********************************************************************************************************
 * 函 数 名 : settings_font_display
 * 功能说明 : 设置界面所有文字提示显示的入口函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 依次显示系统设置、解锁设置、密码设置、楼层设置、卡片设置的文字提示
 *********************************************************************************************************/
static void settings_font_display(void)
{
	set_system_set_font_display();
	set_unlock_set_font_display();
	set_password_set_font_display();
	set_floor_set_font_display();
	set_card_set_font_display();
}

/*********************************************************************************************************
 * 函 数 名 : clear_prev_settings_focus
 * 功能说明 : 清除上一个选中设置项的焦点图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据当前焦点位置确定清除区域，使用黑色背景擦除
 *********************************************************************************************************/
static void clear_prev_settings_focus(void)
{
	position pos = {{237, 30}, {32, 32}};

	// 根据当前焦点设置坐标
	switch (SettingsClass.cur_focus)
	{
	case SYSTEM_SET_FOCUS: // 0
		pos.point.x = 237;
		pos.point.y = 30;
		break;
	case UNLOCK_SET_FOCUS: // 1
		pos.point.x = 237;
		pos.point.y = 90;
		break;
	case PASSWORD_SET_FOCUS: // 2
		pos.point.x = 237;
		pos.point.y = 145;
		break;
	case FLOOR_SET_FOCUS: // 3
		pos.point.x = 425;
		pos.point.y = 30;
		break;
	case CARD_SET_FOCUS: // 4
		pos.point.x = 425;
		pos.point.y = 90;
		break;
	case TOTAL_SET_FOCUS:
		break;
	}

	gui_erase(&pos, 0x00);
}

/*********************************************************************************************************
 * 函 数 名 : display_current_settings_focus
 * 功能说明 : 显示当前选中设置项的焦点图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据当前焦点位置确定显示坐标，加载焦点图标资源并显示
 *********************************************************************************************************/
static void display_current_settings_focus(void)
{
	icon focus;
	position pos = {{237, 30}, {32, 32}};

	// 根据当前焦点设置坐标
	switch (SettingsClass.cur_focus)
	{
	case SYSTEM_SET_FOCUS: // 0
		pos.point.x = 237;
		pos.point.y = 30;
		break;
	case UNLOCK_SET_FOCUS: // 1
		pos.point.x = 237;
		pos.point.y = 90;
		break;
	case PASSWORD_SET_FOCUS: // 2
		pos.point.x = 237;
		pos.point.y = 145;
		break;
	case FLOOR_SET_FOCUS: // 3
		pos.point.x = 425;
		pos.point.y = 30;
		break;
	case CARD_SET_FOCUS: // 4
		pos.point.x = 425;
		pos.point.y = 90;
		break;
	case TOTAL_SET_FOCUS:
		break;
	}

	resource res = resource_get(ROM_R_IMG_SET_PAGE_FOCUS_PNG);
	icon_init(&focus, &pos, &res);
	icon_display(&focus);
}

/*********************************************************************************************************
 * 函 数 名 : settings_focus_display
 * 功能说明 : 设置界面焦点显示的入口函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 先清除上一个焦点，再显示当前选中项的焦点图标
 *********************************************************************************************************/
static void settings_focus_display(void)
{
	clear_prev_settings_focus();
	display_current_settings_focus();
}

/*********************************************************************************************************
 * 函 数 名 : layout_settings_init
 * 功能说明 : 设置布局的初始化函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 预留初始化接口，当前无具体初始化逻辑
 *********************************************************************************************************/
static void layout_settings_init(void)
{
}

/*********************************************************************************************************
 * 函 数 名 : layout_settings_enter
 * 功能说明 : 进入设置布局时的初始化操作
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 显示图标、文字、焦点，注册按键回调函数并打印日志
 *********************************************************************************************************/
static void layout_settings_enter(void)
{
	LOG_WHITE(">>> enter layout settings \n\r");

	SettingsClass.widget_show.icon();
	SettingsClass.widget_show.font();
	SettingsClass.widget_show.focus();
	SettingsClass.key_register();
}

/*********************************************************************************************************
 * 函 数 名 : layout_settings_quit
 * 功能说明 : 退出设置布局时的清理操作
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 仅打印退出日志，预留清理接口
 *********************************************************************************************************/
static void layout_settings_quit(void)
{
	LOG_WHITE(">>> quit layout settings \n\r");
}

/*********************************************************************************************************
 * 函 数 名 : goto_prev_settings_focus
 * 功能说明 : 切换到上一个设置焦点项
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 焦点切换顺序：系统设置←解锁设置←密码设置←楼层设置←卡片设置←系统设置
 *********************************************************************************************************/
static void goto_prev_settings_focus(void)
{
	(SettingsClass.cur_focus == SYSTEM_SET_FOCUS) ? (SettingsClass.cur_focus = CARD_SET_FOCUS) : (SettingsClass.cur_focus == UNLOCK_SET_FOCUS) ? (SettingsClass.cur_focus = SYSTEM_SET_FOCUS)
																							 : (SettingsClass.cur_focus == PASSWORD_SET_FOCUS) ? (SettingsClass.cur_focus = UNLOCK_SET_FOCUS)
																							 : (SettingsClass.cur_focus == FLOOR_SET_FOCUS)	   ? (SettingsClass.cur_focus = PASSWORD_SET_FOCUS)
																																			   : (SettingsClass.cur_focus = FLOOR_SET_FOCUS);

	// (SettingsClass.cur_focus == SYSTEM_SET_FOCUS)   ? (SettingsClass.cur_focus = CARD_SET_FOCUS) :
	// (SettingsClass.cur_focus == UNLOCK_SET_FOCUS)   ? (SettingsClass.cur_focus = SYSTEM_SET_FOCUS) :
	// (SettingsClass.cur_focus == PASSWORD_SET_FOCUS) ? (SettingsClass.cur_focus = UNLOCK_SET_FOCUS) :
	// (SettingsClass.cur_focus = PASSWORD_SET_FOCUS); // 直接跳转到密码设
}

/*********************************************************************************************************
 * 函 数 名 : goto_next_settings_focus
 * 功能说明 : 切换到下一个设置焦点项
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 焦点切换顺序：系统设置→解锁设置→密码设置→楼层设置→卡片设置→系统设置
 *********************************************************************************************************/
static void goto_next_settings_focus(void)
{
	(SettingsClass.cur_focus == SYSTEM_SET_FOCUS) ? (SettingsClass.cur_focus = UNLOCK_SET_FOCUS) : (SettingsClass.cur_focus == UNLOCK_SET_FOCUS) ? (SettingsClass.cur_focus = PASSWORD_SET_FOCUS)
																							   : (SettingsClass.cur_focus == PASSWORD_SET_FOCUS) ? (SettingsClass.cur_focus = FLOOR_SET_FOCUS)
																							   : (SettingsClass.cur_focus == FLOOR_SET_FOCUS)	 ? (SettingsClass.cur_focus = CARD_SET_FOCUS)
																																				 : (SettingsClass.cur_focus = SYSTEM_SET_FOCUS);

	// (SettingsClass.cur_focus == SYSTEM_SET_FOCUS)   ? (SettingsClass.cur_focus = UNLOCK_SET_FOCUS) :
	// (SettingsClass.cur_focus == UNLOCK_SET_FOCUS)   ? (SettingsClass.cur_focus = PASSWORD_SET_FOCUS) :
	// (SettingsClass.cur_focus == PASSWORD_SET_FOCUS) ? (SettingsClass.cur_focus = CARD_SET_FOCUS) :
	// (SettingsClass.cur_focus = SYSTEM_SET_FOCUS);
}

/*********************************************************************************************************
 * 函 数 名 : settings_key_up_up
 * 功能说明 : 上方向键的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 清除旧焦点，切换到上一个焦点项，显示新焦点
 *********************************************************************************************************/
static void settings_key_up_up(void)
{
	clear_prev_settings_focus();
	goto_prev_settings_focus();
	display_current_settings_focus();
}

/*********************************************************************************************************
 * 函 数 名 : settings_key_down_up
 * 功能说明 : 下方向键的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 清除旧焦点，切换到下一个焦点项，显示新焦点
 *********************************************************************************************************/
static void settings_key_down_up(void)
{
	clear_prev_settings_focus();
	goto_next_settings_focus();
	display_current_settings_focus();
}

/*********************************************************************************************************
 * 函 数 名 : settings_key_star_up
 * 功能说明 : 星号键(*)的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 重置焦点为系统设置，跳转到拨号布局
 *********************************************************************************************************/
static void settings_key_star_up(void)
{
	SettingsClass.cur_focus = SYSTEM_SET_FOCUS;
	os_layout_goto(&layout_dial);
}

/*********************************************************************************************************
 * 函 数 名 : settings_key_ring_up
 * 功能说明 : 井号键(#)的按键回调函数（确认键）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据当前焦点项跳转到对应子设置界面，解锁/密码设置需指定触发模式
 *********************************************************************************************************/
static void settings_key_ring_up(void)
{
	switch (SettingsClass.cur_focus)
	{
	case SYSTEM_SET_FOCUS:
		os_layout_goto(&layout_system_set);
		break;
	case UNLOCK_SET_FOCUS:
		PasswordClass.trigger_mode = INPUT_OLD_UNLOCK;
		os_layout_goto(&layout_password);
		break;
	case PASSWORD_SET_FOCUS:
		PasswordClass.trigger_mode = INPUT_OLD_SETTING;
		os_layout_goto(&layout_password);
		break;
	case FLOOR_SET_FOCUS:
		os_layout_goto(&layout_OutPUT);
		break;
	case CARD_SET_FOCUS:
		os_layout_goto(&layout_card_manage);
		break;
	default:
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : settings_key_register
 * 功能说明 : 注册设置界面的所有按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 注册上/下方向键、星号键、井号键的按下事件回调
 *********************************************************************************************************/
static void settings_key_register(void)
{
	key_touch_handle_register(KEY_INDEX_UP, KEY_STATE_DOWN, settings_key_up_up);
	key_touch_handle_register(KEY_INDEX_DOWN, KEY_STATE_DOWN, settings_key_down_up);
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, settings_key_star_up);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, settings_key_ring_up);
}

STR_SettingsClass SettingsClass = {
	{settings_icon_display, settings_font_display, settings_focus_display, NULL, NULL},
	settings_key_register,
	SYSTEM_SET_FOCUS};

layout layout_settings = {
	.init = layout_settings_init,
	.enter = layout_settings_enter,
	.quit = layout_settings_quit,
};
