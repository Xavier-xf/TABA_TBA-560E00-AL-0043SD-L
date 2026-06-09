#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "string.h"
#include "gpio_control.h"
#include "leo_audio_play.h"

static char password_error_count = 0;
static unsigned long long error_delay_start_time = 0;
static bool in_delay_mode = false;

/* ********** widget display fun start ********** */
/*********************************************************************************************************
 * 函 数 名 : password_dialog_display
 * 功能说明 : 显示密码输入相关的对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据触发模式决定是否显示第二个密码对话框
 *********************************************************************************************************/
static void password_dialog_display(void)
{
	widget_dialog_box_display(PasswordClass.dialog_box[ENUM_PASSWORD_DIALOG_BOX1]);
	if (PasswordClass.trigger_mode != INPUT_UNLOCK && PasswordClass.trigger_mode != INPUT_SETTING)
		widget_dialog_box_display(PasswordClass.dialog_box[ENUM_PASSWORD_DIALOG_BOX2]);
}

/*********************************************************************************************************
 * 函 数 名 : password_icon_display
 * 功能说明 : 显示密码输入界面的图标（包含taba按钮）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 无
 *********************************************************************************************************/
static void password_icon_display(void)
{
	taba_btn_display();
}

/*********************************************************************************************************
 * 函 数 名 : input_password_font_display
 * 功能说明 : 显示"输入密码"文字提示，适配英语和波斯语布局
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 英语和波斯语分别使用不同的位置和字体大小
 *********************************************************************************************************/
static void input_password_font_display(void)
{
	position pos = {{33, 30}, {159, 32}};
	text input_password;

	text_init(&input_password, &pos, 26);
	input_password.align = LEFT_MIDDLE;
	text_display(&input_password, font_str(STR_PASSWORD_INPUT_PASSWORD));
}

/*********************************************************************************************************
 * 函 数 名 : input_old_password_font_display
 * 功能说明 : 显示"输入旧密码"文字提示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 统一布局，适配多语言文本字符串
 *********************************************************************************************************/
static void input_old_password_font_display(void)
{
	position pos = {{33, 30}, {320, 32}};
	text old_password;

	text_init(&old_password, &pos, 26);
	old_password.align = LEFT_MIDDLE;
	text_display(&old_password, font_str(STR_PASSWORD_INPUT_OLD_PASSWORD));
}

/*********************************************************************************************************
 * 函 数 名 : input_new_password_font_display
 * 功能说明 : 显示"输入新密码"文字提示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 布局位置与旧密码提示区分开，便于用户识别
 *********************************************************************************************************/
static void input_new_password_font_display(void)
{
	position pos = {{33, 120}, {320, 32}};
	text new_password;

	text_init(&new_password, &pos, 26);
	new_password.align = LEFT_MIDDLE;
	text_display(&new_password, font_str(STR_PASSWORD_INPUT_NEW_PASSWORD));
}

/*********************************************************************************************************
 * 函 数 名 : password_wellcome_font_display
 * 功能说明 : 显示密码验证成功后的欢迎提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 开门成功后显示该提示
 *********************************************************************************************************/
static void password_wellcome_font_display(void)
{
	position pos = {{111, 145}, {258, 45}};
	text welcome;

	text_init(&welcome, &pos, 22);
	text_display(&welcome, font_str(STR_PASSWORD_WELCOME));
}

/*********************************************************************************************************
 * 函 数 名 : password_error_font_display
 * 功能说明 : 显示密码输入错误的提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 密码验证失败时触发显示
 *********************************************************************************************************/
static void password_error_font_display(void)
{
	position pos = {{111, 145}, {258, 45}};
	text password_error;

	text_init(&password_error, &pos, 22);
	text_display(&password_error, font_str(STR_PASSWORD_ERROR));
}

/*********************************************************************************************************
 * 函 数 名 : clear_old_password_font
 * 功能说明 : 清除旧密码输入框区域的文字显示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据当前焦点位置确定清除区域
 *********************************************************************************************************/
static void clear_old_password_font(void)
{
	position pos;
	if (PasswordClass.cur_focus == ENUM_PASSWORD_DIALOG_BOX1)
	{
		pos.point.x = 33;
		pos.point.y = 57;
		pos.vector.width = 150;
		pos.vector.height = 40;
	}
	else
	{
		pos.point.x = 33;
		pos.point.y = 150;
		pos.vector.width = 150;
		pos.vector.height = 40;
	}

	gui_erase(&pos, 0x00000000);
}

