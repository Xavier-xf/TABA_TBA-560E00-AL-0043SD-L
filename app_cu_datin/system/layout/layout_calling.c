#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "string.h"

/*********************************************************************************************************
 * 函 数 名 : calling_icon_display
 * 功能说明 : 显示通话界面的图标控件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 主要显示taba按钮控件，无其他图标元素
 *********************************************************************************************************/
static void calling_icon_display(void)
{
	taba_btn_display();
}

/*************************************************************************
 * @brief  字符串逆序
 * @date   2022-11-02 15:38
 **************************************************************************/
/*********************************************************************************************************
 * 函 数 名 : reverse
 * 功能说明 : 将字符串进行逆序处理
 * 形    参 : left：字符串起始指针；right：字符串结束指针
 * 返 回 值 : 无
 * 备    注 : 用于波斯语数字显示的逆序处理，通过交换首尾字符实现逆序
 *********************************************************************************************************/
static void reverse(char *left, char *right)
{
	while (left < right)
	{
		char tmp = 0;
		tmp = *left;
		*left = *right;
		*right = tmp;
		left++;
		right--;
	}
}

static void calling_text_area_clear(void)
{
	position pos = {{90, 82}, {340, 60}};
	gui_erase(&pos, 0x00000000);
}

/*********************************************************************************************************
 * 函 数 名 : calling_ring_font_display
 * 功能说明 : 显示呼叫用户的文字提示信息，适配英语和波斯语布局
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 英语直接显示单元号，波斯语需将数字字符串逆序后显示，附带省略号排版适配
 *********************************************************************************************************/
static void calling_ring_font_display(void)
{
	unsigned int number = DialClass.home_id[0] * 1000 + DialClass.home_id[1] * 100 + DialClass.home_id[2] * 10 + DialClass.home_id[3];

	if (language_get() == language_english)
	{ /* 英语 */
		position pos = {{110, 86}, {310, 50}};
		text input_unit;
		char string[20];

		calling_text_area_clear();
		sprintf(string, "%s NO.%d ...", (char *)font_str(STR_CALLING_USER), number);
		text_init(&input_unit, &pos, 36);
		input_unit.align = LEFT_TOP;
		text_display(&input_unit, string);
	}
	else
	{ /* 波斯语 */
		char arr[5] = {0};
		sprintf(arr, "%d", number);
		reverse(arr, arr + strlen(arr) - 1);

		position pos = {{110, 86}, {310, 50}};
		text input_unit;
		char string[30];
		calling_text_area_clear();
		text_init(&input_unit, &pos, 36);
		sprintf(string, "%s %s", (char *)font_str(STR_CALLING_USER), arr);

		text_display(&input_unit, string);
	}
}

/*********************************************************************************************************
 * 函 数 名 : calling_guard_font_display
 * 功能说明 : 显示呼叫警卫的文字提示信息
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 文字位置(124,86)，尺寸310x50，字体大小36，左上对齐
 *********************************************************************************************************/
static void calling_guard_font_display(void)
{
	position pos = {{120, 86}, {310, 50}};
	text input_unit;

	calling_text_area_clear();
	text_init(&input_unit, &pos, 36);
	input_unit.align = LEFT_TOP;
	text_display(&input_unit, font_str(STR_CALLING_GUARD));
}

/*********************************************************************************************************
 * 函 数 名 : no_answer_font_display
 * 功能说明 : 显示呼叫无应答的文字提示信息
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 先擦除原有显示区域（黑色背景），再居中显示无应答提示文字
 *********************************************************************************************************/
static void no_answer_font_display(void)
{
	position pos = {{95, 86}, {310, 50}};
	text no_answer;

	calling_text_area_clear();
	text_init(&no_answer, &pos, 36);
	no_answer.align = CENTER_TOP;
	text_display(&no_answer, font_str(STR_CALLING_NO_ANSWER));
}

/*********************************************************************************************************
 * 函 数 名 : calling_font_display
 * 功能说明 : 根据呼叫对象显示对应的文字提示
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 呼叫用户显示单元号提示，呼叫警卫显示警卫提示，通过CallingClass.call_object判断
 *********************************************************************************************************/
static void calling_font_display(void)
{
	if (CallingClass.call_object == CALL_USER)
		calling_ring_font_display();
	else
		calling_guard_font_display();
}

/*********************************************************************************************************
 * 函 数 名 : layout_calling_init
 * 功能说明 : 通话布局的初始化函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 预留初始化接口，当前无具体初始化逻辑
 *********************************************************************************************************/
