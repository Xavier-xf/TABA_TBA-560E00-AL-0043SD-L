#include "ui_api.h"
#include "stdlib.h"
#include <ft2build.h>
#include "math.h"
#include "ak_mem.h"
#include "ui_api.h"
#include "ui.h"
#include "string.h"
#include FT_FREETYPE_H


static int font_file_size = 0;
static unsigned char* font_file_addres = NULL;

extern unsigned char* rom_base_addres;

extern bool position_adj(const position* str_temp_pos,int dst_width,int dst_height,align align,position* dst_pos);

unsigned char* fb_gui_addres_get(const position* pos,int* row_byte);


void font_file_load(const resource* resource)
{
    font_file_size = resource->size;
    font_file_addres = rom_base_addres + resource->id;
}

#if 1
static wchar_t persian_number_index(unsigned char c){

	switch(c){
		case '0': 
			return 0x660;
			break;
		case '1': 
			return 0x661;
			break;
		case '2': 
			return 0x662;
			break;
		case '3': 
			return 0x663;
			break;
		case '4': 
			return 0x664;
			break;
		case '5': 
			return 0x665;
			break;
		case '6': 
			return 0x666;
			break;
		case '7': 
			return 0x667;
			break;
		case '8': 
			return 0x668;
			break;
		case '9': 
			return 0x669;
			break;
		default :
			return 0x660;
			break;
	}
}
#endif

void Reverse22(char str[]){
    int n=strlen(str);
    int i;
	char ptr[100];
    for(i=0;i<=n;i++){
        ptr[i]=str[n-i-1];
    }
    ptr[i-1]='\0';
	strcpy(str,ptr);
}

