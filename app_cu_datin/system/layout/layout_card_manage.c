#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "user_data.h"
#include "string.h"
#include "ak_thread.h"

#define CARD_MANAGE_PROMPT_TIMEOUT_TICKS 67
#define CARD_PROMPT_ERROR_COLOR 0xFFFF0000
#define CARD_PROMPT_SUCCESS_COLOR 0xFFFFFFFF

static ak_pthread_t thread_delete_card;
static ak_pthread_t thread_save_card;

static void *delete_current_card(void *arg);
static void room_card_numbe_display(void);
static void room_card_string_buf_display(void);

// ========== 焦点-坐标映射表 ==========
const FocusPosMap card_manage_error_pos_map[] = {
	{UNIT_FOCUS, 160, 28},
	{TAG_FOCUS, 160, 71},
	{ERASE_FOCUS, 160, 114},
	{SAVE_FOCUS, 160, 157}};

const int card_manage_error_pos_map_count =
	sizeof(card_manage_error_pos_map) / sizeof(card_manage_error_pos_map[0]);

// ========== 通用工具函数：查找焦点对应的坐标 ==========
// focus_type：要查找的焦点类型 | x/y：输出找到的坐标 | 返回值：1=找到，0=未找到
static int get_focus_pos(CARD_MANAGE_MAIN_FOUCUS focus_type, int *x, int *y)
{
	if (x == NULL || y == NULL)
		return 0; // 防空指针，提升鲁棒性

	for (int i = 0; i < card_manage_error_pos_map_count; i++)
	{
		if (card_manage_error_pos_map[i].focus_type == focus_type)
		{
			*x = card_manage_error_pos_map[i].x;
			*y = card_manage_error_pos_map[i].y;
			return 1;
		}
	}
	return 0; // 未找到对应焦点的坐标
}

// ========== 错误文本显示逻辑 ==========
static void display_error_text(int x, int y, int width, int height, int font_size)
{
	position pos = {{x, y}, {width, height}};
	text error;
	text_init(&error, &pos, font_size);
	const char *error_str = font_str(STR_CARD_NUMBER_UNIT_NUMBER_ERROR);
	text_display(&error, error_str);
}

// ========== YES文本显示逻辑 ==========
static void display_yes_text(int x, int y, int width, int height, int font_size)
{
	position pos = {{x, y}, {width, height}};
	text yes;
	text_init(&yes, &pos, font_size);
	const char *yes_str = font_str(STR_CARD_NUMBER_SUCCESS);
	text_display(&yes, yes_str);
}
// ==========错误文本显示函数） ==========
void room_unit_number_error_font_display(void)
{
	const int width = 200;
	const int height = 40;
	const int font_size = 20;
	int target_x = -1, target_y = -1;

	// 查找当前焦点对应的坐标，找到则显示错误文本
	if (get_focus_pos(CardManageClass.cur_focus.main, &target_x, &target_y))
	{
		display_error_text(target_x, target_y, width, height, font_size);
	}
}

// ==========yes文本显示函数） ==========
void room_unit_number_yes_font_display(void)
{
	const int width = 200;
	const int height = 40;
	const int font_size = 20;
	int target_x = -1, target_y = -1;

	// 查找当前焦点对应的坐标，找到则显示文本
	if (get_focus_pos(CardManageClass.cur_focus.main, &target_x, &target_y))
	{
		display_yes_text(target_x, target_y, width, height, font_size);
	}
}

static void card_manage_prompt_display(STRING_ID str_id, unsigned int prompt_color)
{
	position box_pos = {{152, 73}, {236, 66}};
	position text_pos = {{152, 73}, {236, 66}};
	resource res = resource_get(ROM_R_IMG_CARD_MANEAGE_RFID_FOCUS_PNG);
	icon prompt_bg;
	text prompt;

	icon_init(&prompt_bg, &box_pos, &res);
	icon_display(&prompt_bg);

	text_init(&prompt, &text_pos, 20);
	prompt.align = CENTER_MIDDLE;
	prompt.font_color = prompt_color;
	text_display(&prompt, font_str(str_id));
}

static void card_manage_result_text_clear(void)
{
	position tag_pos = {{160, 71}, {280, 40}};
	position save_pos = {{160, 157}, {160, 40}};

	gui_erase(&tag_pos, 0x00000000);
	gui_erase(&save_pos, 0x00000000);
}

