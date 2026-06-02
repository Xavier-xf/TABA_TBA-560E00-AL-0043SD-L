#include "ui_api.h"
#include "math.h"
#include "memory.h"
#include "ak_mem.h"
#include <fcntl.h>

extern bool png_decodec_raw(const resource* res,unsigned char**dst,vector* image_info);
unsigned char* fb_gui_addres_get(const position* pos,int* row_byte);
void argb8888_to_argb8888(unsigned char* src,const vector* src_vector,const position* disp_pos,align align,point* offset,unsigned char* dst,int dst_row_byte);

static point dot_offset = {0,0};

static unsigned char* clock_bg_buffer = NULL;
static vector clock_bg_vector;

static int clock_hour_center_y;
static unsigned char* clock_hour_buffer = NULL;
static vector clock_hour_vector;

static int clock_min_center_y;
static unsigned char* clock_min_buffer = NULL;
static vector clock_min_vector;

static int clock_sec_center_y;
static unsigned char* clock_sec_buffer = NULL;
static vector clock_sec_vector;

static unsigned char* clock_dot_buffer = NULL;
static vector clock_dot_vector;

static unsigned char* analog_clock_dst_buffer = NULL;
static position analog_clock_pos;

static bool is_analog_clock_init = false;

void argb8888_rotate(unsigned int *dst,int dstWidth, int dstHeight,int dstPitch,
							unsigned int *src,int srcWidth,int srcHeight,int srcPitch,
							int centerX,int centerY,float angle,float scale)
{

    float        dst0X_o, dst0Y_o, dst1X_o, dst1Y_o, dst2X_o, dst2Y_o, width_cosT, width_sinT, height_cosT, height_sinT;
    float        sinT, cosT, dst_half_width_scaled, dst_half_height_scaled;
    int          leftDist, rightDist, upDist, downDist, h, v;
    int          row_dX, row_dY, col_dX, col_dY, dstColX, dstColY, dstX, dstY, leftX, upY;
    unsigned int pixel, *dstCol, *dstRow, *pixPtr, newA, newR, newG, newB;
    unsigned int ulPixel, urPixel, dlPixel, drPixel, ulValue, urValue, dlValue, drValue;

    // prepare dst0X, dst0Y, row_dx, row_dy, col_dx, col_dy
    ulPixel                = urPixel = dlPixel = drPixel = 0;
    angle                  = -angle;
    sinT                   = sinf(angle * (3.14159265f / 180.0f));
    cosT                   = cosf(angle * (3.14159265f / 180.0f));
    dst_half_width_scaled  = dstWidth / 2 / scale;
    dst_half_height_scaled = dstHeight / 2 / scale;
    width_cosT             = dst_half_width_scaled * cosT;
    width_sinT             = dst_half_width_scaled * sinT;
    height_cosT            = dst_half_height_scaled * cosT;
    height_sinT            = dst_half_height_scaled * sinT;
    dst0X_o                = -width_cosT - -height_sinT;
    dst0Y_o                = -width_sinT + -height_cosT;
    dst1X_o                = width_cosT - -height_sinT;
    dst1Y_o                = width_sinT + -height_cosT;
    dst2X_o                = -width_cosT - height_sinT;
    dst2Y_o                = -width_sinT + height_cosT;
    row_dX                 = (int)(((dst1X_o - dst0X_o) / dstWidth) * 65536);
    row_dY                 = (int)(((dst1Y_o - dst0Y_o) / dstWidth) * 65536);
    col_dX                 = (int)(((dst2X_o - dst0X_o) / dstHeight) * 65536);
    col_dY                 = (int)(((dst2Y_o - dst0Y_o) / dstHeight) * 65536);

    // start to generate rotated image
    dstCol                 = dst;
    dstColX                = (int)(dst0X_o * 65536) + (centerX << 16);
    dstColY                = (int)(dst0Y_o * 65536) + (centerY << 16);

    for (v = 0; v < dstHeight; v++)
	{
        dstRow = dstCol;
        dstX   = dstColX;
        dstY   = dstColY;
        for (h = 0; h < dstWidth; h++){
            if (dstX < 0 || dstX >= ((srcWidth - 1) << 16) || dstY < 0 || dstY >= ((srcHeight - 1) << 16)){
                // out of source boundary
                pixel = 0;
               // newA = newR = newG = newB = 0;
            }
			else
			{
                leftX     = dstX >> 16;
                upY       = dstY >> 16;

                pixPtr    = src + upY * srcPitch + leftX;
                ulPixel   = *(pixPtr);
                urPixel   = *(pixPtr + 1);
                dlPixel   = *(pixPtr + srcPitch);
                drPixel   = *(pixPtr + srcPitch + 1);

                leftDist  = (dstX & 0x0FFFF) >> 8;
                rightDist = 256 - leftDist;
                upDist    = (dstY & 0x0FFFF) >> 8;
                downDist  = 256 - upDist;

                ulValue   = ulPixel >> 24;
                urValue   = urPixel >> 24;
                dlValue   = dlPixel >> 24;
                drValue   = drPixel >> 24;
                newA      =  ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = (ulPixel >> 16) & 0xFF;
                urValue   = (urPixel >> 16) & 0xFF;
                dlValue   = (dlPixel >> 16) & 0xFF;
                drValue   = (drPixel >> 16) & 0xFF;
                newR      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = (ulPixel >> 8) & 0xFF;
                urValue   = (urPixel >> 8) & 0xFF;
                dlValue   = (dlPixel >> 8) & 0xFF;
                drValue   = (drPixel >> 8) & 0xFF;
                newG      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                ulValue   = ulPixel & 0xFF;
                urValue   = urPixel & 0xFF;
                dlValue   = dlPixel & 0xFF;
                drValue   = drPixel & 0xFF;
                newB      = ((ulValue * downDist + dlValue * upDist) * rightDist + (urValue * downDist + drValue * upDist) * leftDist + 32768) >> 16;

                pixel     = (newA << 24) | (newR << 16) | (newG << 8) | newB;
            }

#if 0			
			if (pixel  == 0)
			{
				
			   
			}
			else if (newA == 0xFF)
			{
				*(dstRow)  = pixel;//(newA << 24) | (newR << 16) | (newG << 8) | newB;
			}
			else
			{
				unsigned int d_argb = pixel;//*(unsigned int *)dstRow;
				unsigned int rev_s_a = 255 - newA;
				unsigned int d_data  = (( d_argb >> 24)* rev_s_a +	 newA * newA) >> 8;
				d_data <<= 8;
				d_data |= (((d_argb >> 16) & 0xFF) * rev_s_a + newR * newA) >> 8;
				d_data <<= 8;
				d_data |= (((d_argb >> 8 ) & 0xFF) * rev_s_a + newG * newA) >> 8;
				d_data <<= 8;
				d_data |= (( d_argb  & 0xFF) * rev_s_a + newB * newA) >> 8;
				*(unsigned int *)dstRow = d_data;
			}
#endif			
		//	if(pixel)
			{
	            *(dstRow) = pixel;
			}
            dstRow++;
            dstX     += row_dX;
            dstY     += row_dY;
        }
        dstCol  += dstPitch;
        dstColX += col_dX;
        dstColY += col_dY;
    }
}


