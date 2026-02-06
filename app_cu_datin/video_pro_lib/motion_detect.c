#include "motion_detection_api.h"
#include "sensor_video_api.h"
#include "stdlib.h"
#include "ak_tde.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include "motion_detect.h"

#define MOTION_SRC_WIDTH 	320
#define MOTION_SRC_HEIGHT 	240

/*
*   取值在0-1200 
*
*	每个8x8的宏块中有一个移动就+1,在320x240的像素块
*	中总共有1200个宏块
*/
static int motion_sensivity = 0; 

static unsigned char motion_threshold = 0; //取值范围在0-255

static bool motion_detection_run = false;

static ak_pthread_t motion_detection_thread_id = 0;

static bool motion_detection_move = false;
static bool motion_detection_first = true;
static int motion_first_frame_filter = 0;

static unsigned char* motion_compare_src = NULL;
static unsigned char* motion_compare_dst = NULL;

static bool motion_yuv_data_ready = false;


static void motion_src_blit_to_dst_buffer(void)
{
	struct ak_tde_layer src,dst;
	src.format_param = GP_FORMAT_YUV420SP;
	src.width = MOTION_SRC_WIDTH;
	src.height = MOTION_SRC_HEIGHT;
	src.pos_width = src.width;
	src.pos_height = src.height;
	src.pos_top = src.pos_left = 0;
	ak_mem_dma_vaddr2paddr(motion_compare_src, (unsigned long*)&src.phyaddr);

	dst.format_param = GP_FORMAT_YUV420SP;
	dst.pos_width = dst.width = MOTION_SRC_WIDTH;
	dst.pos_height = dst.height = MOTION_SRC_HEIGHT;
	dst.pos_left = dst.pos_top = 0;
	ak_mem_dma_vaddr2paddr(motion_compare_dst, (unsigned long*)&dst.phyaddr);
	ak_tde_opt_blit(&src,&dst);
}
static bool motion_y_data_compare_is_move(void)
{
	int i,j,k;
	int piexl_diffret = 0;
	int cell_diffret = 0;
	for( i = 0 ; i < MOTION_SRC_WIDTH ; i += 8)
	{
		for(j = 0 ;  j < MOTION_SRC_HEIGHT ; j++)
		{
			for( k = i ; k < i+8 ; k++)
			{
				if(abs(motion_compare_src[MOTION_SRC_WIDTH*j + k ] - motion_compare_dst[MOTION_SRC_WIDTH*j + k ]) > motion_threshold)
				{
					piexl_diffret++;
				}
			}

			if(!(j%8))
			{
				if(piexl_diffret > 8)
				{
					cell_diffret++;
				}
				piexl_diffret = 0;
			}
		}
	}
	motion_src_blit_to_dst_buffer();
	
//	printf("cell_diffret = %d motion_sensivity = %d\n\r",cell_diffret,motion_sensivity);
	if(cell_diffret > motion_sensivity)
	{
		printf("\n\n\n===================>> motion detection move \n\n\n\n");
		return true;
	}
	return false;
}


static void* motion_detection_task(void* arg)
{
	while(motion_detection_run == true)
	{
		if((motion_yuv_data_ready == false)||(motion_detection_move == true))
		{
			ak_sleep_ms(10);
			continue;
		}
		

		if( motion_detection_first == true)
		{
			motion_src_blit_to_dst_buffer();
			motion_detection_first = false;
		}
		else if(motion_y_data_compare_is_move() == true)
		{
			motion_detection_move = true;
		}
		
		motion_yuv_data_ready = false;
	}

	ak_thread_exit();
	return NULL;
}



static void sensor_src_buffer_scale_to_motion_src(unsigned char* dst_buffer,const struct video_input_frame* frame,const vector* vector)
{
	struct ak_tde_layer src,dst;
	src.format_param = GP_FORMAT_YUV420SP;
	src.width = vector->width;
	src.height = vector->height;
	src.pos_width = src.width;
	src.pos_height = src.height;
	src.pos_top = src.pos_left = 0;
	src.phyaddr = frame->phyaddr;

	dst.format_param = GP_FORMAT_YUV420SP;
	dst.pos_width = dst.width = MOTION_SRC_WIDTH;
	dst.pos_height = dst.height = MOTION_SRC_HEIGHT;
	dst.pos_left = dst.pos_top = 0;
	ak_mem_dma_vaddr2paddr(dst_buffer, (unsigned long*)&dst.phyaddr);
	ak_tde_opt_scale(&src,&dst);
}

bool motion_detect_get_frame_func(const struct video_input_frame* frame,const vector* vector)
{
	if(motion_detection_run == false)
	{
		return false;
	}

	if(motion_detection_move == true)
	{
		return true;
	}

	
	if(motion_first_frame_filter < 10)
	{
		motion_first_frame_filter++;
		return false;
	}
	
	if(motion_yuv_data_ready == true)
	{
		return false;
	}
	sensor_src_buffer_scale_to_motion_src(motion_compare_src,frame,vector);
	motion_yuv_data_ready = true;
	return true;
}


static void motion_detection_buffer_alloc(void)
{
	if(motion_compare_src == NULL)
	{
		motion_compare_src = (unsigned char*)ak_mem_dma_alloc(MODULE_ID_MD, MOTION_SRC_WIDTH*MOTION_SRC_HEIGHT*3/2);
	}
	if(motion_compare_dst == NULL)
	{
		motion_compare_dst = (unsigned char*)ak_mem_dma_alloc(MODULE_ID_MD, MOTION_SRC_WIDTH*MOTION_SRC_HEIGHT*3/2);
	}
}
static void motion_detection_buffer_release(void)
{
	if(motion_compare_src != NULL)
	{
		ak_mem_dma_free(motion_compare_src);
		motion_compare_src = NULL;
	}
	if(motion_compare_dst != NULL)
	{
		ak_mem_dma_free(motion_compare_dst);
		motion_compare_dst = NULL;
	}
}

static void motion_detection_parameter_init(void)
{
	motion_detection_move = false;
	motion_detection_first = true;
	motion_first_frame_filter = 0;
	motion_yuv_data_ready = false;
}

static bool motion_detection_func_open(int sensivity,unsigned char threshold)
{
	if(motion_detection_run == true)
	{
		return false;
	}
	motion_detection_buffer_alloc();
	motion_detection_parameter_init();
	
	motion_threshold = threshold;
	motion_sensivity = sensivity;
	motion_detection_run = true;
	ak_thread_create(&motion_detection_thread_id,motion_detection_task,NULL,ANYKA_THREAD_NORMAL_STACK_SIZE,-1);
	return true;
}



static bool motion_detect_func_close(void)
{
	if(motion_detection_run == false)
	{
		return false;
	}
	

	motion_detection_run = false;
	ak_thread_join(motion_detection_thread_id);
	motion_detection_buffer_release();
	return true;
}


static bool motion_detect_func_move(void)
{
	return motion_detection_move;
}

leo_motion_detect_func motion_detect_api = {

	.open = motion_detection_func_open,
		
	.get_frame = motion_detect_get_frame_func,

	.check_motion = motion_detect_func_move,

	.close = motion_detect_func_close,
		
};


bool motion_detection_open(int sensivity,unsigned char threshold)
{
	return motion_detect_api.open(sensivity,threshold);
}

bool motion_detection_check(void)
{
	return motion_detect_api.check_motion();
}

bool motion_detection_close(void)
{
	return motion_detect_api.close();
}


