#include "os_sys_api.h"
#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "gpio_control.h"

static analog_clock standy_clock;
// extern bool key_num_flag;

/***
 * 描述: 返回息屏界面
 * 日期: 2022-08-15 15:29
 * 作者:
 ***/
void goto_layout_standby(void)
{
	os_layout_goto(&layout_standby);
}
static void layout_standby_enter(void)
{
	LOG_WHITE(">>> enter layout standby \n\r");
	// key_num_flag = true;
	StandbyClass.data_init();
	StandbyClass.bg_show();
	StandbyClass.key_register();
	standby_timer_close();

	analog_clock_init(&standy_clock);
	analog_clock_update();
	amplifier_gpio_control(false);
}

static void layout_standby_quit(void)
{

	LOG_WHITE(">>> quit layout standby  \n\r");
	// key_num_flag = false;
	bg_img_init();
	standby_timer_open(0, NULL);

	amplifier_gpio_control(true);
	analog_clock_deinit();
}

static void standby_bg_display(void)
{
	StandbyClass.bg_index = WELOCOME_BG;
}

static void standby_key0_up(void)
{

	dial_input_add_number(0);
	os_layout_goto(&layout_dial);
}

static void standby_key1_up(void)
{

	dial_input_add_number(1);
	os_layout_goto(&layout_dial);
}

static void standby_key2_up(void)
{

	dial_input_add_number(2);
	os_layout_goto(&layout_dial);
}

static void standby_key3_up(void)
{

	dial_input_add_number(3);
	os_layout_goto(&layout_dial);
}

static void standby_key4_up(void)
{

	dial_input_add_number(4);
	os_layout_goto(&layout_dial);
}

static void standby_key5_up(void)
{

	dial_input_add_number(5);
	os_layout_goto(&layout_dial);
}

static void standby_key6_up(void)
{

	dial_input_add_number(6);
	os_layout_goto(&layout_dial);
}

static void standby_key7_up(void)
{

	dial_input_add_number(7);
	os_layout_goto(&layout_dial);
}

static void standby_key8_up(void)
{

	dial_input_add_number(8);
	os_layout_goto(&layout_dial);
}

static void standby_key9_up(void)
{

	dial_input_add_number(9);
	os_layout_goto(&layout_dial);
}

static void standby_key_up_up(void)
{
	os_layout_goto(&layout_dial);
}

static void standby_key_down_up(void)
{
	os_layout_goto(&layout_dial);
}

static struct ak_timeval standby_key_press_start_time;
static void standby_key_star_down(void)
{
	ak_get_ostime(&standby_key_press_start_time);
}

static void standby_key_star_long(void)
{
	struct ak_timeval cur_cpu_time;
	ak_get_ostime(&cur_cpu_time);
	if (ak_diff_ms_time(&cur_cpu_time, &standby_key_press_start_time) >= 3000)
	{
		key_long_press_goto_layout = DIAL_STAR_LONG_PRESS;
		PasswordClass.trigger_mode = INPUT_UNLOCK;
		os_layout_goto(&layout_password);
	}
}

static void standby_key_star_up(void)
{
	os_layout_goto(&layout_dial);
}

static void standby_key_pound_down(void)
{
	ak_get_ostime(&standby_key_press_start_time);
}

static void standby_key_pound_long(void)
{
	struct ak_timeval cur_cpu_time;
	ak_get_ostime(&cur_cpu_time);
	if (ak_diff_ms_time(&cur_cpu_time, &standby_key_press_start_time) >= 3000)
	{
		PasswordClass.trigger_mode = INPUT_SETTING;
		os_layout_goto(&layout_password);
	}
}

static void standby_key_pound_up(void)
{
	os_layout_goto(&layout_dial);
}

static void standby_key_guard_up(void)
{
	CallingClass.call_object = CALL_GUARD;
	os_layout_goto(&layout_calling);
}

static void standby_key_ring_up(void)
{
	os_layout_goto(&layout_dial);
}

