#include "os_sys_api.h"
#include "layout_base.h"
#include "ui_api.h"
#include "rom.h"
#include "string.h"

#define LONG_PRESS_TIME (3000) // 长按触发时间阈值（毫秒）

/*********************************************************************************************************
 * 函 数 名 : dial_back_ground_display
 * 功能说明 : 显示拨号界面的背景图片
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 背景图片位置为全屏(0,0)，尺寸480x206，资源为拨号界面背景PNG
 *********************************************************************************************************/
static void dial_back_ground_display(void)
{
	icon Background;
	position pos = {{0, 0}, {480, 206}};

	resource res = resource_get(ROM_R_IMG_CALL_PAGE_GROUND_PNG);
	icon_init(&Background, &pos, &res);

	icon_display(&Background);
}

/*********************************************************************************************************
 * 函 数 名 : dial_ring_icon_display
 * 功能说明 : 显示拨号界面的通话/井号键图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 适配英语/波斯语布局，英语位置(135,47)，波斯语位置(80,48)，尺寸均为42x42
 *********************************************************************************************************/
static void dial_ring_icon_display(void)
{
	icon ring;
	resource res = resource_get(ROM_R_IMG_CALL_PAGE_RING_ICON_PNG);
	position pos;
	if (language_get() == language_english)
	{
		pos.point.x = 135;
		pos.point.y = 47;
		pos.vector.height = 42;
		pos.vector.width = 42;
	}
	else if (language_get() == language_persian)
	{
		pos.point.x = 300;
		pos.point.y = 48;
		pos.vector.height = 42;
		pos.vector.width = 42;
	}

	icon_init(&ring, &pos, &res);
	icon_display(&ring);
}

/*********************************************************************************************************
 * 函 数 名 : dial_guard_icon_display
 * 功能说明 : 显示拨号界面的呼叫警卫图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 图标位置(135,100)，尺寸42x42，资源为警卫图标PNG
 *********************************************************************************************************/
static void dial_guard_icon_display(void)
{
	icon guard;
	position pos = {{135, 100}, {42, 42}};
	if (language_get() == language_english)
	{
		pos.point.x = 135;
		pos.point.y = 100;
	}
	else if (language_get() == language_persian)
	{
		pos.point.x = 300;
		pos.point.y = 100;
	}
	resource res = resource_get(ROM_R_IMG_CALL_PAGE_GUARD_ICON_PNG);
	icon_init(&guard, &pos, &res);

	icon_display(&guard);
}

/*********************************************************************************************************
 * 函 数 名 : dial_star_icon_display
 * 功能说明 : 显示拨号界面的星号键图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 图标位置(135,153)，尺寸42x42，资源为星号图标PNG
 *********************************************************************************************************/
static void dial_star_icon_display(void)
{
	icon star;
	position pos = {{135, 153}, {42, 42}};
	if (language_get() == language_english)
	{
		pos.point.x = 135;
		pos.point.y = 153;
	}
	else if (language_get() == language_persian)
	{
		pos.point.x = 300;
		pos.point.y = 153;
	}
	resource res = resource_get(ROM_R_IMG_CALL_PAGE_STAR_ICON_PNG);
	icon_init(&star, &pos, &res);

	icon_display(&star);
}

/*********************************************************************************************************
 * 函 数 名 : dial_icon_display
 * 功能说明 : 拨号界面所有图标显示的入口函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 依次显示背景、通话图标、警卫图标、星号图标
 *********************************************************************************************************/
static void dial_icon_display(void)
{
	dial_back_ground_display();
	dial_ring_icon_display();
	dial_guard_icon_display();
	dial_star_icon_display();
}

/*********************************************************************************************************
 * 函 数 名 : press_font_display
 * 功能说明 : 显示拨号界面的"按下"提示文字
 * 形    参 : x：文字X坐标；y：文字Y坐标
 * 返 回 值 : 无
 * 备    注 : 文字尺寸90x31，字体大小26，左中对齐
 *********************************************************************************************************/
