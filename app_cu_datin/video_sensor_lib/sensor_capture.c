#include "sensor_video_api.h"
#include "ak_thread.h"
#include "string.h"
#include "ui_api.h"
#include "video_encodec.h"
#include "motion_detect.h"
#include "video_output.h"


extern ak_mutex_t sensor_drv_mutex;
extern bool sensor_video_check(void);

extern const vector *screen_vector_get(void);

extern const vector *sensor_vector_get(void);

static int video_block_frame_delay_count = 0;

static bool sensor_display_thread_run = false;
static ak_pthread_t sensor_display_thread_id;
static void* video_capture_task(void* arg)
{
	struct video_input_frame vi_frame;
	position pos = {{0,0}};
	video_block_frame_delay_count = 10;
	
	while(sensor_display_thread_run == true)
	{	
		ak_thread_mutex_lock(&sensor_drv_mutex);
		if(sensor_video_check() == true)
		{
			memset(&vi_frame,0,sizeof(struct video_input_frame));
			//unsigned long long start = os_get_ms();
			if(ak_vi_get_frame(VIDEO_CHN0, &vi_frame) == 0)
			{
				if((video_block_frame_delay_count)>0)
				{
					video_block_frame_delay_count--;
					
				}
				else
				{
					pos.vector.width =((const vector*)sensor_vector_get())->width;
					pos.vector.height = ((const vector*)sensor_vector_get())->height;

					mjpeg_encode_api.get_frame(&vi_frame,&pos.vector);
					motion_detect_api.get_frame(&vi_frame,&pos.vector);
					video_output_api.vi_write(&vi_frame,&pos.vector);
				}
				ak_vi_release_frame(VIDEO_CHN0, &vi_frame);
			}
			//unsigned long long end = os_get_ms();
			//printf("===============>>.%llu \n\r",end - start);
		}
		
		ak_thread_mutex_unlock(&sensor_drv_mutex);
		ak_sleep_ms(1);
	}
	ak_thread_exit();
	return NULL;
}


bool video_capture_open(void)
{
	ak_thread_mutex_lock(&sensor_drv_mutex);
	if(sensor_display_thread_run == true)
	{
		ak_thread_mutex_unlock(&sensor_drv_mutex);
		return false;
	}
	sensor_display_thread_run = true;
	ak_thread_mutex_unlock(&sensor_drv_mutex);
	ak_thread_create(&sensor_display_thread_id, video_capture_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE , -1);
	return true;
}

bool video_capture_close(void)
{
	ak_thread_mutex_lock(&sensor_drv_mutex);
	if(sensor_display_thread_run == false)
	{
		ak_thread_mutex_unlock(&sensor_drv_mutex);
		return false;
	}
	sensor_display_thread_run = false;
	ak_thread_mutex_unlock(&sensor_drv_mutex);
	ak_thread_join(sensor_display_thread_id);
	return true;
}



void video_capture_filter_frames(int frames)
{
	video_block_frame_delay_count = frames;
}