/*********************************************************************************************************
 * 函 数 名 : clear_password_error_font
 * 功能说明 : 清除密码错误提示文字区域的显示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 使用黑色背景擦除指定区域
 *********************************************************************************************************/
static void clear_password_error_font(void)
{
	position pos = {{111, 150}, {258, 45}};
	gui_erase(&pos, 0x00000000);
}

/*********************************************************************************************************
 * 函 数 名 : clear_delay_message_font
 * 功能说明 : 清除密码输入错误后的延迟提示文字区域显示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 延迟倒计时结束后调用该函数清除提示
 *********************************************************************************************************/
static void clear_delay_message_font(void)
{
	position pos = {{111, 150}, {258, 40}};
	gui_erase(&pos, 0x00000000);
}
/* ********** widget display fun end ********** */

/*********************************************************************************************************
 * 函 数 名 : password_font_display
 * 功能说明 : 根据密码触发模式显示对应的密码提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 不同触发模式对应显示旧密码/新密码/普通密码提示
 *********************************************************************************************************/
static void password_font_display(void)
{
	switch (PasswordClass.trigger_mode)
	{
	case INPUT_UNLOCK:
	case INPUT_SETTING:
		input_password_font_display();
		break;
	case INPUT_OLD_UNLOCK:
	case INPUT_OLD_SETTING:
	case INPUT_NEW_UNLOCK:
	case INPUT_NEW_SETTING:
		input_old_password_font_display();
		input_new_password_font_display();
		break;
	case INPUT_NEW_UNLOCK_AGAIN:
	case INPUT_NEW_SETTING_AGAIN:
		break;
	default:
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : input_password_dialog_box_init
 * 功能说明 : 初始化第一个密码输入对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 包含图标、字体、光标等基础属性初始化
 *********************************************************************************************************/
static void input_password_dialog_box_init(void)
{
	icon box;
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 5};
	static char string[20];

	position box_pos = {{33, 57}, {258, 56}};
	icon_init(&box, &box_pos, NULL);
	box.res.id = 0;
	position pos = {{33, 57}, {180, 50}};
	text_init(&font.text1, &pos, 40);
	font.string1 = string;
	font.text1.align = RIGHT_MIDDLE;

	widget_dialog_box_init(PasswordClass.dialog_box[ENUM_PASSWORD_DIALOG_BOX1],
						   &box,
						   &font,
						   &cursor);
}

/*********************************************************************************************************
 * 函 数 名 : input_password_dialog_box2_init
 * 功能说明 : 初始化第二个密码输入对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 布局位置与第一个对话框区分，用于新旧密码二次输入场景
 *********************************************************************************************************/
static void input_password_dialog_box2_init(void)
{
	icon box;
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 5};
	static char string[20];

	position box_pos = {{33, 150}, {258, 56}};
	icon_init(&box, &box_pos, NULL);
	box.res.id = 0;
	position pos = {{33, 150}, {180, 50}};
	text_init(&font.text1, &pos, 40);
	font.string1 = string;
	font.text1.align = RIGHT_MIDDLE;

	widget_dialog_box_init(PasswordClass.dialog_box[ENUM_PASSWORD_DIALOG_BOX2],
						   &box,
						   &font,
						   &cursor);
}

/*********************************************************************************************************
 * 函 数 名 : layout_password_init
 * 功能说明 : 密码输入布局的初始化入口函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 统一初始化两个密码对话框控件
 *********************************************************************************************************/
static void layout_password_init(void)
{
	input_password_dialog_box_init();
	input_password_dialog_box2_init();
}

/*********************************************************************************************************
 * 函 数 名 : layout_password_enter
 * 功能说明 : 进入密码输入布局时的初始化操作
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 包含控件显示、按键注册、待机定时器启动等操作
 *********************************************************************************************************/
static void layout_password_enter(void)
{
	LOG_WHITE(">>> enter layout password \n\r");

	PasswordClass.widget_show.icon();		// 显示图标
	PasswordClass.widget_show.font();		// 显示文字
	PasswordClass.widget_show.dialog_box(); // 显示对话框
	PasswordClass.key_register();			// 注册按键
	standby_timer_open(15 * 1000, goto_layout_standby);
}