static void press_font_display(int x, int y)
{
	text press;
	position pos = {{x, y}, {90, 31}};

	text_init(&press, &pos, 26);
	press.align = LEFT_MIDDLE;
	text_display(&press, font_str(STR_DIAL_PRESS));
}

/*********************************************************************************************************
 * 函 数 名 : underline_font_display
 * 功能说明 : 绘制拨号界面的下划线分隔线
 * 形    参 : x：下划线X坐标；y：下划线Y坐标
 * 返 回 值 : 无
 * 备    注 : 下划线尺寸388x1，颜色为白色(0xFFFFFFFF)
 *********************************************************************************************************/
static void underline_font_display(int x, int y)
{
	position pos = {{x, y}, {388, 1}};
	draw_rect(&pos, 0xFFFFFFFF);
}

/*********************************************************************************************************
 * 函 数 名 : input_unit_font_display
 * 功能说明 : 显示拨号界面的"输入单元号"提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(42,14)，尺寸336x31，字体大小26，左中对齐
 *********************************************************************************************************/
static void input_unit_font_display(void)
{
	text input_unit;
	position pos = {{42, 14}, {336, 31}};
	if (language_get() == language_english)
	{
		pos.point.x = 42;
		pos.point.y = 14;
	}
	else if (language_get() == language_persian)
	{
		pos.point.x = 230;
		pos.point.y = 14;
	}
	text_init(&input_unit, &pos, 26);
	input_unit.align = LEFT_MIDDLE;
	text_display(&input_unit, font_str(STR_DIAL_INPUT_UNIT));
}

/*********************************************************************************************************
 * 函 数 名 : call_guard_font_display
 * 功能说明 : 显示拨号界面的"呼叫警卫"提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(180,106)，尺寸210x31，字体大小26，右中对齐
 *********************************************************************************************************/
static void call_guard_font_display(void)
{
	position pos = {{180, 106}, {210, 31}};
	text call_guard;
	if (language_get() == language_english)
	{
		pos.point.x = 180;
		pos.point.y = 106;
	}
	else if (language_get() == language_persian)
	{
		pos.point.x = 80;
		pos.point.y = 106;
	}
	text_init(&call_guard, &pos, 26);
	call_guard.align = RIGHT_MIDDLE;
	text_display(&call_guard, font_str(STR_DIAL_CALL_GUARD));
}

/*********************************************************************************************************
 * 函 数 名 : dial_star_font_display
 * 功能说明 : 显示拨号界面的"星号清除"提示文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(180,162)，尺寸133x31，字体大小26，右中对齐
 *********************************************************************************************************/
static void dial_star_font_display(void)
{
	position pos = {{180, 162}, {133, 31}};
	text star_corre;
	if (language_get() == language_english)
	{
		pos.point.x = 180;
		pos.point.y = 162;
	}
	else if (language_get() == language_persian)
	{
		pos.point.x = 150;
		pos.point.y = 162;
	}
	text_init(&star_corre, &pos, 26);
	star_corre.align = RIGHT_MIDDLE;
	text_display(&star_corre, font_str(STR_DIAL_STAR_CLEAR));
}

/*********************************************************************************************************
 * 函 数 名 : unit_font_display
 * 功能说明 : 显示拨号界面的"单元号"标题文字
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(43,224)，尺寸95x39，字体大小32，右中对齐，字体颜色为0xFFF5C612
 *********************************************************************************************************/
static void unit_font_display(void)
{
	position pos;
	text star_corre;
	if (language_get() == language_english)
	{
		pos.point.x = 43;
		pos.point.y = 224;
		pos.vector.height = 39;
		pos.vector.width = 95;
	}
	else if (language_get() == language_persian)
	{
		pos.point.x = 300;
		pos.point.y = 224;
		pos.vector.height = 39;
		pos.vector.width = 95;
	}
	text_init(&star_corre, &pos, 32);
	star_corre.align = RIGHT_MIDDLE;
	star_corre.font_color = 0xFFF5C612;
	text_display(&star_corre, font_str(STR_DIAL_UNIT));
}

