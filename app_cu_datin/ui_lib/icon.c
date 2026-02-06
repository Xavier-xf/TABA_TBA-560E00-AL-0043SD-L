#include "ui_api.h"
#include "os_sys_api.h"
#include "ui.h"

bool icon_init(icon* icon,const position* pos,const resource* res)
{
    if(pos != NULL)
    {
		icon->pos = *pos;
	}

	if(res != NULL)
    {
		icon->res = *res;
	}

	icon->offset.x = icon->offset.y = 0;

	/******************
	默认对其为居中对齐
	******************/
	icon->align = CENTER_MIDDLE;

	/*****************
	默认不需要背景色
	*****************/
	icon->bg_color = 0x00;
	icon->erase = false;

	return true;
}


bool icon_display(const icon*icon)
{
	if(icon->erase == true)
	{
		gui_erase(&(icon->pos), (icon->bg_color&0xFF000000)?icon->bg_color:0x00);
	}
	else if(icon->bg_color&0xFF000000)
	{
		draw_rect(&(icon->pos), icon->bg_color);
	}
	

	if(icon->res.id && icon->res.size)
	{
		//unsigned long long start = os_get_ms();
		png_decodec_info info;
		info.align = icon->align;
		info.offset = &icon->offset;
		info.pos = &icon->pos;
		info.res = &icon->res;
		png_decodec(&info);
			//(&icon->res, &icon->pos, &icon->offset, icon->align);
		//printf("outdoor-----png----->> time:%llu ms \n\r",os_get_ms()- start);
	}
	return true;
}


