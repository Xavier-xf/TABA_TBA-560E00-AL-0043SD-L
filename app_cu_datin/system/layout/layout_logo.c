#include "os_sys_api.h"
#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "gpio_control.h"

static int logo_delay_count = 0;

/*************************************************************************
 * @brief  显示版本号
 * @date   2022-08-17 11:58
 * @author xiaole
 **************************************************************************/
void version_txt_display(void)
{
	text version;
	position pos = {{240 - 16, 227}, {240, 45}};

	text_init(&version, &pos, 18);
	version.align = RIGHT_MIDDLE;
	text_display(&version, SYSTEM_VERSION);
}

/*************************************************************************
 * @brief  显示开机图片
 * @date   2022-08-17 11:58
 **************************************************************************/
static void logo_display(void)
{
	icon logo;
	position pos = {{148, 78}, {182, 85}};
	resource res = resource_get(ROM_R_IMG_TB_LG_PNG);
	icon_init(&logo, &pos, &res);
	icon_display(&logo);
	version_txt_display();
}

static void gpio_init(void)
{
	back_light_gpio_control(true);
	intercom_open_door(false);
	camera_led_gpio_control(false);
	camera_power_gpio_control(false);
	amplifier_gpio_control(false);
	mic_mute_gpio_control(false);
}

void printf_user_data(void)
{
	printf("===============================\n\r");
	printf("home id index = %d\n\r", get_int_conf(SET_HOME_ID_INDEX));

	/* 因该是遍历home id */
	for (unsigned char i = 0; i < get_int_conf(SET_HOME_ID_INDEX); i++)
	{
		printf("user home id[%d] = %d\n\r", i, UserData.home_id[i]);
	}

	printf("unit number index = %d\n\r", get_int_conf(UNIT_NUMBER_INDEX));
	/*  应该是遍历unit */
	for (unsigned char i = 0; i < get_int_conf(UNIT_NUMBER_INDEX); i++)
	{
		printf("user unit number[%d] = %d\n\r", i, UserData.unit_number[i]);
	}
	printf("===============================\n\r");
}

// bool key_num_flag = false;

static void layout_logo_enter(void)
{
	LOG_WHITE(">>> enter layout logo \n\r");

	// key_num_flag = false;
	bg_img_init();
	logo_display();
	user_configure_init();
	user_data_init();
	card_id_data_init();
	printf_user_data();
	gpio_init();
	layout_init();

	// 擦除屏幕底部区域（例如从y=270到272，覆盖可能的残留线条）
	position bottom_area = {{0, 270}, {480, 2}};
	gui_erase(&bottom_area, 0x00000000); // 用背景色擦除

	LOG_BLUE("language is %d \n", language_get());

	struct ak_date time;
	ak_get_localdate(&time);
	if (language_get() == language_persian)
	{
		long date_array[3];
		gregorian_to_jalali(time.year, time.month + 1, time.day + 1, date_array);
		if (date_array[0] <= 1399)
		{
			time.year = date_array[0];
			time.month = date_array[1] - 1;
			time.day = date_array[2] - 1;
			date_set(&time);
		}
	}
	else
	{
		if (time.year <= 2021)
		{
			time.year = 2022;
			date_set(&time);
		}
	}
	font_file_reload();
}

static void layout_logo_quit(void)
{
	LOG_WHITE(">>> quit layout logo \n\r");
}

static void layout_logo_timer(void)
{

	logo_delay_count++;

	if (logo_delay_count > 80)
	{
		logo_delay_count = 0;
		amplifier_gpio_control(true);
		os_layout_goto(&layout_dial);
	}
	usleep(10 * 1000);
}

layout layout_logo = {

	.enter = layout_logo_enter,
	.quit = layout_logo_quit,
	.timer = layout_logo_timer};