static void card_manage_success_result_clear(void)
{
	CardManageClass.room_card_info.room_card_num = 0;
	memset(SwipingCard.string_buf[10], 0, sizeof(SwipingCard.string_buf[10]));
	SwipingCard.success_show = false;
	card_manage_result_text_clear();
}

static void card_manage_page_redraw(void)
{
	position page_pos = {{0, 0}, {480, 193}};

	gui_erase(&page_pos, 0x00000000);
	CardManageClass.widget_show.icon();
	CardManageClass.widget_show.font();
	CardManageClass.widget_show.focus();
	CardManageClass.widget_show.dialog_box();
}

static void card_manage_prompt_close(void)
{
	if (CardManageClass.cur_focus.status == CARD_MANAGE_STATUS_NONE)
	{
		return;
	}

	CardManageClass.cur_focus.status = CARD_MANAGE_STATUS_NONE;
	CardManageClass.room_card_info.card_number_status_count = 0;
	card_manage_page_redraw();

	if (SwipingCard.mode == CARD_ADD_CARD_MODE)
	{
		room_card_numbe_display();
		room_card_string_buf_display();
	}
	else
	{
		card_manage_success_result_clear();
	}
}

static bool card_manage_prompt_consume_key(void)
{
	if (CardManageClass.cur_focus.status == CARD_MANAGE_STATUS_NONE)
	{
		return false;
	}

	card_manage_prompt_close();
	return true;
}

static void card_manage_prompt_show(CARD_MANAGE_STATUS status)
{
	CardManageClass.cur_focus.status = status;
	CardManageClass.room_card_info.card_number_status_count = 0;

	switch (status)
	{
	case CARD_MANAGE_STATUS_ERROR:
		card_manage_prompt_display(STR_CARD_NUMBER_UNIT_NUMBER_ERROR, CARD_PROMPT_ERROR_COLOR);
		break;
	case CARD_MANAGE_STATUS_DELETE_CARD:
		card_manage_prompt_display(STR_CARD_NUMBER_DELETE_SUCCESS, CARD_PROMPT_SUCCESS_COLOR);
		break;
	case CARD_MANAGE_STATUS_SAVE_CARD:
		card_manage_prompt_display(STR_CARD_NUMBER_ADD_SUCCESS, CARD_PROMPT_SUCCESS_COLOR);
		break;
	case CARD_MANAGE_STATUS_SUCCESS:
		card_manage_prompt_display(STR_CARD_NUMBER_SUCCESS, CARD_PROMPT_SUCCESS_COLOR);
		break;
	default:
		break;
	}
}