void set_clock_dot_offset(point offset_xy){
    dot_offset.x = offset_xy.x;
    dot_offset.y = offset_xy.y;
}




static void analog_clock_bg_init(const resource* res)
{
	if(clock_bg_buffer != NULL)
	{
		ak_mem_free(clock_bg_buffer);
		clock_bg_buffer = NULL;
	}

	if((res->id != 0)&&(res->size != 0))
	{
		png_decodec_raw(res, &clock_bg_buffer, &clock_bg_vector);
	}
}

static void analog_clock_hour_init(const resource* res,int center_y)
{
	if(clock_hour_buffer != NULL)
	{
		ak_mem_free(clock_hour_buffer);
		clock_hour_buffer = NULL;
	}

	if((res->id != 0)&&(res->size != 0))
	{
		png_decodec_raw(res, &clock_hour_buffer, &clock_hour_vector);
	}
	clock_hour_center_y = center_y;
}

static void analog_clock_min_init(const resource* res,int center_y)
{
	if(clock_min_buffer != NULL)
	{
		ak_mem_free(clock_min_buffer);
		clock_min_buffer = NULL;
	}

	if((res->id != 0)&&(res->size != 0))
	{
		png_decodec_raw(res, &clock_min_buffer, &clock_min_vector);
	}
	clock_min_center_y = center_y;
}