/*********************************************************************************************************
 * 函 数 名 : password_dialog_number_init
 * 功能说明 : 重置密码对话框的输入状态
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 清空输入字符串并重置光标索引
 *********************************************************************************************************/
static void password_dialog_number_init(void)
{
	PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index = 0;
	memset(PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1, 0, 15);
}

/*********************************************************************************************************
 * 函 数 名 : layout_password_quit
 * 功能说明 : 退出密码输入布局时的清理操作
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 重置密码输入状态、焦点位置并重启待机定时器
 *********************************************************************************************************/
static void layout_password_quit(void)
{
	LOG_WHITE(">>> quit layout password \n\r");
	password_dialog_number_init();
	PasswordClass.cur_focus = ENUM_PASSWORD_DIALOG_BOX1;
	standby_timer_open(60 * 1000, goto_layout_standby);
}

/*********************************************************************************************************
 * 函 数 名 : display_delay_message
 * 功能说明 : 显示密码输入错误后的延迟提示信息（倒计时）
 * 形    参 : delay_seconds：需要延迟的秒数
 * 返 回 值 : 无
 * 备    注 : 先擦除原有区域再显示新的倒计时文字
 *********************************************************************************************************/
static void display_delay_message(int delay_seconds)
{
	position pos = {{111, 150}, {258, 40}};
	text delay_msg;
	char delay_str[64];
	const char *delay_template = font_str(STR_PASSWORD_WAIT_SECONDS);

	if (language_get() == language_persian)
	{
		pos = (position){{70, 150}, {340, 40}};
	}

	gui_erase(&pos, 0x00000000);
	text_init(&delay_msg, &pos, 20);
	snprintf(delay_str, sizeof(delay_str), delay_template, delay_seconds);
	text_display(&delay_msg, delay_str);
}

/*********************************************************************************************************
 * 函 数 名 : get_system_time
 * 功能说明 : 获取系统当前时间（毫秒级）
 * 形    参 : 无
 * 返 回 值 : 无符号长整型：当前系统毫秒时间戳
 * 备    注 : 依赖os_get_ms()接口实现
 *********************************************************************************************************/
static unsigned long get_system_time(void)
{
	return os_get_ms(); // 确保这个函数返回毫秒
}

/*********************************************************************************************************
 * 函 数 名 : update_delay_countdown
 * 功能说明 : 实时更新密码输入错误后的延迟倒计时显示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 延迟结束后自动清除延迟状态和提示文字
 *********************************************************************************************************/
static void update_delay_countdown(void)
{
	if (!in_delay_mode)
		return;

	unsigned long current_time = get_system_time();
	unsigned long delay_time = 30 * 1000 * (password_error_count - 2);
	unsigned long elapsed_time = current_time - error_delay_start_time;

	if (elapsed_time >= delay_time)
	{
		// 延迟结束
		in_delay_mode = false;
		clear_delay_message_font();
		LOG_WHITE("Password delay period ended, can retry now\n");
		return;
	}

	// 计算并显示剩余时间
	unsigned long remaining_time = delay_time - elapsed_time;
	int remaining_seconds = (remaining_time + 500) / 1000; // 四舍五入到秒
	display_delay_message(remaining_seconds);
}

/*********************************************************************************************************
 * 函 数 名 : password_error_status_font_display
 * 功能说明 : 处理密码验证状态的显示逻辑（错误/成功/欢迎/延迟）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 包含错误计数、延迟处理、状态重置和布局跳转逻辑
 *********************************************************************************************************/