static void layout_calling_init(void)
{
}

/******************************* <修改内容> *******************************
 * 1.  @date   2022-08-18 1623
 *     @author xiaoele
 *     @brief  no_answer的显示时长，进入页面时将其置零
 ***************************************************************************/
static unsigned int show_no_answer_count = 0; // 无应答提示显示计数

/******************************* <修改内容> *******************************
 * 1.  @date   2022-08-18 1624
 *     @author xiaoele
 *     @brief  呼叫的时长，进入页面时置零
 ***************************************************************************/
static unsigned int calling_count = 0; // 呼叫等待应答计数

/*************************************************************************
 * @brief  计时状态标志， 0-> 呼叫等待 1-> 无应答计时
 * @date   2022-08-18 20:46
 * @author xiaoele
 **************************************************************************/
static unsigned short int count_flag = 0; // 计时状态标志位

/*********************************************************************************************************
 * 函 数 名 : layout_calling_enter
 * 功能说明 : 进入通话布局时的初始化操作
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 重置计时标志和计数变量，初始化应答状态，显示控件并发起对讲呼叫
 *********************************************************************************************************/
static void layout_calling_enter(void)
{
	LOG_WHITE(">>> enter layout calling \n\r");

	/* 数据初始化 */
	count_flag = 0;
	calling_count = 0;
	show_no_answer_count = 0;
	CallingClass.answer_status = NORMAL;

	CallingClass.widget_show.icon();
	CallingClass.widget_show.font();

	Intercom.call();
}

/*********************************************************************************************************
 * 函 数 名 : layout_calling_quit
 * 功能说明 : 退出通话布局时的清理操作
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 重置单元号数组，清空呼叫计数和无应答显示计数，打印退出日志
 *********************************************************************************************************/
static void layout_calling_quit(void)
{
	LOG_WHITE(">>> quit layout calling \n\r");

	dial_home_id_init();

	calling_count = 0;
	show_no_answer_count = 0;
}

/*********************************************************************************************************
 * 函 数 名 : calling_wait_ack
 * 功能说明 : 处理呼叫等待应答的计时逻辑
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 计数超过80时，根据对讲状态处理：非等待状态返回拨号布局，等待状态标记为无应答
 *********************************************************************************************************/
static void calling_wait_ack(void)
{
	calling_count++;
	if (calling_count > 80)
	{
		LOG_WHITE("Intercom status:%d   answer_status:%d\n\r", Intercom.status, CallingClass.answer_status);

		if (Intercom.status != INT_WAIT_ACK)
		{
			LOG_WHITE("goto dial\n");
			os_layout_goto(&layout_dial);
		}
		else
		{
			CallingClass.answer_status = NO_ANSWER; // 为了显示no anwser
			Intercom.status = INT_IDLE;
			count_flag = 1;
		}
	}
}

/*********************************************************************************************************
 * 函 数 名 : calling_no_answer_font_display
 * 功能说明 : 处理无应答提示的显示和计时逻辑
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 首次触发显示无应答文字，计数超过80时重置应答状态并返回拨号布局
 *********************************************************************************************************/
static void calling_no_answer_font_display(void)
{
	if (CallingClass.answer_status == NO_ANSWER)
	{
		if (show_no_answer_count == 0)
		{
			LOG_WHITE("no answer\n");
			no_answer_font_display();
		}

		/* 计时并返回 */
		show_no_answer_count++;

		if (80 < show_no_answer_count)
		{
			CallingClass.answer_status = ANSWER;
			LOG_WHITE("goto dial\n");
			os_layout_goto(&layout_dial);
		}
	}
}

/*********************************************************************************************************
 * 函 数 名 : layout_calling_timer
 * 功能说明 : 通话布局的定时器回调函数
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 根据计数标志位分别处理呼叫等待应答和无应答逻辑，每次执行休眠10ms
 *********************************************************************************************************/
static void layout_calling_timer(void)
{
	if (count_flag == 0)
	{
		calling_wait_ack();
	}
	else
	{
		calling_no_answer_font_display();
	}

	usleep(10 * 1000);
}

STR_CallingClass CallingClass = {
	{calling_icon_display, calling_font_display, NULL, NULL, NULL},
	CALL_USER,
	ANSWER};

layout layout_calling = {
	.init = layout_calling_init,
	.enter = layout_calling_enter,
	.quit = layout_calling_quit,
	.timer = layout_calling_timer};
