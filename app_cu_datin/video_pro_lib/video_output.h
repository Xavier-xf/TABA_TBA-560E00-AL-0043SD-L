#ifndef VIDEO_OUTPUT_H
#define VIDEO_OUTPUT_H
#include <stdbool.h>
#include "ak_common_graphics.h"
#include "ak_common.h"
#include "ak_vdec.h"
#include "ui_api.h"
#include "ak_vi.h"

typedef struct
{
	bool(*init)(void);

	bool (*vdec_write)(struct ak_vdec_frame* frame);

	bool (*vi_write)(const struct video_input_frame* frame,const vector* vetctor);
	
}leo_video_output_func;


extern leo_video_output_func video_output_api;

#endif

