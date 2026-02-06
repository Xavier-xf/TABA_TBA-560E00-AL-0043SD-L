#ifndef VIDEO_ENCODE_H
#define VIDEO_ENCODE_H

#include "ui_api.h"
#include "ak_vi.h"

typedef struct
{

	bool (*open)(void);

	bool (*start)(void);

	bool (*stop)(void);

	bool (*read)(unsigned char**buffer,unsigned int* len,unsigned long long* pts);

	bool (*get_frame)(const struct video_input_frame* vi_frame,const vector* vector);

	bool (*close)(void);
	
}leo_mjpeg_encode_func;

extern leo_mjpeg_encode_func mjpeg_encode_api;

#endif