static void room_card_numbe_display(void)
{
	position pos = {{160, 157}, {160, 40}};
	text system_set;
	char string[5];
	text_init(&system_set, &pos, 26);
	system_set.align = LEFT_TOP;
	gui_erase(&pos, 0x00);
	sprintf(string, "%d", CardManageClass.room_card_info.room_card_num);
	text_display(&system_set, string);
}
static void room_card_string_buf_display(void)
{
	position pos = {{160, 71}, {280, 40}};
	text system_set;
	char string[32] = {0}; // 初始化缓冲区，避免垃圾数据
	text_init(&system_set, &pos, 26);
	system_set.align = LEFT_MIDDLE;
	gui_erase(&pos, 0x00);

	LOG_WHITE("string_buf: %s\n", SwipingCard.string_buf[10]);

	char *src_str = SwipingCard.string_buf[10];
	char *left_trim_str = src_str + 3;
	int trim_len = strlen(left_trim_str);
	char hex_str[32] = {0};	   // 存储裁剪后的纯十六进制字符串
	char target_num[32] = {0}; // 存储最终的数字字符串

	//  得到纯十六进制字符串
	if (trim_len > 2)
	{
		strncpy(hex_str, left_trim_str, trim_len - 4);
	}
	else
	{
		strncpy(hex_str, left_trim_str, sizeof(hex_str) - 1);
	}

	LOG_WHITE("trimmed hex string_buf: %s\n", hex_str);

	// 逐两位解析十六进制字符串，转为数字字符并拼接
	int hex_len = strlen(hex_str);
	int target_idx = 0; // 目标数字字符串的索引

	// 遍历hex_str，每两个字符为一组处理
	for (int i = 0; i < hex_len && (i + 1) < hex_len; i += 2)
	{
		// 将两位十六进制字符转为对应的十六进制数值
		char hex_char1 = hex_str[i];
		char hex_char2 = hex_str[i + 1];
		// 十六进制字符转数值
		int val1 = (hex_char1 >= '0' && hex_char1 <= '9') ? (hex_char1 - '0') : (hex_char1 - 'A' + 10);
		int val2 = (hex_char2 >= '0' && hex_char2 <= '9') ? (hex_char2 - '0') : (hex_char2 - 'A' + 10);
		// 组合成两位十六进制数值
		unsigned char ascii_code = (val1 << 4) | val2;

		// 将ASCII码转为对应的数字字符
		if (ascii_code >= '0' && ascii_code <= '9')
		{
			target_num[target_idx++] = ascii_code; // 拼接数字字符到target_num
		}
	}

	LOG_WHITE("最终拼接的数字字符串: %s\n", target_num);

	//  两种显示方式
	// 直接显示完整数字字符串
	sprintf(string, "%s", target_num);

	// 去掉前导0，显示14206993
	//  char *no_prefix_zero = target_num;
	//  while (*no_prefix_zero == '0' && *(no_prefix_zero + 1) != '\0')
	//  {
	//      no_prefix_zero++;
	//  }
	//  sprintf(string, "%s", no_prefix_zero);

	LOG_WHITE("decimal value string: %s\n", string);
	text_display(&system_set, string);
}
static void card_manage_icon_display(void)
{
	taba_btn_display();
}
static void Unit_font_display(void)
{
	position pos = {{33, 28}, {120, 40}};
	text system_set;

	text_init(&system_set, &pos, 26);
	system_set.align = LEFT_TOP;
	text_display(&system_set, font_str(STR_CARD_MANAGE_UNIT));
}

static void Tag_font_display(void)
{
	position pos = {{33, 71}, {120, 40}};
	text unlock_set;

	text_init(&unlock_set, &pos, 26);
	unlock_set.align = LEFT_TOP;
	text_display(&unlock_set, font_str(STR_CARD_MANAGE_TAG));
}

static void Erase_font_display(void)
{
	position pos = {{33, 114}, {120, 40}};
	text password_set;

	text_init(&password_set, &pos, 26);
	password_set.align = LEFT_TOP;
	text_display(&password_set, font_str(STR_CARD_MANAGE_ERASE));
}

static void Save_font_display(void)
{
	position pos = {{33, 157}, {120, 40}};
	text password_set;

	text_init(&password_set, &pos, 26);
	password_set.align = LEFT_TOP;
	text_display(&password_set, font_str(STR_CARD_MANAGE_SAVE));
}

static void card_manage_font_display(void)
{
	Unit_font_display();
	Tag_font_display();
	Erase_font_display();
	Save_font_display();
}

static void clear_prev_card_manage_focus(void)
{

	position pos = {{430, 27}, {32, 32}};
	pos.point.y = 27 + 42 * CardManageClass.cur_focus.main;
	gui_erase(&pos, 0x00);
}

static void display_current_card_manage_focus(void)
{
	icon focus;
	position pos = {{430, 27}, {32, 32}};

	pos.point.y = 27 + 42 * CardManageClass.cur_focus.main;
	resource res = resource_get(ROM_R_IMG_SET_PAGE_FOCUS_PNG);
	icon_init(&focus, &pos, &res);
	icon_display(&focus);
}

static void card_manage_focus_display(void)
{
	clear_prev_card_manage_focus();
	display_current_card_manage_focus();
}

static void card_mange_dialog_box_init(void)
{
	icon box;
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{0, 0}, {0, 0}}, 0, 0, 3};
	static char string[20];

	position box_pos = {{160, 28}, {160, 40}};
	icon_init(&box, &box_pos, NULL);
	box.res.id = 0;
	position pos = {{160, 28}, {160, 40}};
	text_init(&font.text1, &pos, 26);
	font.string1 = string;
	font.text1.align = LEFT_TOP;

	widget_dialog_box_init(CardManageClass.dialog_box,
						   &box,
						   &font,
						   &cursor);
}
static void card_manage_dialog_display(void)
{
	widget_dialog_box_display(CardManageClass.dialog_box);
}

