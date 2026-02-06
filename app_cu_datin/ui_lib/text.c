#include "ui_api.h"
#include "ui.h"

bool text_init(text* text,const position* pos,int font_size)
{
    text->font_size = font_size;
	text->align = CENTER_MIDDLE;
	text->font_color = 0xFFFFFFFF;
	text->bg_color =  0x00;
	text->erase = false;
	text->offset.x = text->offset.y = 0;
	text->pos = *pos;
	return true;
}


bool text_display(text*text,const char* string)
{
	if(text->erase == true)
	{
		gui_erase(&(text->pos), (text->bg_color&0xFF000000)?text->bg_color:0x00);
	}
	else if(text->bg_color&0xFF000000)
	{
		draw_rect(&(text->pos), text->bg_color);
	}
  
	if(string)
    {
    	//unsigned long long start = os_get_ms();
    	font_decodec_info info;
		info.string = string;
		info.font_color = text->font_color;
		info.font_size = text->font_size;
		info.offset = &text->offset;
		info.pos = &text->pos;
		info.align = text->align;
		font_decodec(&info);
			//(string, text->font_color,&text->pos, &text->offset, text->align,text->font_size);
		//printf("outdoor text decode:%llu ms \n\r",os_get_ms()- start);
	}
	return true;
}