static void analog_clock_sec_init(const resource* res,int center_y)
{
	if(clock_sec_buffer != NULL)
	{
		ak_mem_free(clock_sec_buffer);
		clock_sec_buffer = NULL;
	}

	if((res->id != 0)&&(res->size != 0))
	{
		png_decodec_raw(res, &clock_sec_buffer, &clock_sec_vector);
	}
	clock_sec_center_y = center_y;
}

static void analog_clocK_dot_init(const resource* res)
{
	if(clock_dot_buffer != NULL)
	{
		ak_mem_free(clock_dot_buffer);
		clock_dot_buffer = NULL;
	}

	if((res->id != 0)&&(res->size != 0))
	{
		png_decodec_raw(res, &clock_dot_buffer, &clock_dot_vector);
	}
}



static void analog_clock_dst_init(const position* pos)
{
	analog_clock_pos.point = pos->point;
	analog_clock_pos.vector = pos->vector;
	
	if(analog_clock_dst_buffer != NULL)
	{
		ak_mem_free(analog_clock_dst_buffer);
		analog_clock_dst_buffer = NULL;
	}
	analog_clock_dst_buffer = (unsigned char*)ak_mem_alloc(MODULE_ID_APP, analog_clock_pos.vector.width*analog_clock_pos.vector.height*4);
	if(analog_clock_dst_buffer != NULL)
	{
		memset(analog_clock_dst_buffer, 0, analog_clock_pos.vector.width*analog_clock_pos.vector.height*4);
	}
}

bool analog_clock_init(const analog_clock* clock_info)
{
	if(is_analog_clock_init == true)
	{
		return false;
	}
	if(clock_info == NULL)
	{
		return false;
	}
	point offset = {0,0};
	set_clock_dot_offset(offset);
	analog_clock_bg_init(&clock_info->bg_res);
	analog_clock_hour_init(&clock_info->hour_res,clock_info->hour_center_y);
	analog_clock_min_init(&clock_info->min_res,clock_info->min_center_y);
	analog_clock_sec_init(&clock_info->sec_res,clock_info->sec_center_y);
	analog_clocK_dot_init(&clock_info->dot_res);
	analog_clock_dst_init(&clock_info->pos);
	is_analog_clock_init = true;
	return true;
}



static bool analog_clock_bg_load(void)
{
	if(clock_bg_buffer == NULL)
	{
		return false;
	}
	int dst_row_byte = 0;
	unsigned char* dst = fb_gui_addres_get(&analog_clock_pos,&dst_row_byte);
	argb8888_to_argb8888(clock_bg_buffer,&clock_bg_vector,&analog_clock_pos,CENTER_MIDDLE,NULL,dst,dst_row_byte);
	return true;
}


static bool analog_clock_hour_load(const struct ak_date* date)
{
	if(clock_hour_buffer == NULL)
	{
		return false;
	}

	int center_x = clock_hour_vector.width/2;
	int center_y = clock_hour_center_y;
	float hour;
	if (date->hour > 12)
        hour = (float)(date->hour - 12);
    else
        hour = (float)date->hour;

    hour += date->minute / 60.0f;

    float angle = 360.0f / 12.0f * hour;
	argb8888_rotate((unsigned int*)analog_clock_dst_buffer,analog_clock_pos.vector.width,analog_clock_pos.vector.height,analog_clock_pos.vector.width,
					(unsigned int*)clock_hour_buffer,clock_hour_vector.width,clock_hour_vector.height,clock_hour_vector.width,
					center_x,center_y,angle,1);
	
	return true;
}