static void goto_prev_card_manage_focus(void)
{
	(CardManageClass.cur_focus.main == SAVE_FOCUS) ? (CardManageClass.cur_focus.main = ERASE_FOCUS) : (CardManageClass.cur_focus.main == ERASE_FOCUS) ? (CardManageClass.cur_focus.main = TAG_FOCUS)
																								  : (CardManageClass.cur_focus.main == TAG_FOCUS)	  ? (CardManageClass.cur_focus.main = UNIT_FOCUS)
																								  : (CardManageClass.cur_focus.main == UNIT_FOCUS)	  ? (CardManageClass.cur_focus.main = SAVE_FOCUS)
																																					  : (CardManageClass.cur_focus.main = SAVE_FOCUS);
}

static void goto_next_card_manage_focus(void)
{
	(CardManageClass.cur_focus.main == UNIT_FOCUS) ? (CardManageClass.cur_focus.main = TAG_FOCUS) : (CardManageClass.cur_focus.main == TAG_FOCUS) ? (CardManageClass.cur_focus.main = ERASE_FOCUS)
																								: (CardManageClass.cur_focus.main == ERASE_FOCUS) ? (CardManageClass.cur_focus.main = SAVE_FOCUS)
																								: (CardManageClass.cur_focus.main == SAVE_FOCUS)  ? (CardManageClass.cur_focus.main = UNIT_FOCUS)
																																				  : (CardManageClass.cur_focus.main = UNIT_FOCUS);
}

static int calculateHomeId(void)
{
	// 健壮性检查
	if (CardManageClass.room_card_info.home_id == NULL)
	{
		return -1;
	}

	// 核心计算逻辑
	int home_id = CardManageClass.room_card_info.home_id[0] * 1000 +
				  CardManageClass.room_card_info.home_id[1] * 100 +
				  CardManageClass.room_card_info.home_id[2] * 10 +
				  CardManageClass.room_card_info.home_id[3];

	return home_id;
}
static void card_manage_dialog_box_font_change(void)
{
	switch (CardManageClass.dialog_box->cursor.index)
	{
	case 0:
		memset(CardManageClass.dialog_box->font.string1, 0, 10);
		break;
	case 1:
		sprintf(CardManageClass.dialog_box->font.string1,
				"%d",
				CardManageClass.room_card_info.home_id[0]);
		break;
	case 2:
		sprintf(CardManageClass.dialog_box->font.string1,
				"%d %d",
				CardManageClass.room_card_info.home_id[0],
				CardManageClass.room_card_info.home_id[1]);
		break;
	case 3:
		sprintf(CardManageClass.dialog_box->font.string1,
				"%d %d %d",
				CardManageClass.room_card_info.home_id[0],
				CardManageClass.room_card_info.home_id[1],
				CardManageClass.room_card_info.home_id[2]);
		break;
	case 4:
		sprintf(CardManageClass.dialog_box->font.string1,
				"%d %d %d %d",
				CardManageClass.room_card_info.home_id[0],
				CardManageClass.room_card_info.home_id[1],
				CardManageClass.room_card_info.home_id[2],
				CardManageClass.room_card_info.home_id[3]);
		break;
	}
}

static void card_manage_input_add_number(unsigned char number)
{
	if (card_manage_prompt_consume_key())
		return;
	if (CardManageClass.cur_focus.layer == CARD_MANAGE_MAIN_LAYER_CONFIRM)
	{
		return;
	}
	LOG_WHITE("input_add_number %d\n", number);
	unsigned char index = CardManageClass.dialog_box->cursor.index;
	if (CardManageClass.cur_focus.main != UNIT_FOCUS)
	{
		return;
	}

	if (index <= CardManageClass.dialog_box->cursor.max_index)
	{
		CardManageClass.room_card_info.home_id[index] = number;
		CardManageClass.dialog_box->cursor.index++;
		card_manage_dialog_box_font_change();
		CardManageClass.widget_show.dialog_box();
	}
	LOG_WHITE("input_add_number success\n");
}

static void card_manage_input_sub_number(void)
{
	if (card_manage_prompt_consume_key())
		return;
	if (CardManageClass.dialog_box->cursor.index <= 0 || CardManageClass.cur_focus.main != UNIT_FOCUS)
	{
		return;
	}
	else
	{
		CardManageClass.dialog_box->cursor.index--;
		card_manage_dialog_box_font_change();
	}
}