static int utf2_unicode(wchar_t* wstr,int size,const char* utf8){

	int size_s = strlen(utf8);
    int size_d = size;

    wchar_t *des =wstr;
    const char *src = utf8;
    memset(des, 0, size * sizeof(wchar_t));

	char str_temp[256];
	strcpy(str_temp,src);
    int s = 0, d = 0;
	
	if(language_get() == language_persian)
	{
	   if(str_temp[0] >= 0x30 && str_temp[0] <= 0x39 )
	   {
			Reverse22(&str_temp);
	   } 
	}
	
	

	
    while (s < size_s && d < size_d){
		
		unsigned char c = str_temp[s];
        if ((c & 0x80) == 0){
			if(language_get() == language_persian)
				{
			if(c >= 0x30 && c <= 0x39){
				wchar_t *wideChar = &des[d++];
				*wideChar = persian_number_index(c);
			  
				s ++;
			}else{
				des[d++] += str_temp[s++];
				 }
			
			    }
			else{
				 des[d++] += str_temp[s++];
			}
        }else if((c & 0xE0) == 0xC0){  ///< 110x-xxxx 10xx-xxxx
            wchar_t *wideChar = &des[d++];
            *wideChar  = (str_temp[s + 0] & 0x3F) << 6;
            *wideChar |= (str_temp[s + 1] & 0x3F);
            s += 2;
        }else if((c & 0xF0) == 0xE0){  ///< 1110-xxxx 10xx-xxxx 10xx-xxxx
            wchar_t *wideChar = &des[d++];
            *wideChar  = (str_temp[s + 0] & 0x1F) << 12;
            *wideChar |= (str_temp[s + 1] & 0x3F) << 6;
            *wideChar |= (str_temp[s + 2] & 0x3F);
            s += 3;
        } else if((c & 0xF8) == 0xF0){  ///< 1111-0xxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
            wchar_t *wideChar =  &des[d++];
            *wideChar  = (str_temp[s + 0] & 0x0F) << 18;
            *wideChar  = (str_temp[s + 1] & 0x3F) << 12;
            *wideChar |= (str_temp[s + 2] & 0x3F) << 6;
            *wideChar |= (str_temp[s + 3] & 0x3F);
            s += 4;
        } else {
            wchar_t *wideChar =  &des[d++]; ///< 1111-10xx 10xx-xxxx 10xx-xxxx 10xx-xxxx 10xx-xxxx 
            *wideChar  = (str_temp[s + 0] & 0x07) << 24;
            *wideChar  = (str_temp[s + 1] & 0x3F) << 18;
            *wideChar  = (str_temp[s + 2] & 0x3F) << 12;
            *wideChar |= (str_temp[s + 3] & 0x3F) << 6;
            *wideChar |= (str_temp[s + 4] & 0x3F);
            s += 5;
        }
    }
    return d;
}


 
extern wchar_t* convertArabic(wchar_t* normal,wchar_t* conver_str);
bool font_decodec(font_decodec_info* info)
	//(const char* string, unsigned int font_color , const position* res_pos, const point* offset, align align,int font_size)
{
	//unsigned long long start = os_get_ms();
    FT_Library library;
	FT_Face face;
    unsigned char* buffer = NULL;
    bool reslut = true;

	if(FT_Init_FreeType(&library))
    {
		printf("free init ft fail \n\r");
		reslut =  false;
        goto FONT_DECODEC_FINISH;
	}


	if(FT_New_Memory_Face(library,font_file_addres,font_file_size,0,&face))
    {
		printf("new memory face fail \n\r");
		reslut =  false;
        goto FONT_DECODEC_FINISH;
	}


	if(FT_Set_Char_Size(face,info->font_size*64,0,0,0))
    {
		printf("setting char size fail \n\r");
		reslut =  false;
        goto FONT_DECODEC_FINISH;
	}

    FT_GlyphSlot slot = face->glyph;
	FT_Matrix matrix;

	double angle = ( 0.0 / 360 ) * 3.14159 * 2; 
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

	wchar_t unicode_str[256] = {0};
	int unicode_num = 0;
	if(language_get() == language_persian)
	{
		wchar_t unicode_str_normal[256] = {0};
 		unicode_num = utf2_unicode(unicode_str_normal, strlen(info->string), info->string);
		convertArabic(unicode_str_normal, unicode_str);
	}
	else
	{
		unicode_num = utf2_unicode(unicode_str, strlen(info->string), info->string);
	}
  
	
	
	
	int ascender = face->size->metrics.ascender>>6;
	int image_width = info->pos->vector.width;
	int image_height = ascender*3/2;
	buffer = (unsigned char*)ak_mem_alloc(MODULE_ID_APP,image_width*image_height);
	if(buffer == NULL)
	{
		reslut = false;
        goto FONT_DECODEC_FINISH;
	}
	memset(buffer, 0, image_width*image_height);

    FT_Int  n,i, j, p, q,x_max,y_max,x_base,y_base;
	FT_Vector pen = {0,0};

    for( n = 0 ; n < unicode_num;n++){

        FT_Set_Transform( face, &matrix, &pen );
        if(FT_Load_Char( face, unicode_str[n], FT_LOAD_RENDER ))
        {
            printf("load %c fail \n\r",unicode_str[n]);
            continue;
        }

        x_base = slot->bitmap_left;
        y_base = ascender - slot->bitmap_top;
        x_max = x_base + slot->bitmap.width;
        y_max = y_base + slot->bitmap.rows;
        for ( i = x_base, p = 0; i < x_max; i++, p++ )
        {
            for ( j = y_base, q = 0; j < y_max; j++, q++ )
            {
                if ( i < 0  || j < 0 || i >= info->pos->vector.width || j >= info->pos->vector.height )
                {
                    continue;
                }
                buffer[j*image_width + i] = slot->bitmap.buffer[q*slot->bitmap.width+p];
            }
            
        }
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }	

    position display_pos;
	int width = slot->bitmap_left + slot->bitmap.width;
	int height = image_height;
	if(position_adj(info->pos, width,height, info->align,&display_pos) == false){

		printf("adj position fail %s\n\r",info->string);
		reslut =  false;
        goto FONT_DECODEC_FINISH;
	}
	if(info->offset!= NULL)
	{
    	display_pos.point.x += info->offset->x;
		display_pos.point.y += info->offset->y;
	}
	
	int row_byte;
	unsigned char* dst_addr_start = fb_gui_addres_get(&display_pos, &row_byte);

	unsigned char* palpha = buffer;

	unsigned char sub_alpha;

	for( i = 0 ; i < display_pos.vector.height ; i++)
    {
		unsigned char* dst_addr = dst_addr_start;

		unsigned char* str_temp_addr = palpha;
		
		for(j = 0 ; j < display_pos.vector.width ; j++)
		{
			unsigned char alpha = *str_temp_addr;
			unsigned int s_argb = (alpha<<24)|(info->font_color&0xFFFFFF);
			unsigned int d_argb = *(unsigned int *)dst_addr;
			
			if(alpha == 0){   
                    
            }
			else if((alpha == 0xFF)||(((d_argb>>24)&0xFF) < ALPHA_LEVEL))
			{
				*(unsigned int*)dst_addr = s_argb;
				
            }
			else
			{ 
                sub_alpha = 255 - alpha;
				unsigned int d_data  = (( d_argb >> 24)* sub_alpha +   alpha * alpha) >> 8;
				d_data <<= 8;
				d_data |= (((d_argb >> 16) & 0xFF)* sub_alpha + ((s_argb >> 16) & 0xFF) * alpha) >> 8;
				d_data <<= 8;
				d_data |= (((d_argb >> 8 ) & 0xFF) * sub_alpha + ((s_argb >>  8) & 0xFF) * alpha) >> 8;
				d_data <<= 8;
				d_data |= (( d_argb  & 0xFF) * sub_alpha + ((s_argb) & 0xFF) * alpha) >> 8;
				*(unsigned int *)dst_addr = d_data;
            }
			str_temp_addr += 1;
			dst_addr += 4;
		}
		dst_addr_start += row_byte;
		palpha  += image_width;
    }
	
FONT_DECODEC_FINISH:

    if(buffer != NULL)
    {
        ak_mem_free(buffer);
    }
    FT_Done_Face(face);
    FT_Done_FreeType( library);
	//printf("indoor=================>>font decodec :%llu ms \n\r",os_get_ms() - start);
	return reslut;
}
