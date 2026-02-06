#include "png.h"
#include <memory.h>
#include <stdbool.h>
#include "ui_api.h"
#include "ak_mem.h"
#include "ui.h"

typedef struct{

	unsigned char* addres;
	int len;
	int offset;
}png_image_source;

bool position_adj(const position* src_pos,int dst_width,int dst_height,align align,position* dst_pos);

static void png_read_fn_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
    png_image_source* source = (png_image_source*)png_get_io_ptr(png_ptr);
    if(source->offset + length <= source->len)
	{
       memcpy(data, source->addres + source->offset, length);
       source->offset += length;
    }
	else
	{
        png_error(png_ptr,"png_read_fn_callback failed");
    }
}

extern unsigned char*rom_base_addres;
unsigned char* fb_gui_addres_get(const position* pos,int* row_byte);
bool png_decodec(png_decodec_info* info)
	//(const resource* res,const position* src_pos,const point* offset,align align)
{
	//unsigned long long start = os_get_ms();
    bool reslut = true;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));

    png_image_source png_source;
	png_source.addres = (unsigned char*)rom_base_addres + info->res->id;
	png_source.len = info->res->size;
	png_source.offset = 0;
	png_set_read_fn(png_ptr,(void*)&png_source,png_read_fn_callback);
	
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0); 
	
	/*********************
	只支持解码argb格式图片
	*********************/
	
	if((png_get_channels(png_ptr, info_ptr) != 4)||(png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGB_ALPHA)){

		reslut = false;
		goto PNG_DECODEC_FINISH;
	}

    int image_width = png_get_image_width(png_ptr, info_ptr);
    int image_height = png_get_image_height(png_ptr, info_ptr);

	position display_pos = {{0},{0}};
	if(position_adj(info->pos,image_width,image_height,info->align,&display_pos) == false)
	{
		reslut = false;
		goto PNG_DECODEC_FINISH;
	}
	if(info->offset!= NULL)
	{
    	display_pos.point.x += info->offset->x;
		display_pos.point.y += info->offset->y;
	}
    

    png_bytepp row_pointers = png_get_rows(png_ptr,info_ptr);

	int row_byte;
	unsigned char* dst_addr_start = fb_gui_addres_get(&display_pos, &row_byte);
	
	unsigned char* src_addr_start = NULL;
	unsigned char alpha = 0x00,sub_alpha = 0x00;
	int i,j;
    for( i = 0 ; i < image_height ; i++)
    {
    	src_addr_start = row_pointers[i];
		unsigned char* dst_addr = dst_addr_start;
		for(j = 0 ; j < image_width ; j++)
		{
			alpha = src_addr_start[3];
			if(alpha == 0)
			{   
                    
            }
			else if((alpha == 0xFF)||(dst_addr[3] < ALPHA_LEVEL))
			{
				unsigned int src_argb = *(unsigned int*)src_addr_start;
				*(unsigned int*)dst_addr = (src_argb&0xFF000000)|((src_argb&0xFF)<<16)|(src_argb&0xFF00)|((src_argb&0xFF0000)>>16);
				//dst_addr[3] = src_addr_start[3];
               // dst_addr[2] = src_addr_start[0];
               /// dst_addr[1] = src_addr_start[1];
               // dst_addr[0] = src_addr_start[2];
            }
			else
			{ 	
                sub_alpha = 255 - alpha;
				dst_addr[3] = (src_addr_start[3]*alpha + dst_addr[3]*sub_alpha)>>8;
                dst_addr[2] = (src_addr_start[0]*alpha + dst_addr[2]*sub_alpha)>>8;
                dst_addr[1] = (src_addr_start[1]*alpha + dst_addr[1]*sub_alpha)>>8;
                dst_addr[0] = (src_addr_start[2]*alpha + dst_addr[0]*sub_alpha)>>8;
            }
			src_addr_start += 4;
			dst_addr += 4;
		}
		dst_addr_start += row_byte;           //改动
    }
PNG_DECODEC_FINISH:
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	//printf("png decodec time:%llu ms \n\r",os_get_ms()- start);
	return reslut;
}


bool png_decodec_raw(const resource* res,unsigned char**dst,vector* image_info)
{
	bool reslut = true;

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	setjmp(png_jmpbuf(png_ptr));

    png_image_source png_source;
	png_source.addres = (unsigned char*)rom_base_addres + res->id;
	png_source.len = res->size;
	png_source.offset = 0;
	png_set_read_fn(png_ptr,(void*)&png_source,png_read_fn_callback);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0); 

	/*********************
	只支持解码argb格式图片
	*********************/
	if((png_get_channels(png_ptr, info_ptr) != 4)||(png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGB_ALPHA)){

		reslut = false;
		goto PNG_DECODEC_FINISH;
	}

    int image_width = png_get_image_width(png_ptr, info_ptr);
    int image_height = png_get_image_height(png_ptr, info_ptr);
	int row_byte = image_width*4;
	
	*dst = (unsigned char*)ak_mem_alloc(MODULE_ID_APP, image_height*image_width*4);

    png_bytepp row_pointers = png_get_rows(png_ptr,info_ptr);

	unsigned char* src_addr_start = NULL;
	unsigned char* dst_addr_start = *dst;
	int i,j;
    for( i = 0 ; i < image_height ; i++)
    {
   // 	memcpy(&dst_addr_start[i*row_byte],row_pointers[i],row_byte);
#if 1
    	src_addr_start = row_pointers[i];
		unsigned char* dst_addr = dst_addr_start;
		for(j = 0 ; j < image_width ; j++)
		{
			
			dst_addr[3] = src_addr_start[3];
            dst_addr[2] = src_addr_start[0];
            dst_addr[1] = src_addr_start[1];
            dst_addr[0] = src_addr_start[2];
            
			src_addr_start += 4;
			dst_addr += 4;
		}
		dst_addr_start += row_byte;
	#endif
    }
	image_info->width = image_width;
	image_info->height = image_height;
	
PNG_DECODEC_FINISH:
	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	return reslut;
}