static void card_manage_home_id_adjust(void)
{
	unsigned char i, j;
	unsigned char index = CardManageClass.dialog_box->cursor.index;
	unsigned char max_index = CardManageClass.dialog_box->cursor.max_index;

	for (i = index; i <= max_index; i++)
	{
		for (j = max_index; j > 0; j--)
		{
			CardManageClass.room_card_info.home_id[j] = CardManageClass.room_card_info.home_id[j - 1];
		}
		CardManageClass.room_card_info.home_id[0] = 0;
	}
}
static bool validity_check_unit_number(void)
{
	unsigned int home_id; // 对应输入的那个 四位数 [千 百 十 个]
	home_id = calculateHomeId();

	/* 这里的 home id 同时又是输入的数字 */
	LOG_WHITE("输入的数  %d\n", home_id);

	if (home_id > 0 && home_id < USER_CARD_TOTAL / 10)
	{
		LOG_WHITE("return true\n");
		return true;
	}
	return false;
}

void check_and_set_card_data(int num)
{
	for (int i = 0; i < 10; i++)
	{
		if (strlen(SwipingCard.string_buf[i]) > 0)
		{
			set_card_id_data(num + i, SwipingCard.string_buf[i]);
		}
	}
}

static bool remove_unit_number_by_home_id(int home_id)
{
	int total = get_int_conf(UNIT_NUMBER_INDEX);
	int max_total = sizeof(UserData.unit_number) / sizeof(UserData.unit_number[0]);

	if (total <= 0)
	{
		return false;
	}
	if (total > max_total)
	{
		total = max_total;
	}

	for (int i = 0; i < total; i++)
	{
		if (UserData.unit_number[i] == home_id)
		{
			for (int j = i; j < total - 1; j++)
			{
				UserData.unit_number[j] = UserData.unit_number[j + 1];
			}
			UserData.unit_number[total - 1] = -1;
			set_int_conf(UNIT_NUMBER_INDEX, total - 1);
			return true;
		}
	}

	return false;
}

static void *saveCardData(void *arg)
{
	pthread_mutex_lock(&card_mutex);
	check_and_set_card_data((int)arg);
	card_id_data_save();
	pthread_mutex_unlock(&card_mutex);
	ak_thread_exit();
	return NULL;
}

static void card_manage_dialog_box_init(CARD_MANAGE_STATUS status_param)
{

	if (CardManageClass.dialog_box == NULL)
	{
		return;
	}

	// ========== 固定执行的核心逻辑 ==========

	CardManageClass.dialog_box->cursor.index = 0;
	memset(CardManageClass.dialog_box->font.string1, 0, 10);
	card_manage_page_redraw();
	CardManageClass.cur_focus.layer = CARD_MANAGE_MAIN_LAYER;

	if (status_param != CARD_MANAGE_STATUS_NONE && status_param < TOTAL_CARD_MANAGE_STATUS)
	{
		card_manage_prompt_show(status_param);
	}
}

static int get_room_card_number_by_room_num(int room_num)
{
	CardManageClass.room_card_info.room_card_num = 0;
	for (int i = 0; i < 10; i++)
	{
		if (strlen(get_card_id_data(room_num + i)) == 0)
		{

			break;
		}
		CardManageClass.room_card_info.room_card_num++;
	}
	return CardManageClass.room_card_info.room_card_num;
}
static void card_manage_key0_up(void)
{
	card_manage_input_add_number(0);
}
static void card_manage_key1_up(void)
{
	card_manage_input_add_number(1);
}
static void card_manage_key2_up(void)
{
	card_manage_input_add_number(2);
}
static void card_manage_key3_up(void)
{
	card_manage_input_add_number(3);
}
static void card_manage_key4_up(void)
{
	card_manage_input_add_number(4);
}
static void card_manage_key5_up(void)
{
	card_manage_input_add_number(5);
}
static void card_manage_key6_up(void)
{
	card_manage_input_add_number(6);
}
static void card_manage_key7_up(void)
{
	card_manage_input_add_number(7);
}
static void card_manage_key8_up(void)
{
	card_manage_input_add_number(8);
}
static void card_manage_key9_up(void)
{
	card_manage_input_add_number(9);
}
static void card_manage_key_up_up(void)
{
	LOG_WHITE("card_manage_key_up_up\n");
	if (card_manage_prompt_consume_key())
		return;
	clear_prev_card_manage_focus();
	goto_prev_card_manage_focus();
	display_current_card_manage_focus();
	LOG_WHITE("card_manage_key_up_up sucess\n");
}

