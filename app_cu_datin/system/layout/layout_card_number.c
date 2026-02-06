#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "string.h"

#include "ak_thread.h"

static int room_card_num = 0;

static void input_card_number_font_display(void)
{
	position pos = {{81, 45}, {310, 50}};
	text input_card_number;

	text_init(&input_card_number, &pos, 20);
	text_display(&input_card_number, font_str(STR_CARD_NUMBER_INPUT_UNIT_NUMBER));
}

static void card_number_star_font_display(void)
{
	position pos = {{265, 175}, {100, 50}};
	text star_corre;

	text_init(&star_corre, &pos, 20);
	star_corre.align = RIGHT_MIDDLE;
	text_display(&star_corre, font_str(STR_DIAL_STAR_CORRE));
}

static void card_number_success_font_display(void)
{

	position pos = {{100, 108}, {258, 56}};
	text success;

	text_init(&success, &pos, 20);
	text_display(&success, font_str(STR_CARD_NUMBER_SUCCESS));
}

// "Room number error"
static void room_unit_number_error_font_display(void)
{
	position pos = {{100, 108}, {258, 56}};
	text error;

	text_init(&error, &pos, 20);
	text_display(&error, font_str(STR_CARD_NUMBER_UNIT_NUMBER_ERROR));
}

// static void card_number_status_font_erase(void)
// {
// 	// position pos = {{111,200},{258,50}};		// old

// 	position pos = {{100, 108}, {258, 56}};
// 	gui_erase(&pos, 0x00000000);
// 	// card_number_dialog_display()
// }

static void password_star_icon_display(void)
{
	icon star;
	position pos = {{368, 178}, {42, 42}};

	resource res = resource_get(ROM_R_IMG_CALL_PAGE_STAR_ICON_PNG);
	icon_init(&star, &pos, &res);

	icon_display(&star);
}
static void card_number_icon_display(void)
{
	password_star_icon_display();
}

static void card_number_font_display(void)
{

	input_card_number_font_display();
	card_number_star_font_display();
}

static void card_number_dialog_box_font_change(void)
{
	switch (CardNumberClass.dialog_box->cursor.index)
	{
	case 0:
		memset(CardNumberClass.dialog_box->font.string1, 0, 10);
		break;
	case 1:
		sprintf(CardNumberClass.dialog_box->font.string1,
				"%d",
				CardNumberClass.home_id[0]);
		break;
	case 2:
		sprintf(CardNumberClass.dialog_box->font.string1,
				"%d %d",
				CardNumberClass.home_id[0],
				CardNumberClass.home_id[1]);
		break;
	case 3:
		sprintf(CardNumberClass.dialog_box->font.string1,
				"%d %d %d",
				CardNumberClass.home_id[0],
				CardNumberClass.home_id[1],
				CardNumberClass.home_id[2]);
		break;
	case 4:
		sprintf(CardNumberClass.dialog_box->font.string1,
				"%d %d %d %d",
				CardNumberClass.home_id[0],
				CardNumberClass.home_id[1],
				CardNumberClass.home_id[2],
				CardNumberClass.home_id[3]);
		break;
	}
}

void card_number_input_sub_number(void)
{

	if (CardNumberClass.dialog_box->cursor.index > 0)
	{
		CardNumberClass.dialog_box->cursor.index--;
		card_number_dialog_box_font_change();
	}
}

void card_number_input_add_number(unsigned char number)
{
	unsigned char index = CardNumberClass.dialog_box->cursor.index;

	if (index <= CardNumberClass.dialog_box->cursor.max_index)
	{
		CardNumberClass.home_id[index] = number;
	}
	if (index <= CardNumberClass.dialog_box->cursor.max_index)
	{
		CardNumberClass.dialog_box->cursor.index++;
		card_number_dialog_box_font_change();
		CardNumberClass.widget_show.dialog_box();
	}
}

static void card_number_home_id_adjust(void)
{
	unsigned char i, j;
	unsigned char index = CardNumberClass.dialog_box->cursor.index;
	unsigned char max_index = CardNumberClass.dialog_box->cursor.max_index;

	for (i = index; i <= max_index; i++)
	{
		for (j = max_index; j > 0; j--)
		{
			CardNumberClass.home_id[j] = CardNumberClass.home_id[j - 1];
		}
		CardNumberClass.home_id[0] = 0;
	}
}

static void card_number_key0_up(void)
{
	card_number_input_add_number(0);
}

static void card_number_key1_up(void)
{
	card_number_input_add_number(1);
}

static void card_number_key2_up(void)
{
	card_number_input_add_number(2);
}

static void card_number_key3_up(void)
{
	card_number_input_add_number(3);
}

static void card_number_key4_up(void)
{
	card_number_input_add_number(4);
}