static void password_error_status_font_display(void)
{
	static char show_password_status_count = 0;

	// 在延迟模式下，持续更新倒计时
	if (in_delay_mode)
	{
		update_delay_countdown();
		if (in_delay_mode)
		{
			return;
		}
	}
	if (PasswordClass.password_status != PASSWORD_STATUS_NONE)
	{
		if (show_password_status_count == 0)
		{
			if (PasswordClass.password_status == PASSWORD_ERROR)
			{
				clear_old_password_font();
				password_error_font_display();
				password_error_count++;
				if (password_error_count >= 3)
				{
					in_delay_mode = true;
					error_delay_start_time = get_system_time();
					int delay_seconds = 30 * (password_error_count - 2);
					LOG_WHITE("Too many password errors, delay %d seconds\n", delay_seconds);
					display_delay_message(delay_seconds);
				}
			}
			else if (PasswordClass.password_status == PASSWORD_OLD_ERROR)
			{
				clear_old_password_font();
				password_error_font_display();
				password_error_count++;
				if (password_error_count >= 3)
				{
					in_delay_mode = true;
					error_delay_start_time = get_system_time();
					int delay_seconds = 30 * (password_error_count - 2);
					LOG_WHITE("Too many password errors, delay %d seconds\n", delay_seconds);
					display_delay_message(delay_seconds);
				}
			}
			else if (PasswordClass.password_status == PASSWORD_SUCCESS)
			{
				password_error_count = 0;
				in_delay_mode = false;
				LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			}
			else if (PasswordClass.password_status == PASSWORD_WELLCOME)
			{
				swiping_card_sound_play(); // 开门铃声 同 刷卡
				password_wellcome_font_display();
				LOG_WHITE("open the door \n");
				card_pwd_open_door(true);
				password_error_count = 0;
				in_delay_mode = false;
				LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			}
			password_dialog_number_init();
		}

		show_password_status_count++;
		if (show_password_status_count >= 30)
		{
			clear_password_error_font();
			show_password_status_count = 0;
			int mode = PasswordClass.trigger_mode;
			switch (mode)
			{
			case INPUT_UNLOCK:
				LOG_WHITE("close the door \n");
				card_pwd_open_door(false);
				os_layout_goto(&layout_dial);
				break;
			case INPUT_NEW_UNLOCK:
				if (PasswordClass.password_status == PASSWORD_ERROR)
				{
					PasswordClass.widget_show.font();
				}
				else if (PasswordClass.password_status == PASSWORD_SUCCESS)
				{
					password_error_count = 0;
					in_delay_mode = false;
					LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
					os_layout_goto(&layout_settings);
				}
				break;
			case INPUT_NEW_SETTING:
				if (PasswordClass.password_status == PASSWORD_ERROR)
				{
					PasswordClass.widget_show.font();
				}
				else if (PasswordClass.password_status == PASSWORD_SUCCESS)
				{
					password_error_count = 0;
					in_delay_mode = false;
					LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
					os_layout_goto(&layout_settings);
				}
				break;
			}
			PasswordClass.password_status = PASSWORD_STATUS_NONE;
		}
	}
}

/*********************************************************************************************************
 * 函 数 名 : layout_password_timer
 * 功能说明 : 密码布局的定时器回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 定时处理密码验证状态的显示逻辑
 *********************************************************************************************************/
static void layout_password_timer(void)
{
	password_error_status_font_display();
}

/*********************************************************************************************************
 * 函 数 名 : password_star_display
 * 功能说明 : 显示密码输入的星号掩码（替代明文显示）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 适配英语/波斯语字体大小，根据焦点位置调整显示区域
 *********************************************************************************************************/
static void password_star_display(void)
{
	text text_date;
	char string[20] = {0};
	position pos;
	unsigned char index = PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index;
	// 根据语言设置字体大小：波斯语35，英语40
	int font_size = (language_get() == language_persian) ? 34 : 40;

	// 根据当前焦点设置位置
	if (PasswordClass.cur_focus == ENUM_PASSWORD_DIALOG_BOX1)
	{
		pos.point.x = 33;
		pos.point.y = 57;
		pos.vector.width = 150;
		pos.vector.height = 40;
	}
	else
	{
		pos.point.x = 33;
		pos.point.y = 150;
		pos.vector.width = 150;
		pos.vector.height = 40;
	}

	// 生成星号字符串
	for (char i = 0; i < index; i++)
	{
		if (i != 0)
		{
			strcat(string, " ");
		}
		strcat(string, "*");
	}

	gui_erase(&pos, 0xFF20428A);
	text_init(&text_date, &pos, font_size); // 使用动态设置的字体大小
	text_date.align = LEFT_TOP;
	text_display(&text_date, string);
}

/*********************************************************************************************************
 * 函 数 名 : check_password
 * 功能说明 : 验证输入的密码是否正确
 * 形    参 : 无
 * 返 回 值 : 布尔型：true-密码正确；false-密码错误/处于延迟模式
 * 备    注 : 根据不同触发模式验证解锁/设置密码，新密码仅验证长度
 *********************************************************************************************************/
