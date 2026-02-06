#if 0

#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "swiping_card.h"

static unsigned int show_card_number = 0;


static void read_card_icon_display(void){
	icon card_icon;
	position pos = {{100,101},{100,110}};
	resource res = resource_get(ROM_R_IMG_SWIPE_PAGE_CARD_PNG);
	icon_init(&card_icon, &pos, &res);
	icon_display(&card_icon);
}




static void read_card_font_display(void){
	position welcome_pos = {{75,25},{338,50}};
	text welcome;

	text_init(&welcome, &welcome_pos, 20);
	text_display(&welcome, font_str(STR_READ_CARD_NUMBER));

	position room_pos = {{200,100},{338,50}};
	static char * str_room = {"1010"};
	text room_t;
	text_init(&room_t, &room_pos, 20);
	text_display(&room_t, str_room);

}


static void read_card_key_star_up(void){

	os_layout_goto(&layout_dial);

}

static void read_card_key_register(void){

	key_touch_handle_register(KEY_INDEX_STAR, KEY_STATE_DOWN, read_card_key_star_up);

}

static void layout_read_card_init(void){


}

static void layout_read_card_enter(void){
	LOG_WHITE(">>> enter layout swiping card \n\r");
	
	ReadCardClass.widget_show.icon();
	ReadCardClass.widget_show.font();
	ReadCardClass.key_register();
	show_card_number = 1;
}

static void layout_read_card_quit(void){
	LOG_WHITE(">>> quit layout swiping card \n\r");
}

static void layout_read_card_timer(void){

	static char back_count = 0;
	if(show_card_number == 1){
		back_count++;
		if(back_count == 100){
			os_layout_goto(&layout_dial);
			show_card_number = 0;
			back_count = 0;
		}
	}

}




STR_ReadCard ReadCardClass = {
	{read_card_icon_display,read_card_font_display,NULL,NULL,NULL},
	read_card_key_register,
};


layout layout_read_card ={
	.init  = layout_read_card_init,
    .enter = layout_read_card_enter,
    .quit  = layout_read_card_quit,
    .timer = layout_read_card_timer
};

#endif 