static bool analog_clock_min_load(const struct ak_date* date)
{
	if(clock_min_buffer == NULL)
	{
		return false;
	}

	int center_x = clock_min_vector.width/2;
	int center_y = clock_min_center_y;
	
	float angle = 360.0f / 60.0f * date->minute;
	argb8888_rotate((unsigned int*)analog_clock_dst_buffer,analog_clock_pos.vector.width,analog_clock_pos.vector.height,analog_clock_pos.vector.width,
					(unsigned int*)clock_min_buffer,clock_min_vector.width,clock_min_vector.height,clock_min_vector.width,
					center_x,center_y,angle,1);

	return true;

}


static bool analog_clock_sec_load(const struct ak_date* date)
{
	if(clock_sec_buffer == NULL)
	{
		return false;
	}

	int center_x = clock_sec_vector.width/2;
	int center_y = clock_sec_center_y;
	
	float angle = 360.0f / 60.0f * date->second;
	argb8888_rotate((unsigned int*)analog_clock_dst_buffer,analog_clock_pos.vector.width,analog_clock_pos.vector.height,analog_clock_pos.vector.width,
					(unsigned int*)clock_sec_buffer,clock_sec_vector.width,clock_sec_vector.height,clock_sec_vector.width,
					center_x,center_y,angle,1);


	return true;
}

static bool analog_clock_dot_load(void)
{
	if(clock_dot_buffer == NULL)
	{
		return false;
	}
	
	int dst_row_byte = 0;
	unsigned char* dst = fb_gui_addres_get(&analog_clock_pos,&dst_row_byte);
	argb8888_to_argb8888(clock_dot_buffer,&clock_dot_vector,&analog_clock_pos,CENTER_MIDDLE,&dot_offset,dst,dst_row_byte);
	return true;
}
static void analog_clock_to_layer(void)
{
	int dst_row_byte = 0;
	unsigned char* dst = fb_gui_addres_get(&analog_clock_pos,&dst_row_byte);
	argb8888_to_argb8888(analog_clock_dst_buffer,&analog_clock_pos.vector,&analog_clock_pos,CENTER_MIDDLE,NULL,dst,dst_row_byte);
}

bool analog_clock_update(void)
{
	if(is_analog_clock_init == false)
	{
		return false;
	}
	struct ak_date date;
	ak_get_localdate(&date);

	analog_clock_bg_load();
	
	analog_clock_hour_load(&date);	
	analog_clock_to_layer();
	
	analog_clock_min_load(&date);	
	analog_clock_to_layer();

	analog_clock_sec_load(&date);	
	analog_clock_to_layer();

	analog_clock_dot_load();
	return true;
}


bool analog_clock_deinit(void)
{
	if(is_analog_clock_init == false)
	{
		return false;
	}
	if(clock_bg_buffer != NULL)
	{
		ak_mem_free(clock_bg_buffer);
		clock_bg_buffer = NULL;
	}

	if(clock_hour_buffer != NULL)
	{
		ak_mem_free(clock_hour_buffer);
		clock_hour_buffer = NULL;
	}

	if(clock_min_buffer != NULL)
	{
		ak_mem_free(clock_min_buffer);
		clock_min_buffer = NULL;
	}
	
	if(clock_sec_buffer != NULL)
	{
		ak_mem_free(clock_sec_buffer);
		clock_sec_buffer = NULL;
	}

	if(clock_dot_buffer !=NULL)
	{
		ak_mem_free(clock_dot_buffer);
		clock_dot_buffer = NULL;
	}

	if(analog_clock_dst_buffer != NULL)
	{
		ak_mem_free(analog_clock_dst_buffer);
		analog_clock_dst_buffer = NULL;
	}
	
	is_analog_clock_init = false;
	return true;
}