static void card_number_key5_up(void)
{
	card_number_input_add_number(5);
}

static void card_number_key6_up(void)
{
	card_number_input_add_number(6);
}

static void card_number_key7_up(void)
{
	card_number_input_add_number(7);
}

static void card_number_key8_up(void)
{
	card_number_input_add_number(8);
}

static void card_number_key9_up(void)
{
	card_number_input_add_number(9);
}

static void card_number_key_star_up(void)
{
	if (CardNumberClass.dialog_box->cursor.index == 0) // 返回
	{
		os_layout_goto(&layout_card_manage);
	}
	else // 删除一个数
	{
		card_number_input_sub_number();
		CardNumberClass.widget_show.dialog_box();
	}
}

// /* 检查数字 是否有效 */
// static bool validity_check_unit_number(void)
// {
// 	unsigned int home_id; // 对应输入的那个 四位数 [千 百 十 个]
// 	home_id = CardNumberClass.home_id[0] * 1000 +
// 			  CardNumberClass.home_id[1] * 100 +
// 			  CardNumberClass.home_id[2] * 10 +
// 			  CardNumberClass.home_id[3];

// 	/* 这里的 home id 同时又是输入的数字 */
// 	LOG_WHITE("输入的数  %d\n", home_id);

// 	for (int i = 0; i < get_int_conf(SET_HOME_ID_INDEX); i++)
// 	{
// 		LOG_WHITE("UserData.home_id[%d] = %d\n\033[0m", i, UserData.home_id[i]);
// 		if (UserData.home_id[i] == home_id && home_id < 10000)
// 		{
// 			LOG_WHITE("return true\n");
// 			return true;
// 		}
// 	}
// 	return false;
// }

static void card_number_key_ring_up(void)
{
	if (CardNumberClass.dialog_box->cursor.index == 0)
	{
		return;
	}
	if (CardNumberClass.status != CARD_NUMBER_STATUS_NONE)
	{
		return;
	}

	if (CardNumberClass.trigger_mode == ADD_CARD_MODE)
	{ // add card mode
		card_number_home_id_adjust();
		int home_id = CardNumberClass.home_id[0] * 1000 +
					  CardNumberClass.home_id[1] * 100 +
					  CardNumberClass.home_id[2] * 10 +
					  CardNumberClass.home_id[3];

		if (unit_number_exist(home_id) == false)
		{
			UserData.unit_number[get_int_conf(UNIT_NUMBER_INDEX)] = home_id;
			user_data_save();
			set_int_conf(UNIT_NUMBER_INDEX, get_int_conf(UNIT_NUMBER_INDEX) + 1);
		}
		os_layout_goto(&layout_swiping_card);
	}
	else
	{ // delete
		card_number_home_id_adjust();

		LOG_WHITE("DELETE_CARD_NUMBER_SUCCESS\n");
		CardNumberClass.status = DELETE_CARD_NUMBER_SUCCESS;

		CardNumberClass.dialog_box->cursor.index = 0;
		memset(CardNumberClass.dialog_box->font.string1, 0, 10);
		CardNumberClass.widget_show.dialog_box();
	}
}

static void card_number_key_register(void)
{
	key_touch_handle_register(KEY_INDEX_0, KEY_STATE_DOWN, card_number_key0_up);
	key_touch_handle_register(KEY_INDEX_1, KEY_STATE_DOWN, card_number_key1_up);
	key_touch_handle_register(KEY_INDEX_2, KEY_STATE_DOWN, card_number_key2_up);
	key_touch_handle_register(KEY_INDEX_3, KEY_STATE_DOWN, card_number_key3_up);
	key_touch_handle_register(KEY_INDEX_4, KEY_STATE_DOWN, card_number_key4_up);
	key_touch_handle_register(KEY_INDEX_5, KEY_STATE_DOWN, card_number_key5_up);
	key_touch_handle_register(KEY_INDEX_6, KEY_STATE_DOWN, card_number_key6_up);
	key_touch_handle_register(KEY_INDEX_7, KEY_STATE_DOWN, card_number_key7_up);
	key_touch_handle_register(KEY_INDEX_8, KEY_STATE_DOWN, card_number_key8_up);
	key_touch_handle_register(KEY_INDEX_9, KEY_STATE_DOWN, card_number_key9_up);
	// key_touch_handle_register(KEY_INDEX_UP,    KEY_STATE_DOWN,   	card_number_key_up_up);
	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, card_number_key_star_up);
	key_touch_handle_register(KEY_INDEX_POUND, KEY_STATE_DOWN, card_number_key_ring_up);
}

static void card_number_dialog_display(void)
{

	widget_dialog_box_display(CardNumberClass.dialog_box);
}

