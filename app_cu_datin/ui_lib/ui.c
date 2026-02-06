#include "ui_api.h"
#include "ak_thread.h"
#include <string.h>
#include "ak_tde.h"
#include "ak_mem.h"
#include "video_output.h"

#define ROM_BIN_PATH "/app/app/rom.bin"
#define APP_RUN_PATH "/app/app/ANYKA37E.BIN"
// #define RUN_ZIP_PATH "/tmp/ANYKA37EOS"




unsigned char* rom_base_addres = NULL;

extern void(*button_down_proc_callback)(void);

static void anyka_sdk_init(void)
{
    sdk_run_config config = {0};
    config.mem_trace_flag = SDK_RUN_NORMAL;
    ak_sdk_init(&config);
}


static void rom_bin_load(void)
{
    FILE* fp = fopen(ROM_BIN_PATH,"rb");
    fseek(fp,0,SEEK_END);
    long size = ftell(fp);
    fseek(fp,0,SEEK_SET);

    rom_base_addres = (unsigned char*)ak_mem_alloc(MODULE_ID_APP,size);
    fread(rom_base_addres,1,size,fp);
    fclose(fp);
}

// static void app_bin_rom_remove(void)
// { 

//     // system("rm -rf "APP_RUN_PATH);
// 	// system("rm -rf "ROM_BIN_PATH);
// 	system("rm -rf "RUN_ZIP_PATH);
// 	system("sync");
// }




extern void font_file_load(const resource* resource);
extern bool touch_init(int,int,bool);
extern bool	fb_devices_init(const vector*vetctor,char);
extern bool video_output_init(void);

void ui_init(vector* vector,
				const resource* font_resource,
				void(*button_down_pro_func)(void),
				int touch_x_scale,
				int touch_y_scale,
				char rota,
				bool touch_swap_xy)
{
	// printf("--------------------->>>[%s:%d]\n",__func__,__LINE__);
    anyka_sdk_init();
	fb_devices_init(vector,rota);
	video_output_api.init();
    rom_bin_load();
	if(font_resource != NULL)
	{
		font_file_load(font_resource);
	}
    
    // app_bin_rom_remove();
	touch_init(touch_x_scale,touch_y_scale,touch_swap_xy);
	button_down_proc_callback = button_down_pro_func;
}



bool position_adj(const position* src_pos,int dst_width,int dst_height,align align,position* dst_pos)
{

	dst_pos->vector.width = dst_width;
	dst_pos->vector.height = dst_height;

	switch(align) {
		case LEFT_TOP:		
			dst_pos->point.x = src_pos->point.x;				
			dst_pos->point.y = src_pos->point.y;
		break;
		case LEFT_MIDDLE:	
			dst_pos->point.x = src_pos->point.x;
			dst_pos->point.y = src_pos->point.y + (src_pos->vector.height - dst_pos->vector.height)/2;
		break;
		case LEFT_BOTTOM: 	
			dst_pos->point.x = src_pos->point.x;
			dst_pos->point.y = src_pos->point.y + src_pos->vector.height - dst_pos->vector.height;
		break;
		case CENTER_TOP:	
			dst_pos->point.x = src_pos->point.x + (src_pos->vector.width - dst_pos->vector.width)/2;
			dst_pos->point.y = src_pos->point.y;
		break;
		case CENTER_MIDDLE:
			dst_pos->point.x = src_pos->point.x + (src_pos->vector.width - dst_pos->vector.width)/2;
			dst_pos->point.y = src_pos->point.y + (src_pos->vector.height - dst_pos->vector.height)/2;
		break;
		case CENTER_BOTTOM:
			dst_pos->point.x = src_pos->point.x + (src_pos->vector.width - dst_pos->vector.width)/2;
			dst_pos->point.y = src_pos->point.y + src_pos->vector.height - dst_pos->vector.height;
		break;
		case RIGHT_TOP:
			dst_pos->point.x = src_pos->point.x + src_pos->vector.width - dst_pos->vector.width;
			dst_pos->point.y = src_pos->point.y;
		break;
		case RIGHT_MIDDLE:
			dst_pos->point.x = src_pos->point.x + src_pos->vector.width - dst_pos->vector.width;
			dst_pos->point.y = src_pos->point.y + (src_pos->vector.height - dst_pos->vector.height)/2;
		break;
		case RIGHT_BOTTOM:
			dst_pos->point.x = src_pos->point.x + src_pos->vector.width - dst_pos->vector.width;
			dst_pos->point.y = src_pos->point.y + src_pos->vector.height - dst_pos->vector.height;
		break;
		default:
		    return  false;
	}
	if(dst_pos->point.y < 0) dst_pos->point.y = 0;
	if(dst_pos->point.x < 0) dst_pos->point.x = 0;

	return true;
}


void argb8888_to_argb8888(unsigned char* src,const vector* src_vector,const position* disp_pos,align align,point* offset,unsigned char* dst,int dst_row_byte)
{
	position adj;
	adj.vector = *src_vector;
	position_adj(disp_pos, src_vector->width, src_vector->height, align ,&adj);
	if(offset != NULL)
	{
		adj.point.x += offset->x;
		adj.point.y += offset->y;
	}

	unsigned char* src_addr = src;
	unsigned char* dst_addr = dst + (adj.point.y- disp_pos->point.y)*dst_row_byte + (adj.point.x - disp_pos->point.x)*4;

	int src_row_byte = src_vector->width*4;
	
	int j,i;
	for ( j = 0; j < src_vector->height; j ++)
	{
		unsigned char *s, *d;
		s = src_addr;
		d = dst_addr;
		for (i = 0; i < src_vector->width; i ++)
		{
			
			unsigned int s_argb = *(unsigned int *)s;
			unsigned int s_a = s_argb >> 24;
			if(s_a == 0)
			{
				
			}
			else if(s_a == 0xFF)
			{
				*(unsigned int *)d =s_argb;
			}
			else
			{
				unsigned int d_argb = *(unsigned int *)d;
				unsigned int rev_s_a = 255 - s_a;
				unsigned int d_data  = (( d_argb >> 24)* rev_s_a +   s_a * s_a) >> 8;
				d_data <<= 8;
				d_data |= (((d_argb >> 16) & 0xFF) * rev_s_a + ((s_argb >> 16) & 0xFF) * s_a) >> 8;
				d_data <<= 8;
				d_data |= (((d_argb >> 8 ) & 0xFF) * rev_s_a + ((s_argb >>  8) & 0xFF) * s_a) >> 8;
				d_data <<= 8;
				d_data |= (( d_argb  & 0xFF) * rev_s_a + ((s_argb) & 0xFF) * s_a) >> 8;
				*(unsigned int *)d = d_data;

			}
			
			s += 4;
			d += 4;
		}
		src_addr += src_row_byte;
		dst_addr += dst_row_byte;
	}
}



