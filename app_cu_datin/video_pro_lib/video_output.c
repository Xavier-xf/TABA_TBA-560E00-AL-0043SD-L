#include "ak_mem.h"
#include "video_output.h"
#include <stdbool.h>
#include "ak_thread.h"
#include "ui_api.h"
#include <string.h>
#include "ak_vi.h"
#include "ak_tde.h"

const vector* screen_vector_get(void);
bool fb_video_data_pos_adj(struct ak_tde_layer* src,const position* pos);

static ak_mutex_t video_output_mutex_t;


static bool video_output_init(void)
{
	ak_thread_mutex_init(&video_output_mutex_t, NULL);
	return true;
}


static bool video_decode_write(struct ak_vdec_frame* frame)
{
	ak_thread_mutex_lock(&video_output_mutex_t);
	struct ak_tde_layer src;
	src.format_param = GP_FORMAT_YUV420SP;
	src.width = frame->frame_obj.data.pitch_width;
	src.height = frame->frame_obj.data.pitch_height;
	src.pos_left = src.pos_top = 0;
	src.pos_width = frame->width;
	src.pos_height = frame->height;
	ak_mem_dma_vaddr2paddr(frame->frame_obj.data.data, (unsigned long*)&src.phyaddr);
	position pos = {{0,0},{0,0}};
	pos.vector = *(screen_vector_get());
	fb_video_data_pos_adj(&src,&pos);
	ak_thread_mutex_unlock(&video_output_mutex_t);
   	return true;
}

bool video_vi_write(const struct           video_input_frame* frame,const vector* vetctor)
{
	ak_thread_mutex_lock(&video_output_mutex_t);
	struct ak_tde_layer src;
	src.format_param = GP_FORMAT_YUV420P;
	src.width = vetctor->width;
	src.height = vetctor->height;
	src.pos_left = src.pos_top = 0;
	src.pos_width = vetctor->width;
	src.pos_height = vetctor->height;
	src.phyaddr = frame->phyaddr;

	position pos = {{0,0},{0,0}};
	pos.vector = *(screen_vector_get());
	fb_video_data_pos_adj(&src,&pos);
	ak_thread_mutex_unlock(&video_output_mutex_t);
   	return true;
}




leo_video_output_func video_output_api = 
{

	.init = video_output_init,

	.vdec_write = video_decode_write,

	.vi_write = video_vi_write
};