static bool check_password(void)
{
	if (in_delay_mode)
	{
		LOG_WHITE("In password delay mode, verification disabled\n");
		return false;
	}
	printf("===>>>input   password = %s\n\r", PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1);
	printf("===>>>unlock  password = %s\n\r", get_string_conf(UNLOCK_PASSWORD));
	printf("===>>>setting password = %s\n\r", get_string_conf(SET_PASSWORD));

	switch (PasswordClass.trigger_mode)
	{
	case INPUT_UNLOCK:
	case INPUT_OLD_UNLOCK:
		if (strcmp(PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1, get_string_conf(UNLOCK_PASSWORD)) == 0) // 0 表示两个数相等
			return true;
		else
			return false;
		break;
	case INPUT_NEW_UNLOCK:
	case INPUT_NEW_SETTING:
		if (PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index == 6)
			return true;
		else
			return false;
		break;
	case INPUT_NEW_UNLOCK_AGAIN:
	case INPUT_NEW_SETTING_AGAIN:
		if (strcmp(PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1, PasswordClass.new_password) == 0)
			return true;
		else
			return false;
		break;
	case INPUT_SETTING:
	case INPUT_OLD_SETTING:
		if (strcmp(PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1, get_string_conf(SET_PASSWORD)) == 0)
			return true;
		else
			return false;
		break;
	default:
		return false;
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : password_input_add_number
 * 功能说明 : 向密码输入框添加数字字符
 * 形    参 : string：要添加的数字字符串（单个数字）
 * 返 回 值 : 无
 * 备    注 : 延迟模式下禁止输入，输入完成后验证密码（解锁模式）
 *********************************************************************************************************/
static void password_input_add_number(const char *string)
{
	if (in_delay_mode)
	{
		LOG_WHITE("In password delay mode, input disabled\n");
		return;
	}
	unsigned char index = PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index;
	unsigned char max_index = PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.max_index;

	if (index > max_index)
		return;
	if (PasswordClass.password_status != PASSWORD_STATUS_NONE)
		return;

	if (index != 0)
	{
		strcat(PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1, " ");
	}
	strcat(PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1, string);
	PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index++;
	if (PasswordClass.trigger_mode == INPUT_UNLOCK)
	{
		if (index == max_index)
		{
			if (check_password() == false)
			{
				PasswordClass.password_status = PASSWORD_ERROR;
			}
			else
			{
				PasswordClass.password_status = PASSWORD_WELLCOME;
			}
		}
	}
	password_star_display();
}

/*********************************************************************************************************
 * 函 数 名 : password_input_sub_number
 * 功能说明 : 从密码输入框删除最后一个数字字符
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 删除后更新星号显示，索引为0时不执行操作
 *********************************************************************************************************/
static void password_input_sub_number(void)
{
	unsigned char index = PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index;

	if (index == 0)
		return;

	char string[20];
	strcpy(string, PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1);
	memset(PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1, 0, 20);
	PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index--;
	if (PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index != 0)
	{
		strncpy(PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1,
				string,
				PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index * 2 - 1);
	}

	password_star_display();
}

/******************key start******************/

/*********************************************************************************************************
 * 函 数 名 : password_key0_up
 * 功能说明 : 数字键0的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下0键时向密码框添加数字"0"
 *********************************************************************************************************/
static void password_key0_up(void)
{
	password_input_add_number("0");
}

/*********************************************************************************************************
 * 函 数 名 : password_key1_up
 * 功能说明 : 数字键1的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下1键时向密码框添加数字"1"
 *********************************************************************************************************/
static void password_key1_up(void)
{
	password_input_add_number("1");
}

/*********************************************************************************************************
 * 函 数 名 : password_key2_up
 * 功能说明 : 数字键2的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下2键时向密码框添加数字"2"
 *********************************************************************************************************/
static void password_key2_up(void)
{
	password_input_add_number("2");
}

/*********************************************************************************************************
 * 函 数 名 : password_key3_up
 * 功能说明 : 数字键3的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下3键时向密码框添加数字"3"
 *********************************************************************************************************/
static void password_key3_up(void)
{
	password_input_add_number("3");
}

/*********************************************************************************************************
 * 函 数 名 : password_key4_up
 * 功能说明 : 数字键4的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下4键时向密码框添加数字"4"
 *********************************************************************************************************/
static void password_key4_up(void)
{
	password_input_add_number("4");
}

/*********************************************************************************************************
 * 函 数 名 : password_key5_up
 * 功能说明 : 数字键5的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下5键时向密码框添加数字"5"
 *********************************************************************************************************/
static void password_key5_up(void)
{
	password_input_add_number("5");
}

/*********************************************************************************************************
 * 函 数 名 : password_key6_up
 * 功能说明 : 数字键6的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下6键时向密码框添加数字"6"
 *********************************************************************************************************/
static void password_key6_up(void)
{
	password_input_add_number("6");
}

/*********************************************************************************************************
 * 函 数 名 : password_key7_up
 * 功能说明 : 数字键7的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下7键时向密码框添加数字"7"
 *********************************************************************************************************/
static void password_key7_up(void)
{
	password_input_add_number("7");
}

/*********************************************************************************************************
 * 函 数 名 : password_key8_up
 * 功能说明 : 数字键8的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下8键时向密码框添加数字"8"
 *********************************************************************************************************/
static void password_key8_up(void)
{
	password_input_add_number("8");
}

/*********************************************************************************************************
 * 函 数 名 : password_key9_up
 * 功能说明 : 数字键9的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下9键时向密码框添加数字"9"
 *********************************************************************************************************/
static void password_key9_up(void)
{
	password_input_add_number("9");
}

/******************key end******************/

/*********************************************************************************************************
 * 函 数 名 : password_key_star_up
 * 功能说明 : 星号键(*)的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 输入框为空时返回上级布局，非空时删除最后一位密码
 *********************************************************************************************************/
static void password_key_star_up(void)
{
	if (key_long_press_goto_layout == DIAL_STAR_LONG_PRESS)
	{
		key_long_press_goto_layout = NONE_LONG_PRESS;
		return;
	}
	if (PasswordClass.trigger_mode == INPUT_UNLOCK || PasswordClass.trigger_mode == INPUT_SETTING)
	{

		if (PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index == 0)
		{

			os_layout_goto(&layout_dial);
		}
		else
		{

			password_input_sub_number();
		}
	}
	else
	{
		if (PasswordClass.dialog_box[PasswordClass.cur_focus]->cursor.index == 0)
		{
			os_layout_goto(&layout_settings);
		}
		else
		{
			password_input_sub_number();
		}
	}
}

/*********************************************************************************************************
 * 函 数 名 : password_key_ring_up
 * 功能说明 : 井号键(#)的按键回调函数（确认键）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据不同触发模式验证密码，处理解锁/修改密码逻辑
 *********************************************************************************************************/
static void password_key_ring_up(void)
{
	if (PasswordClass.password_status == PASSWORD_SUCCESS)
		return;
	switch (PasswordClass.trigger_mode)
	{
	case INPUT_UNLOCK:
		if (check_password() == false)
		{
			PasswordClass.password_status = PASSWORD_ERROR;
		}
		else
		{
			password_error_count = 0;
			in_delay_mode = false;
			LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
		}
		break;
	case INPUT_SETTING:
		if (check_password() == true)
		{
			password_error_count = 0;
			in_delay_mode = false;
			LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			os_layout_goto(&layout_settings);
		}
		else
		{
			PasswordClass.password_status = PASSWORD_ERROR;
		}
		break;
	case INPUT_OLD_UNLOCK:
		if (check_password() == true)
		{
			password_error_count = 0;
			in_delay_mode = false;
			LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			password_dialog_number_init();
			PasswordClass.trigger_mode = INPUT_NEW_UNLOCK;
			PasswordClass.cur_focus = ENUM_PASSWORD_DIALOG_BOX2;
		}
		else
		{
			PasswordClass.password_status = PASSWORD_OLD_ERROR;
		}
		break;
	case INPUT_NEW_UNLOCK:
		if (check_password() == true)
		{
			password_error_count = 0;
			in_delay_mode = false;
			LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			memset(PasswordClass.new_password, 0, 15);
			strcpy(PasswordClass.new_password, PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1);
			set_string_conf(UNLOCK_PASSWORD, PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1);
			password_dialog_number_init();

			PasswordClass.password_status = PASSWORD_SUCCESS;
		}
		else
		{
			PasswordClass.password_status = PASSWORD_ERROR;
		}
		break;
	case INPUT_NEW_UNLOCK_AGAIN:
		if (check_password() == true)
		{
			password_error_count = 0;
			in_delay_mode = false;
			LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			set_string_conf(UNLOCK_PASSWORD, PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1);
			PasswordClass.password_status = PASSWORD_SUCCESS;
		}
		else
		{
			PasswordClass.password_status = PASSWORD_ERROR;
		}
		break;
	case INPUT_OLD_SETTING:
		if (check_password() == true)
		{
			password_error_count = 0;
			in_delay_mode = false;
			LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			password_dialog_number_init();
			PasswordClass.trigger_mode = INPUT_NEW_SETTING;
			PasswordClass.cur_focus = ENUM_PASSWORD_DIALOG_BOX2;
		}
		else
		{
			PasswordClass.password_status = PASSWORD_OLD_ERROR;
		}
		break;
	case INPUT_NEW_SETTING:
		if (check_password() == true)
		{
			password_error_count = 0;
			in_delay_mode = false;
			LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			memset(PasswordClass.new_password, 0, 15);
			strcpy(PasswordClass.new_password, PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1);
			set_string_conf(SET_PASSWORD, PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1);
			password_dialog_number_init();
			PasswordClass.password_status = PASSWORD_SUCCESS;
		}
		else
		{
			PasswordClass.password_status = PASSWORD_ERROR;
		}
		break;
	case INPUT_NEW_SETTING_AGAIN:
		if (check_password() == true)
		{
			password_error_count = 0;
			in_delay_mode = false;
			LOG_WHITE("=== PASSWORD SUCCESS - reset error count to 0 ===\n");
			set_string_conf(SET_PASSWORD, PasswordClass.dialog_box[PasswordClass.cur_focus]->font.string1);
			PasswordClass.password_status = PASSWORD_SUCCESS;
		}
		else
		{
			PasswordClass.password_status = PASSWORD_ERROR;
		}
		break;
	default:
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : password_key_register
 * 功能说明 : 注册密码输入界面的所有按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 包含数字键、星号键、井号键的按下事件注册
 *********************************************************************************************************/
static void password_key_register(void)
{
	key_touch_handle_register(KEY_INDEX_0, KEY_STATE_DOWN, password_key0_up);
	key_touch_handle_register(KEY_INDEX_1, KEY_STATE_DOWN, password_key1_up);
	key_touch_handle_register(KEY_INDEX_2, KEY_STATE_DOWN, password_key2_up);
	key_touch_handle_register(KEY_INDEX_3, KEY_STATE_DOWN, password_key3_up);
	key_touch_handle_register(KEY_INDEX_4, KEY_STATE_DOWN, password_key4_up);
	key_touch_handle_register(KEY_INDEX_5, KEY_STATE_DOWN, password_key5_up);
	key_touch_handle_register(KEY_INDEX_6, KEY_STATE_DOWN, password_key6_up);
	key_touch_handle_register(KEY_INDEX_7, KEY_STATE_DOWN, password_key7_up);
	key_touch_handle_register(KEY_INDEX_8, KEY_STATE_DOWN, password_key8_up);
	key_touch_handle_register(KEY_INDEX_9, KEY_STATE_DOWN, password_key9_up);
	//	key_touch_handle_register(KEY_INDEX_UP,    KEY_STATE_DOWN,   password_key_up_up);
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, password_key_star_up);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, password_key_ring_up);
}

STR_WidgetDialogBox PasswordDialogBox1;
STR_WidgetDialogBox PasswordDialogBox2;
STR_WidgetDialogBox *PasswordDialogBox[ENUM_PASSWORD_DIALOG_BOX_TOTAL] = {
	&PasswordDialogBox1,
	&PasswordDialogBox2};

STR_PasswordClass PasswordClass = {
	{password_icon_display, password_font_display, NULL, password_dialog_display, NULL},
	PasswordDialogBox,
	password_key_register,
	INPUT_UNLOCK,
	PASSWORD_STATUS_NONE,
	ENUM_PASSWORD_DIALOG_BOX1,
	{0}};

layout layout_password = {
	.init = layout_password_init,
	.enter = layout_password_enter,
	.quit = layout_password_quit,
	.timer = layout_password_timer};