static void card_number_dialog_init(void)
{
	icon box;
	static char number_string[10];
	STR_DialogFont font = {{0}, {0}, {0}, NULL, NULL, NULL};
	STR_Cursor cursor = {{{320, 116}, {2, 40}}, 0xFFFFFFFF, 0, 3};

	position box_pos = {{111, 108}, {258, 56}};
	resource box_res = resource_get(ROM_R_IMG_CALL_PAGE_NUMBER_DIALOG_BOX_PNG);
	icon_init(&box, &box_pos, &box_res);

	position text1_pos = {{145, 115}, {150, 50}};
	text_init(&font.text1, &text1_pos, 40);
	font.text1.align = RIGHT_MIDDLE;
	font.string1 = number_string;

	widget_dialog_box_init(CardNumberClass.dialog_box,
						   &box,
						   &font,
						   &cursor);
}

static void card_number_cursor_widget_blink(BLINK blink_state)
{

	STR_Cursor cursor = {{{320, 116}, {2, 40}}, 0xFFFFFFFF, 0, 0};
	if (blink_state == WIDGET_SHOW)
	{
		draw_rect(&cursor.pos, cursor.color);
	}
	else if (blink_state == WIDGET_HIDE)
	{
		gui_erase(&cursor.pos, 0xFF000000);
	}
}

static void card_number_widget_blink_refresh(void)
{
	static struct ak_timeval cursor_start_cpu_time;
	static BLINK cursor_widget_blink_state = WIDGET_SHOW;

	widget_blink(&cursor_start_cpu_time, 1000, &cursor_widget_blink_state, card_number_cursor_widget_blink);
}

static void layout_card_number_init(void)
{

	card_number_dialog_init();
}

static void layout_card_number_enter(void)
{
	LOG_WHITE(">>> enter layout card number \n\r");
	CardNumberClass.widget_show.icon();
	CardNumberClass.widget_show.font();
	CardNumberClass.widget_show.dialog_box();
	CardNumberClass.key_register();
}

static void layout_card_number_quit(void)
{
	LOG_WHITE(">>> quit layout card number \n\r");

	CardNumberClass.dialog_box->cursor.index = 0;
	memset(CardNumberClass.dialog_box->font.string1, 0, 10);
}

static void *delete_current_card(void *arg)
{
	LOG_WHITE("start delet %d\n", (int)arg);
	char string[32] = {0};
	room_card_num = 0;
	UserData.unit_number[(int)arg] = -1;

	for (int i = 0; i < 10; i++)
	{
		if (strlen(get_card_id_data((int)arg + i)) == 0)
		{

			break;
		}
		set_card_id_data((int)arg + i, string);
		room_card_num++;
	}
	card_id_data_save();
	user_data_save();
	set_int_conf(UNIT_NUMBER_INDEX, get_int_conf(UNIT_NUMBER_INDEX) - 1);

	ak_thread_exit();
	return NULL;
}

static ak_pthread_t thread_delete_card;
static void card_number_status_font_display(void)
{
	static char card_number_status_count = 0;

	int home_id = CardNumberClass.home_id[0] * 1000 +
				  CardNumberClass.home_id[1] * 100 +
				  CardNumberClass.home_id[2] * 10 +
				  CardNumberClass.home_id[3];

	if (CardNumberClass.status != CARD_NUMBER_STATUS_NONE)
	{
		if (card_number_status_count == 3)
		{
			if (CardNumberClass.status == DELETE_CARD_NUMBER_SUCCESS)
			{
				int num = home_id * 10;
				ak_thread_create(&thread_delete_card, delete_current_card, (void *)num, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
				// delete_current_card(home_id*10);
				card_number_success_font_display();
			}
			if (CardNumberClass.status == INPUT_CARD_NUMBER_ERROR)
				room_unit_number_error_font_display();
		}

		card_number_status_count++;

		if (card_number_status_count >= 40)
		{
			CardNumberClass.status = CARD_NUMBER_STATUS_NONE;
			card_number_status_count = 0;
			// card_number_status_font_erase();
			card_number_dialog_display();
		}
	}
}

static void layout_card_number_timer(void)
{

	card_number_widget_blink_refresh();
	card_number_status_font_display();
}

STR_WidgetDialogBox CardNumberDialogBox;

STR_CardNumber CardNumberClass = {
	{card_number_icon_display, card_number_font_display, NULL, card_number_dialog_display, NULL}, // widget_show
	&CardNumberDialogBox,																		  // *dialog_box
	card_number_key_register,																	  //  key_register
	DELETE_CARD_MODE,																			  // trigger_mode
	CARD_NUMBER_STATUS_NONE,																	  // status
	{0}																							  // home_id[4]
};

layout layout_card_number = {
	.init = layout_card_number_init,
	.enter = layout_card_number_enter,
	.quit = layout_card_number_quit,
	.timer = layout_card_number_timer};
