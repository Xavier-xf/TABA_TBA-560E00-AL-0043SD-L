#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"

static void swiping_card_icon_card_display(void)
{
	icon card_icon;
	position pos = {{190, 81}, {100, 110}};
	resource res = resource_get(ROM_R_IMG_SWIPE_PAGE_CARD_PNG);
	icon_init(&card_icon, &pos, &res);
	icon_display(&card_icon);
}

static void swiping_card_icon_display(void)
{
	taba_btn_display();
	swiping_card_icon_card_display();
}

static void put_card_font_display(void)
{
	position put_pos = {{75, 202}, {338, 50}};
	text put_card;

	text_init(&put_card, &put_pos, 20);
	text_display(&put_card, font_str(STR_SWIPING_CARD_PUT_CARD));
}
static void success_font_display(void)
{
	position put_pos = {{135, 33}, {200, 50}};
	text put_card;

	text_init(&put_card, &put_pos, 24);
	put_card.font_color = 0xFF00FF00; // 设置为绿色
	text_display(&put_card, font_str(STR_SWIPING_CARD_SUCCESS));
}
static void clear_success_font(void)
{
	position put_pos = {{108, 33}, {280, 50}};
	gui_erase(&put_pos, 0X00000000);
}

static void swiping_card_font_display(void)
{
	put_card_font_display();
}

static void swiping_card_key_star_up(void)
{

	os_layout_goto(&layout_card_manage);
}

static void swiping_card_key_register(void)
{

	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_UP, swiping_card_key_star_up);
}

static void layout_swiping_card_init(void)
{
	SwipingCard.success_show = false;
}

static void layout_swiping_card_enter(void)
{
	LOG_WHITE(">>> enter layout swiping card \n\r");

	SwipingCardClass.widget_show.icon();
	SwipingCardClass.widget_show.font();
	SwipingCardClass.key_register();
	SwipingCard.mode = CARD_ADD_CARD_MODE;
}

static void *saveCardData(void *arg)
{
	pthread_mutex_lock(&card_mutex);
	card_id_data_save();

	pthread_mutex_unlock(&card_mutex);
	pthread_exit(NULL);
}
static pthread_t thread_save_card;
static void layout_swiping_card_quit(void)
{
	LOG_WHITE(">>> quit layout swiping card \n\r");
	SwipingCard.mode = CARD_IDLE_MODE;
	pthread_create(&thread_save_card, NULL, saveCardData, NULL);
}

static void layout_swiping_card_timer(void)
{
	struct ak_timeval now;
	ak_get_ostime(&now);

	if (SwipingCard.success_show)
	{
		// 显示成功文字
		success_font_display();

		clear_success_font();
		SwipingCard.success_show = false; // 重置状态
	}
}

STR_SwipingCard SwipingCardClass = {
	{swiping_card_icon_display, swiping_card_font_display, NULL, NULL, NULL},
	swiping_card_key_register,
};

layout layout_swiping_card = {
	.init = layout_swiping_card_init,
	.enter = layout_swiping_card_enter,
	.quit = layout_swiping_card_quit,
	.timer = layout_swiping_card_timer};