static void standby_key_register(void)
{

	key_touch_handle_register(KEY_INDEX_0, KEY_STATE_DOWN, standby_key0_up);
	key_touch_handle_register(KEY_INDEX_1, KEY_STATE_DOWN, standby_key1_up);
	key_touch_handle_register(KEY_INDEX_2, KEY_STATE_DOWN, standby_key2_up);
	key_touch_handle_register(KEY_INDEX_3, KEY_STATE_DOWN, standby_key3_up);
	key_touch_handle_register(KEY_INDEX_4, KEY_STATE_DOWN, standby_key4_up);
	key_touch_handle_register(KEY_INDEX_5, KEY_STATE_DOWN, standby_key5_up);
	key_touch_handle_register(KEY_INDEX_6, KEY_STATE_DOWN, standby_key6_up);
	key_touch_handle_register(KEY_INDEX_7, KEY_STATE_DOWN, standby_key7_up);
	key_touch_handle_register(KEY_INDEX_8, KEY_STATE_DOWN, standby_key8_up);
	key_touch_handle_register(KEY_INDEX_9, KEY_STATE_DOWN, standby_key9_up);
	key_touch_handle_register(KEY_INDEX_UP, KEY_STATE_DOWN, standby_key_up_up);
	key_touch_handle_register(KEY_INDEX_DOWN, KEY_STATE_DOWN, standby_key_down_up);

	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, standby_key_star_down);
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_LONG_DOWN, standby_key_star_long);
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, standby_key_star_up);

	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_UP, standby_key_pound_up);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_LONG_DOWN, standby_key_pound_long);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, standby_key_pound_down);

	key_touch_handle_register(KEY_INDEX_GUARD, KEY_STATE_DOWN, standby_key_guard_up);
	key_touch_handle_register(KEY_INDEX_RING, KEY_STATE_DOWN, standby_key_ring_up);
}

static void standby_data_init(void)
{
	SettingsClass.cur_focus = SYSTEM_SET_FOCUS;
	SystemSetClass.cur_focus.main = LANGUAGE_SET_FOCUS;
	SystemSetClass.cur_focus.layer = SYSTEM_SET_MAIN_LAYER;
	SwipingCard.mode = CARD_IDLE_MODE;
	amplifier_gpio_control(false);
}

static void layout_standby_timer(void)
{
	// 新增：每秒更新时钟显示
	struct ak_date date;
	static int Prev_Sec_Time = -1; // 记录上一秒，避免重复更新
	ak_get_localdate(&date);	   // 获取当前时间
	if (date.second != Prev_Sec_Time)
	{
		Prev_Sec_Time = date.second;
		analog_clock_update(); // 每秒更新时钟指针
	}
}

static void layout_standy_init(void)
{
	printf("初始化待机室界面...\n");
	// 加载时钟相关资源
	resource bg_res = resource_get(ROM_R_IMG_CLOCK_CLOCK_PNG);	 // 时钟背景
	resource dot_res = resource_get(ROM_R_IMG_CLOCK_POINT_PNG);	 // 时钟点
	resource hour_res = resource_get(ROM_R_IMG_CLOCK_HOUR_PNG);	 // 小时指针
	resource min_res = resource_get(ROM_R_IMG_CLOCK_MINUTE_PNG); // 分钟指针
	resource sec_res = resource_get(ROM_R_IMG_CLOCK_S_PNG);		 // 秒针

	// 配置时钟结构体参数
	standy_clock.bg_res = bg_res;
	standy_clock.dot_res = dot_res;
	standy_clock.hour_res = hour_res;
	standy_clock.min_res = min_res;
	standy_clock.sec_res = sec_res;
	standy_clock.hour_center_y = 56; // 小时指针中心点Y坐标
	standy_clock.min_center_y = 67;	 // 分钟指针中心点Y坐标
	standy_clock.sec_center_y = 85;	 // 秒针中心点Y坐标

	// 检查资源是否有效
	LOG_WHITE("bg_res id: %ld, size: %d\n\r", bg_res.id, bg_res.size);
	LOG_WHITE("hour_res id: %ld, size: %d\n\r", hour_res.id, hour_res.size);
	position pos = {{121, 16}, {238, 238}};
	standy_clock.pos = pos;
}

STR_StandbyClass StandbyClass = {
	standby_bg_display,
	standby_key_register,
	standby_data_init,
	WELOCOME_BG,
};

layout layout_standby = {

	.init = layout_standy_init,
	.enter = layout_standby_enter,
	.quit = layout_standby_quit,
	.timer = layout_standby_timer};