static void card_manage_key_down_up(void)
{
	if (card_manage_prompt_consume_key())
		return;
	clear_prev_card_manage_focus();
	goto_next_card_manage_focus();
	display_current_card_manage_focus();
}

static void card_manage_key_star_up(void)
{
	if (card_manage_prompt_consume_key())
		return;
	if (CardManageClass.dialog_box->cursor.index == 0 || CardManageClass.cur_focus.main != UNIT_FOCUS) // 返回
		os_layout_goto(&layout_settings);

	else if (CardManageClass.cur_focus.layer == CARD_MANAGE_MAIN_LAYER_CONFIRM)
		os_layout_goto(&layout_card_manage);

	else // 删除一个数
	{
		card_manage_input_sub_number();
		gui_erase(&CardManageClass.dialog_box->box.pos, 0XFF20428A);
		CardManageClass.widget_show.dialog_box();
	}
}

/***********
 * @function: 确认按钮
 * @os_layout_goto:页面跳转
 */
static void card_manage_key_ring_up(void)
{
	if (card_manage_prompt_consume_key())
		return;
	if (CardManageClass.cur_focus.layer == CARD_MANAGE_MAIN_LAYER)
	{
		switch (CardManageClass.cur_focus.main)
		{
		case UNIT_FOCUS:
			if (CardManageClass.dialog_box->cursor.index == 0)
				return;
			CardManageClass.cur_focus.layer = CARD_MANAGE_MAIN_LAYER_CONFIRM;
			card_manage_home_id_adjust();
			if (validity_check_unit_number() == true)
			{
				int home_id = calculateHomeId();
				get_room_card_number_by_room_num(home_id * 10);
				room_card_numbe_display();
				if (unit_number_exist(home_id) == false)
				{
					UserData.unit_number[get_int_conf(UNIT_NUMBER_INDEX)] = home_id;
					user_data_save();
					set_int_conf(UNIT_NUMBER_INDEX, get_int_conf(UNIT_NUMBER_INDEX) + 1);
				}
				memset(SwipingCard.string_buf, 0, sizeof(SwipingCard.string_buf));
				SwipingCard.mode = CARD_ADD_CARD_MODE;
			}
			else
			{
				card_manage_dialog_box_init(CARD_MANAGE_STATUS_ERROR);
			}
			break;

		case ERASE_FOCUS:
			card_manage_prompt_show(CARD_MANAGE_STATUS_ERROR);
			break;

		case SAVE_FOCUS:
			card_manage_prompt_show(CARD_MANAGE_STATUS_ERROR);
			break;

		default:
			break;
		}
	}
	else
	{
		switch (CardManageClass.cur_focus.main)
		{
		case UNIT_FOCUS:
			break;

		case ERASE_FOCUS:
			SwipingCard.mode = CARD_IDLE_MODE;
			ak_thread_create(&thread_delete_card, delete_current_card,
							 (void *)(calculateHomeId() * 10), ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
			card_manage_dialog_box_init(CARD_MANAGE_STATUS_DELETE_CARD);
			break;

		case SAVE_FOCUS:
			SwipingCard.mode = CARD_IDLE_MODE;
			ak_thread_create(&thread_save_card, saveCardData,
							 (void *)(calculateHomeId() * 10), ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
			card_manage_dialog_box_init(CARD_MANAGE_STATUS_SAVE_CARD);
			break;
		default:
			break;
		}
	}
}

static void card_manage_key_register(void)
{
	key_touch_handle_register(KEY_INDEX_0, KEY_STATE_DOWN, card_manage_key0_up);
	key_touch_handle_register(KEY_INDEX_1, KEY_STATE_DOWN, card_manage_key1_up);
	key_touch_handle_register(KEY_INDEX_2, KEY_STATE_DOWN, card_manage_key2_up);
	key_touch_handle_register(KEY_INDEX_3, KEY_STATE_DOWN, card_manage_key3_up);
	key_touch_handle_register(KEY_INDEX_4, KEY_STATE_DOWN, card_manage_key4_up);
	key_touch_handle_register(KEY_INDEX_5, KEY_STATE_DOWN, card_manage_key5_up);
	key_touch_handle_register(KEY_INDEX_6, KEY_STATE_DOWN, card_manage_key6_up);
	key_touch_handle_register(KEY_INDEX_7, KEY_STATE_DOWN, card_manage_key7_up);
	key_touch_handle_register(KEY_INDEX_8, KEY_STATE_DOWN, card_manage_key8_up);
	key_touch_handle_register(KEY_INDEX_9, KEY_STATE_DOWN, card_manage_key9_up);

	key_touch_handle_register(KEY_INDEX_UP, KEY_STATE_DOWN, card_manage_key_up_up);
	key_touch_handle_register(KEY_INDEX_DOWN, KEY_STATE_DOWN, card_manage_key_down_up);
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, card_manage_key_star_up);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, card_manage_key_ring_up);
}