/*********************************************************************************************************
 * 函 数 名 : dial_font_display
 * 功能说明 : 拨号界面所有文字提示显示的入口函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按布局顺序显示各类提示文字和分隔线
 *********************************************************************************************************/
static void dial_font_display(void)
{
	unsigned char language = language_get();

	input_unit_font_display();

	/* 显示"按下"提示文字 */
	if (language == language_english)
	{
		press_font_display(42, 50);
		press_font_display(42, 106);
		press_font_display(42, 162);
	}
	else if (language == language_persian)
	{
		press_font_display(347, 50);
		press_font_display(347, 106);
		press_font_display(347, 162);
	}

	/* 显示分隔线 */
	underline_font_display(42, 94);
	underline_font_display(42, 149);

	/* 显示功能提示文字 */
	call_guard_font_display();
	dial_star_font_display();
	unit_font_display();
}

/*********************************************************************************************************
 * 函 数 名 : dial_home_id_init
 * 功能说明 : 初始化单元号输入数组
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 将4位单元号数组全部置0
 *********************************************************************************************************/
void dial_home_id_init(void)
{
	unsigned char i;
	for (i = 0; i < 4; i++)
	{
		DialClass.home_id[i] = 0;
	}
}

/*********************************************************************************************************
 * 函 数 名 : dial_dialog_box_display
 * 功能说明 : 显示拨号界面的数字输入对话框
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 显示预设的数字输入对话框控件
 *********************************************************************************************************/
static void dial_dialog_box_display(void)
{
	widget_dialog_box_display(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]);
}

/*********************************************************************************************************
 * 函 数 名 : dial_number_dialog_box_init
 * 功能说明 : 初始化数字输入对话框控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 包含对话框图标、字体、光标初始化，最大输入长度为3位（光标max_index=3）
 *********************************************************************************************************/
static void dial_number_dialog_box_init(void)
{
	icon box;
	static char number_string[10];
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 3};

	position box_pos = {{156, 220}, {174, 34}};
	resource box_res = resource_get(ROM_R_IMG_CALL_PAGE_NUMBER_DIALOG_BOX_PNG);
	icon_init(&box, &box_pos, &box_res);

	position text1_pos = {{168, 222}, {150, 28}};
	text_init(&font.text1, &text1_pos, 25);
	font.text1.align = CENTER_MIDDLE;
	font.string1 = number_string;

	widget_dialog_box_init(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX],
						   &box,
						   &font,
						   &cursor);
}

/*********************************************************************************************************
 * 函 数 名 : dial_number_dialog_box_font_change
 * 功能说明 : 根据输入索引更新数字输入对话框的显示内容
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按输入位数拼接数字字符串，位数从0到4位，数字间用空格分隔
 *********************************************************************************************************/
static void dial_number_dialog_box_font_change(void)
{
	switch (DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.index)
	{
	case 0:
		memset(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->font.string1, 0, 10);
		break;
	case 1:
		sprintf(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->font.string1,
				"%d",
				DialClass.home_id[0]);
		break;
	case 2:
		sprintf(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->font.string1,
				"%d %d",
				DialClass.home_id[0],
				DialClass.home_id[1]);
		break;
	case 3:
		sprintf(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->font.string1,
				"%d %d %d", DialClass.home_id[0],
				DialClass.home_id[1],
				DialClass.home_id[2]);
		break;
	case 4:
		sprintf(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->font.string1,
				"%d %d %d %d",
				DialClass.home_id[0],
				DialClass.home_id[1],
				DialClass.home_id[2],
				DialClass.home_id[3]);
		break;
	}
}

/*********************************************************************************************************
 * 函 数 名 : dial_input_number_display
 * 功能说明 : 刷新数字输入对话框的显示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 重新显示数字输入对话框，用于输入后更新界面
 *********************************************************************************************************/
static void dial_input_number_display(void)
{
	widget_dialog_box_display(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]);
}

