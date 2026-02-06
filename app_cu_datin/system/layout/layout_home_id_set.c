/*********************************************************************************************************
 * 函 数 名 : password_key0_up
 * 功能说明 : 数字键0的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下0键时向密码框添加数字"0"
 *********************************************************************************************************/

#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "string.h"
#include "intercom.h"

// 房号设置焦点位置数组（4个输入框的坐标和尺寸）
static const position HomeIdSetFocusPos[HOME_ID_SET_TOTAL_DIALOG_BOX] =
	{
		{{24, 113}, {92, 60}},
		{{140, 113}, {92, 60}},
		{{256, 113}, {92, 60}},
		{{372, 113}, {92, 60}}};

static void OUT_font_display(void)
{
	position pos = {{40, 15}, {150, 40}};
	text system_set;
	char string[20] = {0};
	text_init(&system_set, &pos, 26);
	system_set.align = LEFT_TOP;

	int brancher_id = OutPUTClass.auth_info.brancher_seq[0] * 100 +
					  OutPUTClass.auth_info.brancher_seq[1] * 10 +
					  OutPUTClass.auth_info.brancher_seq[2] * 1;
	sprintf(string, "%s   %d", (char *)font_str(STR_HOME_ID_OUT_ID), brancher_id);
	text_display(&system_set, string);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_m1_font_display
 * 功能说明 : 显示房号设置M1输入框的标题文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 定位在(24,59)，字体大小20
 *********************************************************************************************************/
static void home_id_set_m1_font_display(void)
{
	position pos = {{24, 59}, {92, 50}};
	text m1_text;

	text_init(&m1_text, &pos, 20);
	text_display(&m1_text, font_str(STR_HOME_ID_SET_M1));
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_m2_font_display
 * 功能说明 : 显示房号设置M2输入框的标题文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 定位在(140,59)，字体大小20
 *********************************************************************************************************/
static void home_id_set_m2_font_display(void)
{
	position pos = {{140, 59}, {92, 50}};
	text m2_text;

	text_init(&m2_text, &pos, 20);
	text_display(&m2_text, font_str(STR_HOME_ID_SET_M2));
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_m3_font_display
 * 功能说明 : 显示房号设置M3输入框的标题文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 定位在(256,59)，字体大小20
 *********************************************************************************************************/
static void home_id_set_m3_font_display(void)
{
	position pos = {{256, 59}, {92, 50}};
	text m3_text;

	text_init(&m3_text, &pos, 20);
	text_display(&m3_text, font_str(STR_HOME_ID_SET_M3));
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_m4_font_display
 * 功能说明 : 显示房号设置M4输入框的标题文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 定位在(372,59)，字体大小20
 *********************************************************************************************************/
static void home_id_set_m4_font_display(void)
{
	position pos = {{372, 59}, {92, 50}};
	text m4_text;

	text_init(&m4_text, &pos, 20);
	text_display(&m4_text, font_str(STR_HOME_ID_SET_M4));
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_success_font_display
 * 功能说明 : 显示房号设置成功的提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 居中显示(197,190)，字体大小20
 *********************************************************************************************************/
static void home_id_set_success_font_display(void)
{
	text m1_text;
	position pos = {{380, 15}, {92, 50}};

	text_init(&m1_text, &pos, 20);
	text_display(&m1_text, font_str(STR_HOME_ID_SET_SUCCESS));
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_fail_font_display
 * 功能说明 : 显示房号设置失败的提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 居中显示(197,190)，字体大小20
 *********************************************************************************************************/
static void home_id_set_fail_font_display(void)
{
	text m1_text;
	position pos = {{380, 15}, {92, 50}};

	text_init(&m1_text, &pos, 20);
	text_display(&m1_text, font_str(STR_HOME_ID_SET_FAIL));
}

/*********************************************************************************************************
 * 函 数 名 : home_id_already_exists_font_display
 * 功能说明 : 显示房号已存在的提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 居中显示(197,190)，字体大小20
 *********************************************************************************************************/
static void home_id_already_exists_font_display(void)
{
	text m1_text;
	position pos = {{380, 15}, {92, 50}};

	text_init(&m1_text, &pos, 20);
	text_display(&m1_text, font_str(STR_HOME_ID_SET_EXISTED));
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_status_font_erase
 * 功能说明 : 清除房号设置状态提示文字（成功/失败/已存在）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 清除(197,190)区域的显示内容，填充色0x00
 *********************************************************************************************************/
static void home_id_set_status_font_erase(void)
{
	position pos = {{380, 15}, {92, 50}};

	gui_erase(&pos, 0x00);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_font_display
 * 功能说明 : 批量显示所有房号设置输入框的标题文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 依次调用M1-M4的标题显示函数
 *********************************************************************************************************/
static void home_id_set_font_display(void)
{
	OUT_font_display();
	home_id_set_m1_font_display();
	home_id_set_m2_font_display();
	home_id_set_m3_font_display();
	home_id_set_m4_font_display();
}
static void home_id_set_icon_display(void)
{
	taba_btn_display();
}
/*********************************************************************************************************
 * 函 数 名 : clear_prev_home_id_set_focus
 * 功能说明 : 清除上一个焦点输入框的高亮显示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据当前焦点位置清除对应区域的焦点图标
 *********************************************************************************************************/
static void clear_prev_home_id_set_focus(void)
{
	gui_erase(&HomeIdSetFocusPos[HomeIdSetClass.cur_focus], 0x00);
}

/*********************************************************************************************************
 * 函 数 名 : display_current_home_id_set_focus
 * 功能说明 : 显示当前焦点输入框的高亮图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 加载焦点图标资源并显示在当前焦点位置
 *********************************************************************************************************/
static void display_current_home_id_set_focus(void)
{
	icon focus;

	resource res = resource_get(ROM_R_IMG_ROOM_ID_SET_FOCUS_PNG);
	icon_init(&focus, &HomeIdSetFocusPos[HomeIdSetClass.cur_focus], &res);
	icon_display(&focus);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_focus_display
 * 功能说明 : 初始化显示房号设置的焦点相关元素
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 先显示标签按钮，再显示当前焦点
 *********************************************************************************************************/
static void home_id_set_focus_display(void)
{
	display_current_home_id_set_focus();
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_dialog_display
 * 功能说明 : 显示所有4个房号设置输入框的对话框
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 依次显示M1-M4输入框的对话框控件
 *********************************************************************************************************/
static void home_id_set_dialog_display(void)
{
	widget_dialog_box_display(HomeIdSetClass.dialog_box[HOME_ID_SET_M1_DIALOG_BOX]);
	widget_dialog_box_display(HomeIdSetClass.dialog_box[HOME_ID_SET_M2_DIALOG_BOX]);
	widget_dialog_box_display(HomeIdSetClass.dialog_box[HOME_ID_SET_M3_DIALOG_BOX]);
	widget_dialog_box_display(HomeIdSetClass.dialog_box[HOME_ID_SET_M4_DIALOG_BOX]);
}

/*********************************************************************************************************
 * 函 数 名 : display_prev_home_id_set_dialog
 * 功能说明 : 显示上一个焦点对应的房号输入框对话框
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据当前焦点位置显示对应对话框
 *********************************************************************************************************/
static void display_prev_home_id_set_dialog(void)
{
	widget_dialog_box_display(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]);
}

/*********************************************************************************************************
 * 函 数 名 : goto_prev_home_id_set_focus
 * 功能说明 : 将焦点切换到上一个房号输入框
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : M1→M4，M2→M1，M3→M2，M4→M3 循环切换
 *********************************************************************************************************/
static void goto_prev_home_id_set_focus(void)
{
	switch (HomeIdSetClass.cur_focus)
	{
	case HOME_ID_SET_M1_DIALOG_BOX:
		HomeIdSetClass.cur_focus = HOME_ID_SET_M4_DIALOG_BOX;
		break;
	case HOME_ID_SET_M2_DIALOG_BOX:
		HomeIdSetClass.cur_focus = HOME_ID_SET_M1_DIALOG_BOX;
		break;
	case HOME_ID_SET_M3_DIALOG_BOX:
		HomeIdSetClass.cur_focus = HOME_ID_SET_M2_DIALOG_BOX;
		break;
	case HOME_ID_SET_M4_DIALOG_BOX:
		HomeIdSetClass.cur_focus = HOME_ID_SET_M3_DIALOG_BOX;
		break;
	default:
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : goto_next_home_id_set_focus
 * 功能说明 : 将焦点切换到下一个房号输入框
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : M1→M2，M2→M3，M3→M4，M4→M1 循环切换
 *********************************************************************************************************/
static void goto_next_home_id_set_focus(void)
{
	switch (HomeIdSetClass.cur_focus)
	{
	case HOME_ID_SET_M1_DIALOG_BOX:
		HomeIdSetClass.cur_focus = HOME_ID_SET_M2_DIALOG_BOX;
		break;
	case HOME_ID_SET_M2_DIALOG_BOX:
		HomeIdSetClass.cur_focus = HOME_ID_SET_M3_DIALOG_BOX;
		break;
	case HOME_ID_SET_M3_DIALOG_BOX:
		HomeIdSetClass.cur_focus = HOME_ID_SET_M4_DIALOG_BOX;
		break;
	case HOME_ID_SET_M4_DIALOG_BOX:
		HomeIdSetClass.cur_focus = HOME_ID_SET_M1_DIALOG_BOX;
		break;

	default:
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_dialog_box_font_change
 * 功能说明 : 根据输入框光标位置更新显示的数字字符串
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 光标位置0清空，1-4位依次拼接对应数字（带空格分隔）
 *********************************************************************************************************/
static void home_id_set_dialog_box_font_change(void)
{
	switch (HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.index)
	{
	case 0:
		memset(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->font.string1, 0, 10);
		break;
	case 1:
		sprintf(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->font.string1,
				"%d",
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][0]);
		break;
	case 2:
		sprintf(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->font.string1,
				"%d %d",
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][0],
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][1]);
		break;
	case 3:
		sprintf(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->font.string1,
				"%d %d %d",
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][0],
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][1],
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][2]);
		break;
	case 4:
		sprintf(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->font.string1,
				"%d %d %d %d",
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][0],
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][1],
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][2],
				HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][3]);
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_sub_number
 * 功能说明 : 房号输入框退格（删除最后一位数字）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 光标位置>0时，清空当前位并将光标左移，更新显示字符串
 *********************************************************************************************************/
void home_id_set_sub_number(void)
{
	unsigned char index = HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.index;

	HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][index] = 0;
	if (index > 0)
	{
		HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.index--;
		home_id_set_dialog_box_font_change();
	}
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_input_number_display
 * 功能说明 : 更新显示当前焦点输入框的数字和焦点图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 先显示对话框，再显示焦点图标
 *********************************************************************************************************/
static void home_id_set_input_number_display(void)
{
	widget_dialog_box_display(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]);
	display_current_home_id_set_focus();
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_add_number
 * 功能说明 : 向当前焦点输入框添加指定数字
 * 形    参 : number - 要添加的数字（0-9）
 * 返 回 值 : 无
 * 备    注 : 光标未到最大值时，存入数字并右移光标，更新显示
 *********************************************************************************************************/
static void home_id_set_add_number(unsigned char number)
{
	unsigned char index = HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.index;
	unsigned char max_index = HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.max_index;

	HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][index] = number;

	if (index <= max_index)
	{
		HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.index++;
		home_id_set_dialog_box_font_change();
	}
	home_id_set_input_number_display();
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key0_up
 * 功能说明 : 数字键0的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字0
 *********************************************************************************************************/
static void home_id_set_key0_up(void)
{
	home_id_set_add_number(0);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key1_up
 * 功能说明 : 数字键1的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字1
 *********************************************************************************************************/
static void home_id_set_key1_up(void)
{
	home_id_set_add_number(1);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key2_up
 * 功能说明 : 数字键2的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字2
 *********************************************************************************************************/
static void home_id_set_key2_up(void)
{
	home_id_set_add_number(2);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key3_up
 * 功能说明 : 数字键3的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字3
 *********************************************************************************************************/
static void home_id_set_key3_up(void)
{
	home_id_set_add_number(3);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key4_up
 * 功能说明 : 数字键4的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字4
 *********************************************************************************************************/
static void home_id_set_key4_up(void)
{
	home_id_set_add_number(4);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key5_up
 * 功能说明 : 数字键5的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字5
 *********************************************************************************************************/
static void home_id_set_key5_up(void)
{
	home_id_set_add_number(5);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key6_up
 * 功能说明 : 数字键6的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字6
 *********************************************************************************************************/
static void home_id_set_key6_up(void)
{
	home_id_set_add_number(6);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key7_up
 * 功能说明 : 数字键7的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字7
 *********************************************************************************************************/
static void home_id_set_key7_up(void)
{
	home_id_set_add_number(7);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key8_up
 * 功能说明 : 数字键8的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字8
 *********************************************************************************************************/
static void home_id_set_key8_up(void)
{
	home_id_set_add_number(8);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key9_up
 * 功能说明 : 数字键9的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 向当前焦点输入框添加数字9
 *********************************************************************************************************/
static void home_id_set_key9_up(void)
{
	home_id_set_add_number(9);
}

/*********************************************************************************************************
 * 函 数 名 : printf_home_id
 * 功能说明 : 打印所有已设置的房号并检查重复
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 控制台输出SET_HOME_ID[0-3]的值，并检测重复项
 *********************************************************************************************************/
void printf_home_id(void)
{
	for (unsigned char k = 0; k <= 3; k++)
	{
		printf("SET_HOME_ID[%d] = %d\n\r", k, HomeIdSetClass.set_id[k]);
		for (unsigned char j = 0; j <= 3; j++)
		{
			if (j == k)
			{
				continue;
			}
			if (HomeIdSetClass.set_id[k] == HomeIdSetClass.set_id[j])
			{
				LOG_WHITE("set_id[%d]:%d == set_id[%d]:%d\n\r", k, HomeIdSetClass.set_id[k], j, HomeIdSetClass.set_id[j]);
				return;
			}
		}
	}
}

/*********************************************************************************************************
 * 函 数 名 : clear_current_show_home_id
 * 功能说明 : 清空当前焦点输入框的显示数字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 清空show_id数组对应位置，重置光标，清空显示字符串
 *********************************************************************************************************/
static void clear_current_show_home_id(void)
{
	memset(HomeIdSetClass.show_id[HomeIdSetClass.cur_focus], 0, sizeof(HomeIdSetClass.show_id[HomeIdSetClass.cur_focus]));
	HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.index = 0;
	memset(HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->font.string1, 0, 10);
}

/*********************************************************************************************************
 * 函 数 名 : clear_save_fail_show_home_id
 * 功能说明 : 清空保存失败的输入框显示并重新绘制
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 重置保存失败输入框的光标，并重新显示对话框
 *********************************************************************************************************/
static void clear_save_fail_show_home_id(void)
{
	LOG_WHITE("clear and redisplay\n\r");
	HomeIdSetClass.dialog_box[HomeIdSetClass.save_id_falg]->cursor.index = 0;

	widget_dialog_box_display(HomeIdSetClass.dialog_box[HomeIdSetClass.save_id_falg]);
}

/*********************************************************************************************************
 * 函 数 名 : set_home_id_adjust
 * 功能说明 : 将输入框的数字拼接为完整房号（十进制）
 * 形    参 : 无
 * 返 回 值 : bool - 拼接成功返回true，无输入返回false
 * 备    注 : 按位权(1/10/100/1000)将4位数字拼接为整数存入set_id
 *********************************************************************************************************/
static bool set_home_id_adjust(void)
{
	unsigned char i;
	unsigned int place[4] = {1, 10, 100, 1000};
	unsigned char index = HomeIdSetClass.dialog_box[HomeIdSetClass.cur_focus]->cursor.index;

	if (index == 0)
	{
		return false;
	}
	HomeIdSetClass.set_id[HomeIdSetClass.cur_focus] = 0;
	for (i = 0; i < index; i++)
	{
		HomeIdSetClass.set_id[HomeIdSetClass.cur_focus] +=
			HomeIdSetClass.show_id[HomeIdSetClass.cur_focus][i] * place[index - i - 1];
	}
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : is_set_home_id_repetition
 * 功能说明 : 检查当前房号是否与其他输入框重复
 * 形    参 : 无
 * 返 回 值 : bool - 重复返回true，否则返回false
 * 备    注 : 排除当前焦点，检查其他输入框非空时的数值重复
 *********************************************************************************************************/
static bool is_set_home_id_repetition(void)
{
	unsigned char i;
	unsigned int set_home_id;
	unsigned int cur_home_id = HomeIdSetClass.set_id[HomeIdSetClass.cur_focus];

	for (i = 0; i < HOME_ID_SET_TOTAL_DIALOG_BOX; i++)
	{
		if (i == HomeIdSetClass.cur_focus)
		{
			continue;
		}
		set_home_id = HomeIdSetClass.set_id[i];
		if ((set_home_id == cur_home_id) && (HomeIdSetClass.dialog_box[i]->cursor.index != 0))
		{
			return true;
		}
	}
	return false;
}

static int m_id[4] = {0};  // 当前输入框对应的已存房号索引
static int m_num[4] = {0}; // 当前输入框对应的已存房号数值
static int flagMid = 0;	   // 读取房号状态标志（0-未读，1-已读，保存成功后重置）

/*********************************************************************************************************
 * 函 数 名 : getTheNumBeforeInput
 * 功能说明 : 获取指定输入框对应的已保存房号数值
 * 形    参 : m - 输入框索引（0-3对应M1-M4）
 * 返 回 值 : int - 找到返回房号，未找到返回-1
 * 备    注 : 遍历UserData.home_id数组匹配set_id[m]的值
 *********************************************************************************************************/
static int getTheNumBeforeInput(int m)
{
	for (unsigned k = 0; k < get_int_conf(SET_HOME_ID_INDEX); k++)
	{
		if (HomeIdSetClass.set_id[m] == UserData.home_id[k])
		{
			return UserData.home_id[k];
		}
	}
	return -1;
}

/*********************************************************************************************************
 * 函 数 名 : getTheIDBeforeInput
 * 功能说明 : 获取指定输入框对应的已保存房号索引
 * 形    参 : m - 输入框索引（0-3对应M1-M4）
 * 返 回 值 : int - 找到返回索引，未找到返回-1
 * 备    注 : 遍历UserData.home_id数组匹配set_id[m]的值，返回其索引
 *********************************************************************************************************/
static int getTheIDBeforeInput(int m)
{
	for (unsigned k = 0; k < get_int_conf(SET_HOME_ID_INDEX); k++)
	{
		if (HomeIdSetClass.set_id[m] == UserData.home_id[k])
		{
			return k;
		}
	}
	return -1;
}

/*********************************************************************************************************
 * 函 数 名 : getTheIDAndNumBeforeInput
 * 功能说明 : 批量获取4个输入框对应的已存房号索引和数值
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : flagMid=0且已存房号≥4时执行，执行后flagMid置1
 *********************************************************************************************************/
static void getTheIDAndNumBeforeInput(void)
{
	if (flagMid == 0 && get_int_conf(SET_HOME_ID_INDEX) >= 4)
	{
		m_id[0] = getTheIDBeforeInput(0);
		m_num[0] = getTheNumBeforeInput(0);

		m_id[1] = getTheIDBeforeInput(1);
		m_num[1] = getTheNumBeforeInput(1);

		m_id[2] = getTheIDBeforeInput(2);
		m_num[2] = getTheNumBeforeInput(2);

		m_id[3] = getTheIDBeforeInput(3);
		m_num[3] = getTheNumBeforeInput(3);

		flagMid = 1;
	}
}

/*********************************************************************************************************
 * 函 数 名 : selectDialogTitle_display
 * 功能说明 : 显示房号已存在确认对话框的标题
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 显示在(0,145)区域，字体大小20
 *********************************************************************************************************/
static void selectDialogTitle_display(void)
{
	position pos = {{0, 155 - 10}, {480, 50}};
	text title;
	text_init(&title, &pos, 20);
	text_display(&title, font_str(STR_HOME_ID_SET_SELECT_DIALOG_BOX_TITLE1));
}

/*********************************************************************************************************
 * 函 数 名 : selectDialog_display
 * 功能说明 : 显示房号已存在的确认对话框（YES/NO）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据select状态设置YES/NO字体颜色（黄色高亮选中项）
 *********************************************************************************************************/
static void selectDialog_display(void)
{
	if (HomeIdSetClass.layer == LAYER_DIALOG)
	{
		LOG_WHITE("redisplay dialog\n");
		if (HomeIdSetClass.select == SELECT_YES)
		{
			HomeIdSetClass.selectDialog->font.text2.font_color = 0xFFFED606;
			HomeIdSetClass.selectDialog->font.text3.font_color = 0xFFFFFFFF;
		}
		else
		{
			HomeIdSetClass.selectDialog->font.text2.font_color = 0xFFFFFFFF;
			HomeIdSetClass.selectDialog->font.text3.font_color = 0xFFFED606;
		}
		widget_dialog_box_display(HomeIdSetClass.selectDialog);
	}

	selectDialogTitle_display();
}

/*********************************************************************************************************
 * 函 数 名 : home_id_exist
 * 功能说明 : 检查指定房号是否已存在于用户数据中
 * 形    参 : home_id - 要检查的房号
 * 返 回 值 : bool - 存在返回true，不存在返回false
 * 备    注 : 遍历UserData.home_id数组进行匹配
 *********************************************************************************************************/
static bool home_id_exist(int home_id);

/*********************************************************************************************************
 * 函 数 名 : is_same_as_other
 * 功能说明 : 检查当前保存的房号是否与其他输入框数值重复
 * 形    参 : 无
 * 返 回 值 : bool - 重复返回true，否则返回false
 * 备    注 : 对比当前save_id_falg对应的数值与m_num[0-3]
 *********************************************************************************************************/
static bool is_same_as_other(void)
{
	for (unsigned char i = 0; i <= 3; i++)
	{
		if (HomeIdSetClass.set_id[HomeIdSetClass.save_id_falg] == m_num[i])
		{
			LOG_WHITE("[M%d] same as [M%d] :%d  \n", HomeIdSetClass.save_id_falg + 1, i + 1, HomeIdSetClass.set_id[HomeIdSetClass.save_id_falg]);
			HomeIdSetClass.set_status = HOME_ID_SET_STATUS_FAIL;
			return true;
		}
	}
	return false;
}

/*********************************************************************************************************
 * 函 数 名 : isSameAsOtherInputBoxNumber
 * 功能说明 : 封装检查当前输入框数值与其他输入框重复的逻辑
 * 形    参 : 无
 * 返 回 值 : bool - 重复返回true，否则返回false
 * 备    注 : 调用is_same_as_other，兼容所有4个输入框的检查
 *********************************************************************************************************/
static bool isSameAsOtherInputBoxNumber(void)
{
	switch (HomeIdSetClass.save_id_falg)
	{
	case HOME_ID_SET_M1_DIALOG_BOX:
		return is_same_as_other();
		break;
	case HOME_ID_SET_M2_DIALOG_BOX:
		return is_same_as_other();
		break;
	case HOME_ID_SET_M3_DIALOG_BOX:
		return is_same_as_other();
		break;
	case HOME_ID_SET_M4_DIALOG_BOX:
		return is_same_as_other();
		break;
	default:
		break;
	}
	return false;
}

/*********************************************************************************************************
 * 函 数 名 : set_home_id_number
 * 功能说明 : 处理房号设置的核心逻辑（验证+保存/确认）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 依次执行：读取历史数据→验证数值→检查重复→检查已存在→触发保存/显示确认框
 *********************************************************************************************************/
static void set_home_id_number(void)
{
	getTheIDAndNumBeforeInput();

	HomeIdSetClass.save_id_falg = HomeIdSetClass.cur_focus;

	if (set_home_id_adjust() == false)
	{
		return;
	}

	if (isSameAsOtherInputBoxNumber() == true)
	{
		return;
	}

	if (is_set_home_id_repetition() == true)
	{
		LOG_WHITE("SET_STATUS_FAIL\n\r");
		return;
	}

	if (home_id_exist(HomeIdSetClass.set_id[HomeIdSetClass.save_id_falg]))
	{
		HomeIdSetClass.layer = LAYER_DIALOG;
		selectDialog_display();
		return;
	}

	Intercom.set_id();
	Intercom.status = INT_SET_HOME_ID;
}

/*********************************************************************************************************
 * 函 数 名 : refreshSelectDialog
 * 功能说明 : 清除确认对话框区域的显示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 清空(0,151)区域的显示内容，填充色0x00
 *********************************************************************************************************/
static void refreshSelectDialog(void)
{
	position pos = {{0, 151}, {480, 84}};
	gui_erase(&pos, 0x00);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key_up_up
 * 功能说明 : 上方向键的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 主图层：保存数值+切换上一个焦点；对话框图层：切换YES/NO选中状态
 *********************************************************************************************************/
static void home_id_set_key_up_up(void)
{
	switch (HomeIdSetClass.layer)
	{
	case LAYER_MAIN:
		if (HomeIdSetClass.set_status != HOME_ID_SET_STATUS_NONE)
			return;

		set_home_id_number();
		clear_prev_home_id_set_focus();
		display_prev_home_id_set_dialog();
		goto_prev_home_id_set_focus();
		clear_current_show_home_id();
		home_id_set_input_number_display();
		break;

	case LAYER_DIALOG:
		if (HomeIdSetClass.select == SELECT_YES)
		{
			HomeIdSetClass.select = SELECT_NO;
		}
		else
		{
			HomeIdSetClass.select = SELECT_YES;
		}
		selectDialog_display();
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key_down_up
 * 功能说明 : 下方向键的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 主图层：保存数值+切换下一个焦点；对话框图层：切换YES/NO选中状态
 *********************************************************************************************************/
static void home_id_set_key_down_up(void)
{
	switch (HomeIdSetClass.layer)
	{
	case LAYER_MAIN:
		if (HomeIdSetClass.set_status != HOME_ID_SET_STATUS_NONE)
			return;

		set_home_id_number();
		if (HomeIdSetClass.layer == LAYER_MAIN)
		{
			clear_prev_home_id_set_focus();
			display_prev_home_id_set_dialog();
			goto_next_home_id_set_focus();
			clear_current_show_home_id();
			home_id_set_input_number_display();
		}
		break;

	case LAYER_DIALOG:
		LOG_WHITE("LAYER_DIALOG\n");
		if (HomeIdSetClass.select == SELECT_YES)
		{
			HomeIdSetClass.select = SELECT_NO;
		}
		else
		{
			HomeIdSetClass.select = SELECT_YES;
		}
		selectDialog_display();
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key_star_up
 * 功能说明 : *键的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 退出房号设置界面，跳转到输出布局
 *********************************************************************************************************/
static void home_id_set_key_star_up(void)
{
	os_layout_goto(&layout_OutPUT);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key_ring_up
 * 功能说明 : 铃声键的按键回调函数（确认键）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 主图层：保存数值+切换下一个焦点；对话框图层：确认/取消房号覆盖
 *********************************************************************************************************/
static void home_id_set_key_ring_up(void)
{
	switch (HomeIdSetClass.layer)
	{
	case LAYER_MAIN:
		if (HomeIdSetClass.set_status != HOME_ID_SET_STATUS_NONE)
			return;

		set_home_id_number();
		if (HomeIdSetClass.layer == LAYER_MAIN)
		{
			clear_prev_home_id_set_focus();
			display_prev_home_id_set_dialog();
			goto_next_home_id_set_focus();
			clear_current_show_home_id();
			home_id_set_input_number_display();
		}
		break;

	case LAYER_DIALOG:
		LOG_WHITE("LAYER_DIALOG\n");

		if (HomeIdSetClass.select == SELECT_YES)
		{
			LOG_WHITE("SELECT_YES\n");
			Intercom.set_id();
			Intercom.status = INT_SET_HOME_ID;
		}
		else
		{
			LOG_WHITE("SELECT_NO\n");
			HomeIdSetClass.set_status = HOME_ID_SET_STATUS_FAIL;
		}

		refreshSelectDialog();
		home_id_set_dialog_display();
		home_id_set_icon_display();
		clear_prev_home_id_set_focus();
		display_prev_home_id_set_dialog();
		goto_next_home_id_set_focus();
		clear_current_show_home_id();
		home_id_set_input_number_display();

		HomeIdSetClass.layer = LAYER_MAIN;
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : home_id_read_key_pound_up
 * 功能说明 : #键的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 触发读取房号操作，并重新显示所有输入框
 *********************************************************************************************************/
static void home_id_read_key_pound_up(void)
{
	Intercom.read_id();
	home_id_set_dialog_display();
}

/*********************************************************************************************************
 * 函 数 名 : set_home_id_init
 * 功能说明 : 初始化房号设置界面的所有参数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 重置图层、选中状态、光标、数值、显示字符串等所有相关变量
 *********************************************************************************************************/
void set_home_id_init(void)
{
	LOG_WHITE("set home id init \n");

	HomeIdSetClass.layer = LAYER_MAIN;
	HomeIdSetClass.select = SELECT_YES;

	HomeIdSetClass.dialog_box[HOME_ID_SET_M1_DIALOG_BOX]->cursor.index = 0;
	HomeIdSetClass.dialog_box[HOME_ID_SET_M2_DIALOG_BOX]->cursor.index = 0;
	HomeIdSetClass.dialog_box[HOME_ID_SET_M3_DIALOG_BOX]->cursor.index = 0;
	HomeIdSetClass.dialog_box[HOME_ID_SET_M4_DIALOG_BOX]->cursor.index = 0;

	HomeIdSetClass.dialog_box[HOME_ID_SET_M1_DIALOG_BOX]->font.text1.font_color = 0xFFFFFFFF;
	HomeIdSetClass.dialog_box[HOME_ID_SET_M2_DIALOG_BOX]->font.text1.font_color = 0xFFFFFFFF;
	HomeIdSetClass.dialog_box[HOME_ID_SET_M3_DIALOG_BOX]->font.text1.font_color = 0xFFFFFFFF;
	HomeIdSetClass.dialog_box[HOME_ID_SET_M4_DIALOG_BOX]->font.text1.font_color = 0xFFFFFFFF;

	HomeIdSetClass.set_id[HOME_ID_SET_M1_DIALOG_BOX] = 0;
	HomeIdSetClass.set_id[HOME_ID_SET_M2_DIALOG_BOX] = 0;
	HomeIdSetClass.set_id[HOME_ID_SET_M3_DIALOG_BOX] = 0;
	HomeIdSetClass.set_id[HOME_ID_SET_M4_DIALOG_BOX] = 0;

	memset(HomeIdSetClass.show_id, 0, sizeof(HomeIdSetClass.show_id));

	memset(HomeIdSetClass.dialog_box[HOME_ID_SET_M1_DIALOG_BOX]->font.string1, 0, 10);
	memset(HomeIdSetClass.dialog_box[HOME_ID_SET_M2_DIALOG_BOX]->font.string1, 0, 10);
	memset(HomeIdSetClass.dialog_box[HOME_ID_SET_M3_DIALOG_BOX]->font.string1, 0, 10);
	memset(HomeIdSetClass.dialog_box[HOME_ID_SET_M4_DIALOG_BOX]->font.string1, 0, 10);

	m_id[0] = -1;
	m_id[1] = -1;
	m_id[2] = -1;
	m_id[3] = -1;

	m_num[0] = -1;
	m_num[1] = -1;
	m_num[2] = -1;
	m_num[3] = -1;
	flagMid = 0;
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_key_register
 * 功能说明 : 注册房号设置界面的所有按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 注册数字键、方向键、*键、铃声键、#键的DOWN状态回调
 *********************************************************************************************************/
static void home_id_set_key_register(void)
{
	key_touch_handle_register(KEY_INDEX_0, KEY_STATE_DOWN, home_id_set_key0_up);
	key_touch_handle_register(KEY_INDEX_1, KEY_STATE_DOWN, home_id_set_key1_up);
	key_touch_handle_register(KEY_INDEX_2, KEY_STATE_DOWN, home_id_set_key2_up);
	key_touch_handle_register(KEY_INDEX_3, KEY_STATE_DOWN, home_id_set_key3_up);
	key_touch_handle_register(KEY_INDEX_4, KEY_STATE_DOWN, home_id_set_key4_up);
	key_touch_handle_register(KEY_INDEX_5, KEY_STATE_DOWN, home_id_set_key5_up);
	key_touch_handle_register(KEY_INDEX_6, KEY_STATE_DOWN, home_id_set_key6_up);
	key_touch_handle_register(KEY_INDEX_7, KEY_STATE_DOWN, home_id_set_key7_up);
	key_touch_handle_register(KEY_INDEX_8, KEY_STATE_DOWN, home_id_set_key8_up);
	key_touch_handle_register(KEY_INDEX_9, KEY_STATE_DOWN, home_id_set_key9_up);

	key_touch_handle_register(KEY_INDEX_UP, KEY_STATE_DOWN, home_id_set_key_up_up);
	key_touch_handle_register(KEY_INDEX_DOWN, KEY_STATE_DOWN, home_id_set_key_down_up);

	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, home_id_set_key_star_up);
	key_touch_handle_register(KEY_INDEX_RING, KEY_STATE_DOWN, home_id_read_key_pound_up);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, home_id_set_key_ring_up);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_m1_dialog_box_init
 * 功能说明 : 初始化M1房号输入框的对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 设置图标、字体、光标参数，初始化对话框控件
 *********************************************************************************************************/
static void home_id_set_m1_dialog_box_init(void)
{
	icon box;
	static char number_string[10];
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 0};

	cursor.max_index = 3;
	resource box_res = resource_get(ROM_R_IMG_ROOM_ID_SET_DIALOG_BOX_PNG);
	icon_init(&box, &HomeIdSetFocusPos[HOME_ID_SET_M1_DIALOG_BOX], &box_res);

	text_init(&font.text1, &HomeIdSetFocusPos[HOME_ID_SET_M1_DIALOG_BOX], 22);
	font.string1 = number_string;

	widget_dialog_box_init(HomeIdSetClass.dialog_box[HOME_ID_SET_M1_DIALOG_BOX],
						   &box,
						   &font,
						   &cursor);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_m2_dialog_box_init
 * 功能说明 : 初始化M2房号输入框的对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 设置图标、字体、光标参数，初始化对话框控件
 *********************************************************************************************************/
static void home_id_set_m2_dialog_box_init(void)
{
	icon box;
	static char number_string[10];
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 0};

	cursor.max_index = 3;
	resource box_res = resource_get(ROM_R_IMG_ROOM_ID_SET_DIALOG_BOX_PNG);
	icon_init(&box, &HomeIdSetFocusPos[HOME_ID_SET_M2_DIALOG_BOX], &box_res);

	text_init(&font.text1, &HomeIdSetFocusPos[HOME_ID_SET_M2_DIALOG_BOX], 22);
	font.string1 = number_string;

	widget_dialog_box_init(HomeIdSetClass.dialog_box[HOME_ID_SET_M2_DIALOG_BOX],
						   &box,
						   &font,
						   &cursor);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_m3_dialog_box_init
 * 功能说明 : 初始化M3房号输入框的对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 设置图标、字体、光标参数，初始化对话框控件
 *********************************************************************************************************/
static void home_id_set_m3_dialog_box_init(void)
{
	icon box;
	static char number_string[10];
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 0};

	cursor.max_index = 3;
	resource box_res = resource_get(ROM_R_IMG_ROOM_ID_SET_DIALOG_BOX_PNG);
	icon_init(&box, &HomeIdSetFocusPos[HOME_ID_SET_M3_DIALOG_BOX], &box_res);

	text_init(&font.text1, &HomeIdSetFocusPos[HOME_ID_SET_M3_DIALOG_BOX], 22);
	font.string1 = number_string;

	widget_dialog_box_init(HomeIdSetClass.dialog_box[HOME_ID_SET_M3_DIALOG_BOX],
						   &box,
						   &font,
						   &cursor);
}

/*********************************************************************************************************
 * 函 数 名 : home_id_set_m4_dialog_box_init
 * 功能说明 : 初始化M4房号输入框的对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 设置图标、字体、光标参数，初始化对话框控件
 *********************************************************************************************************/
static void home_id_set_m4_dialog_box_init(void)
{
	icon box;
	static char number_string[10];
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 0};

	cursor.max_index = 3;
	resource box_res = resource_get(ROM_R_IMG_ROOM_ID_SET_DIALOG_BOX_PNG);
	icon_init(&box, &HomeIdSetFocusPos[HOME_ID_SET_M4_DIALOG_BOX], &box_res);

	text_init(&font.text1, &HomeIdSetFocusPos[HOME_ID_SET_M4_DIALOG_BOX], 22);
	font.string1 = number_string;

	widget_dialog_box_init(HomeIdSetClass.dialog_box[HOME_ID_SET_M4_DIALOG_BOX],
						   &box,
						   &font,
						   &cursor);
}

/*********************************************************************************************************
 * 函 数 名 : selectDialog_init
 * 功能说明 : 初始化房号已存在的确认对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 设置对话框背景、标题、YES/NO文字的位置和样式
 *********************************************************************************************************/
static void selectDialog_init(void)
{
	icon box;
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 0};

	position box_pos = {{0, 151}, {480, 84}};
	resource box_res = resource_get(ROM_R_IMG_CARD_MANEAGE_DIALOG_BOX_PNG);
	icon_init(&box, &box_pos, &box_res);

	position text1_pos = {{0, 155 + 15}, {480, 50}};
	text_init(&font.text1, &text1_pos, 20);
	font.string1 = font_str(STR_HOME_ID_SET_SELECT_DIALOG_BOX_TITLE2);

	position text2_pos = {{180, 185 + 10}, {60, 50}};
	text_init(&font.text2, &text2_pos, 20);
	font.string2 = font_str(STR_CARD_MANAGE_YES);

	position text3_pos = {{240, 185 + 10}, {60, 50}};
	text_init(&font.text3, &text3_pos, 20);
	font.string3 = font_str(STR_CARD_MANAGE_NO);

	widget_dialog_box_init(HomeIdSetClass.selectDialog,
						   &box,
						   &font,
						   &cursor);
}

/*********************************************************************************************************
 * 函 数 名 : layout_home_id_set_init
 * 功能说明 : 初始化房号设置布局的所有输入框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 依次初始化M1-M4输入框的对话框控件
 *********************************************************************************************************/
static void layout_home_id_set_init(void)
{
	home_id_set_m1_dialog_box_init();
	home_id_set_m2_dialog_box_init();
	home_id_set_m3_dialog_box_init();
	home_id_set_m4_dialog_box_init();
}

/*********************************************************************************************************
 * 函 数 名 : layout_home_id_set_enter
 * 功能说明 : 进入房号设置布局的入口函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 显示控件+注册按键+发送读房号指令+初始化显示+初始化确认框
 *********************************************************************************************************/
static void layout_home_id_set_enter(void)
{
	LOG_WHITE("enter layout home id set \n\r");
	HomeIdSetClass.widget_show.icon();
	HomeIdSetClass.widget_show.font();
	HomeIdSetClass.widget_show.dialog_box();
	HomeIdSetClass.widget_show.focus();
	HomeIdSetClass.key_register();

	LOG_WHITE("read id \n");
	if (Intercom.read_id != NULL)
	{
		LOG_WHITE("send CMD_READ_HOME_ID \n ");
		unsigned char brancher_id = OutPUTClass.auth_info.brancher_seq[0] * 100 +
									OutPUTClass.auth_info.brancher_seq[1] * 10 +
									OutPUTClass.auth_info.brancher_seq[2] * 1;
		Intercom.send_cmd(CMD_READ_HOME_ID, HOME_ID1, brancher_id, CMD_NULL, CMD_NULL);
	}

	LOG_WHITE("dialog display \n");
	home_id_set_dialog_display();
	clear_current_show_home_id();
	selectDialog_init();
	display_current_home_id_set_focus();
}

/*********************************************************************************************************
 * 函 数 名 : layout_home_id_set_quit
 * 功能说明 : 退出房号设置布局的清理函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 初始化参数+重置焦点到M1输入框
 *********************************************************************************************************/
static void layout_home_id_set_quit(void)
{
	LOG_WHITE(">>> quit layout home id set \n\r");
	set_home_id_init();
	HomeIdSetClass.cur_focus = HOME_ID_SET_M1_DIALOG_BOX;
}

/*********************************************************************************************************
 * 函 数 名 : home_id_exist
 * 功能说明 : 检查指定房号是否已存在于用户数据中
 * 形    参 : home_id - 要检查的房号
 * 返 回 值 : bool - 存在返回true，不存在返回false
 * 备    注 : 遍历UserData.home_id数组，匹配成功返回true
 *********************************************************************************************************/
static bool home_id_exist(int home_id)
{
	for (unsigned char i = 0; i < get_int_conf(SET_HOME_ID_INDEX); i++)
	{
		if (UserData.home_id[i] == home_id)
		{
			LOG_WHITE("%d exist\n\r", home_id);
			return true;
		}
	}
	return false;
}

/*********************************************************************************************************
 * 函 数 名 : save_set_home_id
 * 功能说明 : 保存设置的房号到用户数据（更新/新增/删除）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 已存在则删除原数据，否则更新/新增，最后保存并更新索引
 *********************************************************************************************************/
static void save_set_home_id(void)
{
	int index = get_int_conf(SET_HOME_ID_INDEX);

	if (!home_id_exist(HomeIdSetClass.set_id[HomeIdSetClass.save_id_falg]))
	{
		if ((index >= 3) && (m_id[0] != m_id[1] && m_id[1] != m_id[2] && m_id[2] != m_id[3]) && m_id[HomeIdSetClass.save_id_falg] < index)
		{
			LOG_WHITE("通过M1 M2 M3 M4 原来的数字对应的下标来保存新数据\n\r");
			UserData.home_id[m_id[HomeIdSetClass.save_id_falg]] = HomeIdSetClass.set_id[HomeIdSetClass.save_id_falg];
			user_data_save();
		}
		else
		{
			LOG_WHITE("新增数据\n\r");
			UserData.home_id[index] = HomeIdSetClass.set_id[HomeIdSetClass.save_id_falg];
			index++;
			user_data_save();
			set_int_conf(SET_HOME_ID_INDEX, index);
		}
	}
	else
	{
		int id = m_id[HomeIdSetClass.save_id_falg];
		int num = m_num[HomeIdSetClass.save_id_falg];
		LOG_WHITE("room number is existed,need delete the original number userdataid[%d]:[%d]\n", id, num);

		for (int i = id; i < get_int_conf(SET_HOME_ID_INDEX); i++)
		{
			if (i == get_int_conf(SET_HOME_ID_INDEX) - 1)
			{
				UserData.home_id[i] = 0;
				break;
			}
			UserData.home_id[i] = UserData.home_id[i + 1];
		}

		index--;
		user_data_save();
		set_int_conf(SET_HOME_ID_INDEX, index);
	}

	for (int i = 0; i < get_int_conf(SET_HOME_ID_INDEX); i++)
	{
		LOG_WHITE("userdata.home id [%d]:%d\n\r", i, UserData.home_id[i]);
	}
}

/*********************************************************************************************************
 * 函 数 名 : layout_home_id_set_timer
 * 功能说明 : 房号设置布局的定时器处理函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 处理设置状态提示（成功/失败/已存在）的显示时长，刷新读房号后的显示
 *********************************************************************************************************/
static void layout_home_id_set_timer(void)
{
	static char show_set_status_count = 0;
	static int redisplay_falg = 0;
	if (HomeIdSetClass.set_status != HOME_ID_SET_STATUS_NONE)
	{
		if (show_set_status_count == 0)
		{
			if (HomeIdSetClass.set_status == HOME_ID_SET_STATUS_SUCCESS)
			{
				save_set_home_id();
				home_id_set_success_font_display();

				HomeIdSetClass.dialog_box[HomeIdSetClass.save_id_falg]->font.text1.font_color = 0xFFFFFFFF;
				widget_dialog_box_display(HomeIdSetClass.dialog_box[HomeIdSetClass.save_id_falg]);
				LOG_WHITE("save set home id success\n");

				flagMid = 0;
			}

			if (HomeIdSetClass.set_status == HOME_ID_SET_STATUS_FAIL)
			{
				home_id_set_fail_font_display();
				HomeIdSetClass.dialog_box[HomeIdSetClass.save_id_falg]->font.text1.font_color = 0xFFFED606;
				widget_dialog_box_display(HomeIdSetClass.dialog_box[HomeIdSetClass.save_id_falg]);
				redisplay_falg = 1;
				LOG_WHITE("save set home id fail\n");
			}

			if (HomeIdSetClass.set_status == HOME_ID_SET_STATUS_EXISTED)
			{
				home_id_already_exists_font_display();
				HomeIdSetClass.dialog_box[HomeIdSetClass.save_id_falg]->font.text1.font_color = 0xFFFED606;
				widget_dialog_box_display(HomeIdSetClass.dialog_box[HomeIdSetClass.save_id_falg]);
				redisplay_falg = 1;
				LOG_WHITE("number existed\n");
			}
		}

		show_set_status_count++;
		if (show_set_status_count >= 30)
		{
			HomeIdSetClass.set_status = HOME_ID_SET_STATUS_NONE;
			show_set_status_count = 0;
			home_id_set_status_font_erase();
			if (redisplay_falg == 1)
			{
				redisplay_falg = 0;
				clear_save_fail_show_home_id();
			}
		}
	}
	if (HomeIdSetClass.read_id_show == READ_HOME_ID_SHOW)
	{
		unsigned int i;
		HomeIdSetClass.read_id_show = READ_HOME_ID_HIDE;
		for (i = 0; i < 4; i++)
		{
			widget_dialog_box_display(HomeIdSetClass.dialog_box[i]);
		}
		display_current_home_id_set_focus();
	}
}

// M1-M4房号输入框对话框实例
STR_WidgetDialogBox HomeIdSetM1DialogBox;
STR_WidgetDialogBox HomeIdSetM2DialogBox;
STR_WidgetDialogBox HomeIdSetM3DialogBox;
STR_WidgetDialogBox HomeIdSetM4DialogBox;

// 房号输入框对话框指针数组
STR_WidgetDialogBox *HomeIdSetDialogBox[HOME_ID_SET_TOTAL_DIALOG_BOX] = {
	&HomeIdSetM1DialogBox,
	&HomeIdSetM2DialogBox,
	&HomeIdSetM3DialogBox,
	&HomeIdSetM4DialogBox,
};

// 确认对话框实例
STR_WidgetDialogBox selectDialogBox;

// 房号设置类实例（核心状态管理）
STR_HomeIdSetClass HomeIdSetClass = {
	{home_id_set_icon_display, home_id_set_font_display, home_id_set_focus_display, home_id_set_dialog_display, NULL},
	HomeIdSetDialogBox,
	home_id_set_key_register,
	HOME_ID_SET_M1_DIALOG_BOX,
	HOME_ID_SET_STATUS_NONE,
	READ_HOME_ID_HIDE,
	{{0}, {0}},
	{0},
	0,
	LAYER_MAIN,
	&selectDialogBox,
	SELECT_YES,
};

// 房号设置布局实例（UI生命周期管理）
layout layout_home_id_set = {
	.init = layout_home_id_set_init,
	.enter = layout_home_id_set_enter,
	.quit = layout_home_id_set_quit,
	.timer = layout_home_id_set_timer};