static void layout_card_manage_init(void)
{
	CardManageClass.cur_focus.status = CARD_MANAGE_STATUS_NONE; // 重置状态
	card_mange_dialog_box_init();
}

static void layout_card_manage_enter(void)
{
	LOG_WHITE(">>> enter layout card manage \n\r");
	CardManageClass.room_card_info.card_number_status_count = 0;

	CardManageClass.widget_show.icon();
	CardManageClass.widget_show.font();
	CardManageClass.widget_show.focus();
	CardManageClass.key_register();

	LOG_WHITE("enter layout card manage success\n");
}

static void layout_card_manage_quit(void)
{
	LOG_WHITE(">>> quit layout card manage \n\r");

	SwipingCard.mode = CARD_IDLE_MODE;
	CardManageClass.cur_focus.layer = CARD_MANAGE_MAIN_LAYER;
	CardManageClass.cur_focus.main = UNIT_FOCUS;
	CardManageClass.cur_focus.status = CARD_MANAGE_STATUS_NONE; // 重置状态
	CardManageClass.room_card_info.card_number_status_count = 0;

	CardManageClass.dialog_box->cursor.index = 0;
	memset(CardManageClass.dialog_box->font.string1, 0, 10);
}
static void *delete_current_card(void *arg)
{
	int card_base = (int)arg;
	int home_id = card_base / 10;

	LOG_WHITE("start delet %d\n", card_base);
	char string[32] = {0};

	if (card_base < 0 || card_base + 9 >= USER_CARD_TOTAL)
	{
		LOG_RED("delete card index out of range:%d\n", card_base);
		ak_thread_exit();
		return NULL;
	}

	for (int i = 0; i < 10; i++)
	{
		set_card_id_data(card_base + i, string);
	}
	remove_unit_number_by_home_id(home_id);
	card_id_data_save();
	user_data_save();
	ak_thread_exit();
	return NULL;
}

static void handle_swiping_success(int num)
{
	SwipingCard.success_show = false;
	// get_room_card_number_by_room_num(num);
	room_card_numbe_display();
	room_card_string_buf_display();
}

static void card_manage_success_font_display(void)
{
	int home_id = calculateHomeId();
	int num = home_id * 10;

	// 主状态处理
	if (CardManageClass.cur_focus.status != CARD_MANAGE_STATUS_NONE)
	{
		CardManageClass.room_card_info.card_number_status_count++;
		if (CardManageClass.room_card_info.card_number_status_count >= CARD_MANAGE_PROMPT_TIMEOUT_TICKS)
			card_manage_prompt_close();
	}

	// 刷卡成功显示
	if (SwipingCard.success_show)
	{
		handle_swiping_success(num);
	}
}

static void layout_card_manage_timer(void)
{
	card_manage_success_font_display();
}

STR_WidgetDialogBox CardMangeDdialgBox;

STR_CardManage CardManageClass = {
	{card_manage_icon_display, card_manage_font_display, card_manage_focus_display, card_manage_dialog_display, NULL},
	&CardMangeDdialgBox,
	card_manage_key_register,
	{UNIT_FOCUS, CARD_MANAGE_MAIN_LAYER, CARD_MANAGE_STATUS_NONE},
	{{0}, 0, 0}};

layout layout_card_manage = {
	.init = layout_card_manage_init,
	.enter = layout_card_manage_enter,
	.quit = layout_card_manage_quit,
	.timer = layout_card_manage_timer};
