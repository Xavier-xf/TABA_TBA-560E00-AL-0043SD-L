#ifndef _LEO_DEFINE_UI_H_
#define _LEO_DEFINE_UI_H_
#include "ui_api.h"
#define ALPHA_LEVEL 0X60

typedef struct 
{
	const resource* res;
	const position* pos;
	const point* offset;
	align align;
}png_decodec_info;

bool png_decodec(png_decodec_info* info);

typedef struct
{
	const char* string;
	unsigned int font_color;
	position* pos;
	point* offset;
	align align;
	int font_size;
}font_decodec_info;

bool font_decodec(font_decodec_info* info);

typedef struct
{
	const resource* resource;
	const position* src_pos;
	const point* offset;
	align align;

	unsigned char** dst;
	vector* dst_vector;

	bool extern_create_buffer;
}jpeg_decodec_info;




#endif