/*********************************************************************************************************
 * 函 数 名 : dial_input_sub_number
 * 功能说明 : 数字输入框的删除功能（删除最后一位）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 索引大于0时才执行删除，删除后更新对话框显示内容
 *********************************************************************************************************/
void dial_input_sub_number(void)
{
	if (DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.index > 0)
	{
		DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.index--;
		dial_number_dialog_box_font_change();
	}
}

/*********************************************************************************************************
 * 函 数 名 : dial_input_add_number
 * 功能说明 : 向数字输入框添加单个数字
 * 形    参 : number：要添加的数字（0-9）
 * 返 回 值 : 无
 * 备    注 : 数字存入单元号数组，索引不超过最大值时递增索引并更新显示
 *********************************************************************************************************/
void dial_input_add_number(unsigned char number)
{
	unsigned char index = DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.index;
	DialClass.home_id[index] = number;

	if (index <= DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.max_index)
	{ //   < 三位数     <=  四位数
		DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.index++;
		dial_number_dialog_box_font_change();
	}
	dial_input_number_display();
}

/*********************************************************************************************************
 * 函 数 名 : dial_key0_up
 * 功能说明 : 数字键0的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下0键时向输入框添加数字0
 *********************************************************************************************************/
static void dial_key0_up(void)
{
	dial_input_add_number(0);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key1_up
 * 功能说明 : 数字键1的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下1键时向输入框添加数字1
 *********************************************************************************************************/
static void dial_key1_up(void)
{
	dial_input_add_number(1);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key2_up
 * 功能说明 : 数字键2的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下2键时向输入框添加数字2
 *********************************************************************************************************/
static void dial_key2_up(void)
{
	dial_input_add_number(2);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key3_up
 * 功能说明 : 数字键3的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下3键时向输入框添加数字3
 *********************************************************************************************************/
static void dial_key3_up(void)
{
	dial_input_add_number(3);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key4_up
 * 功能说明 : 数字键4的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下4键时向输入框添加数字4
 *********************************************************************************************************/
static void dial_key4_up(void)
{
	dial_input_add_number(4);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key5_up
 * 功能说明 : 数字键5的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下5键时向输入框添加数字5
 *********************************************************************************************************/
static void dial_key5_up(void)
{
	dial_input_add_number(5);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key6_up
 * 功能说明 : 数字键6的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下6键时向输入框添加数字6
 *********************************************************************************************************/
static void dial_key6_up(void)
{
	dial_input_add_number(6);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key7_up
 * 功能说明 : 数字键7的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下7键时向输入框添加数字7
 *********************************************************************************************************/
static void dial_key7_up(void)
{
	dial_input_add_number(7);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key8_up
 * 功能说明 : 数字键8的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下8键时向输入框添加数字8
 *********************************************************************************************************/
static void dial_key8_up(void)
{
	dial_input_add_number(8);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key9_up
 * 功能说明 : 数字键9的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 按下9键时向输入框添加数字9
 *********************************************************************************************************/
static void dial_key9_up(void)
{
	dial_input_add_number(9);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key_star_up
 * 功能说明 : 星号键(*)抬起时的回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 执行删除最后一位数字操作，并刷新输入框显示
 *********************************************************************************************************/
static void dial_key_star_up(void)
{
	dial_input_sub_number();
	dial_input_number_display();
}

static struct ak_timeval dial_key_press_start_time; // 按键按下起始时间

/*********************************************************************************************************
 * 函 数 名 : dial_key_star_down
 * 功能说明 : 星号键(*)按下时的回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 记录按键按下的起始时间，用于长按判断
 *********************************************************************************************************/
static void dial_key_star_down(void)
{
	ak_get_ostime(&dial_key_press_start_time);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key_star_long
 * 功能说明 : 星号键(*)长按回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 长按超过3秒时，设置解锁触发模式并跳转到密码输入布局
 *********************************************************************************************************/
static void dial_key_star_long(void)
{
	struct ak_timeval cur_cpu_time;
	ak_get_ostime(&cur_cpu_time);
	if (ak_diff_ms_time(&cur_cpu_time, &dial_key_press_start_time) >= LONG_PRESS_TIME)
	{ /* befor 3000 */
		key_long_press_goto_layout = DIAL_STAR_LONG_PRESS;
		PasswordClass.trigger_mode = INPUT_UNLOCK;
		os_layout_goto(&layout_password);
	}
}

/*********************************************************************************************************
 * 函 数 名 : dial_key_pound_down
 * 功能说明 : 井号键(#)按下时的回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 记录按键按下的起始时间，用于长按判断
 *********************************************************************************************************/
static void dial_key_pound_down(void)
{
	ak_get_ostime(&dial_key_press_start_time);
}

/*********************************************************************************************************
 * 函 数 名 : dial_key_pound_long
 * 功能说明 : 井号键(#)长按回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 长按超过3秒时，设置设置触发模式并跳转到密码输入布局
 *********************************************************************************************************/
static void dial_key_pound_long(void)
{
	struct ak_timeval cur_cpu_time;
	ak_get_ostime(&cur_cpu_time);
	if (ak_diff_ms_time(&cur_cpu_time, &dial_key_press_start_time) >= LONG_PRESS_TIME)
	{
		PasswordClass.trigger_mode = INPUT_SETTING;
		os_layout_goto(&layout_password);
	}
}

/*********************************************************************************************************
 * 函 数 名 : dial_key_guard_up
 * 功能说明 : 警卫键的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 设置呼叫对象为警卫，跳转到通话布局
 *********************************************************************************************************/
static void dial_key_guard_up(void)
{
	CallingClass.call_object = CALL_GUARD;
	os_layout_goto(&layout_calling);
}

/*********************************************************************************************************
 * 函 数 名 : dial_home_id_adjust
 * 功能说明 : 调整单元号数组的存储顺序
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 将输入的数字左移，首位补0，适配呼叫逻辑的数字解析规则
 *********************************************************************************************************/
static void dial_home_id_adjust(void)
{
	unsigned char i, j;
	unsigned char index = DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.index;
	unsigned char max_index = DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.max_index;

	for (i = index; i <= max_index; i++)
	{
		for (j = max_index; j > 0; j--)
		{
			DialClass.home_id[j] = DialClass.home_id[j - 1];
		}
		DialClass.home_id[0] = 0;
	}
}

/*********************************************************************************************************
 * 函 数 名 : dial_key_ring_up
 * 功能说明 : 通话键(ring)的按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 无输入时呼叫警卫，有输入时呼叫用户；输入为空时直接返回
 *********************************************************************************************************/
static void dial_key_ring_up(void)
{
	unsigned long home_id_add;
	unsigned char index = DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.index;

	if (index == 0)
	{
		return;
	}
	dial_home_id_adjust();
	home_id_add = DialClass.home_id[0] +
				  DialClass.home_id[1] +
				  DialClass.home_id[2] +
				  DialClass.home_id[3];

	if (home_id_add == 0)
	{
		CallingClass.call_object = CALL_GUARD;
		os_layout_goto(&layout_calling);
	}
	else
	{
		CallingClass.call_object = CALL_USER;
		os_layout_goto(&layout_calling);
	}
}

/*********************************************************************************************************
 * 函 数 名 : dial_key_register
 * 功能说明 : 注册拨号界面的所有按键回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 包含数字键、星号键（短按/长按）、井号键（长按）、警卫键、通话键的事件注册
 *********************************************************************************************************/
static void dial_key_register(void)
{
	key_touch_handle_register(KEY_INDEX_0, KEY_STATE_DOWN, dial_key0_up);
	key_touch_handle_register(KEY_INDEX_1, KEY_STATE_DOWN, dial_key1_up);
	key_touch_handle_register(KEY_INDEX_2, KEY_STATE_DOWN, dial_key2_up);
	key_touch_handle_register(KEY_INDEX_3, KEY_STATE_DOWN, dial_key3_up);
	key_touch_handle_register(KEY_INDEX_4, KEY_STATE_DOWN, dial_key4_up);
	key_touch_handle_register(KEY_INDEX_5, KEY_STATE_DOWN, dial_key5_up);
	key_touch_handle_register(KEY_INDEX_6, KEY_STATE_DOWN, dial_key6_up);
	key_touch_handle_register(KEY_INDEX_7, KEY_STATE_DOWN, dial_key7_up);
	key_touch_handle_register(KEY_INDEX_8, KEY_STATE_DOWN, dial_key8_up);
	key_touch_handle_register(KEY_INDEX_9, KEY_STATE_DOWN, dial_key9_up);

	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_UP, dial_key_star_up);			// [x]
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, dial_key_star_down);		// [x]
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_LONG_DOWN, dial_key_star_long); // [x]

	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, dial_key_pound_down);	  // [OK]
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_LONG_DOWN, dial_key_pound_long); // [OK]

	key_touch_handle_register(KEY_INDEX_GUARD, KEY_STATE_DOWN, dial_key_guard_up);
	key_touch_handle_register(KEY_INDEX_RING, KEY_STATE_DOWN, dial_key_ring_up); // [ring]
}

/*********************************************************************************************************
 * 函 数 名 : layout_dial_init
 * 功能说明 : 拨号布局的初始化函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 初始化数字输入对话框，重置刷卡模式为空闲状态
 *********************************************************************************************************/
static void layout_dial_init(void)
{
	dial_number_dialog_box_init();
	SwipingCard.mode = CARD_IDLE_MODE;
}

/*********************************************************************************************************
 * 函 数 名 : layout_dial_enter
 * 功能说明 : 进入拨号布局时的初始化操作
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 显示控件、注册按键、启动60秒待机定时器，打印语言信息日志
 *********************************************************************************************************/
static void layout_dial_enter(void)
{
	LOG_WHITE(">>> enter layout dial \n\r");
	LOG_BLUE("language is %d \n", language_get());

	DialClass.widget_show.icon();
	DialClass.widget_show.dialog_box();
	DialClass.widget_show.font();
	DialClass.key_register();

	standby_timer_open(60000, goto_layout_standby);
}

/*********************************************************************************************************
 * 函 数 名 : layout_dial_quit
 * 功能说明 : 退出拨号布局时的清理操作
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 重置输入框索引为0，清空输入字符串，打印退出日志
 *********************************************************************************************************/
static void layout_dial_quit(void)
{
	LOG_WHITE(">>> quit layout dial \n\r");
	// key_num_flag = false;
	DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->cursor.index = 0;
	memset(DialClass.dialog_box[DIAL_NUMBER_DIALOG_BOX]->font.string1, 0, 10);
}

/*********************************************************************************************************
 * 函 数 名 : layout_dial_timer
 * 功能说明 : 拨号布局的定时器回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 连接状态下等待3秒后启用触摸音，通过计数实现3秒延时（120次定时器周期）
 *********************************************************************************************************/
static void layout_dial_timer(void)
{
	static int wait3s_count = 0;

	if (whichAway == CONNECTED)
	{
		// LOG_WHITE("call from out door ,so indoor must be playing sound, now wait 3 seconds\n");
		wait3s_count++;
		if (wait3s_count >= 120)
		{
			LOG_WHITE("after 3 seconds ,we need enable touch tone, set [whichAway] = FROM_NONE to enable it\n");
			whichAway = FROM_NONE;
			wait3s_count = 0;
		}
	}
}

STR_WidgetDialogBox DialNumberDialogBox;

STR_WidgetDialogBox *DialDialogBox[DIAL_TOTAL_DIALOG_BOX] = {
	&DialNumberDialogBox};

STR_DialClass DialClass = {
	{dial_icon_display, dial_font_display, NULL, dial_dialog_box_display, NULL},
	DialDialogBox,
	dial_key_register,
	{0}};

layout layout_dial = {
	.init = layout_dial_init,
	.enter = layout_dial_enter,
	.quit = layout_dial_quit,
	.timer = layout_dial_timer};