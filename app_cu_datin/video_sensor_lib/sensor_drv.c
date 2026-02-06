#include "sensor_video_api.h"
#include "ak_thread.h"
#include "tp9950_drv.h"
#include "ak_common.h"
#include "ak_vi.h"
#include "string.h"
#include "sensor_video_api.h"
#include "ui_api.h"
#include "video_encodec.h"
#include "audio_input.h"
bool tp9950_init(void(*reset_callback_func)(void),
						int(*detection_before_callback)(void),
						bool(*write_conf_after_callback)(int*,int*,int*),
						unsigned char (* bright_map)[6],
						unsigned char (* cont_map)[6],
						unsigned char (* color_map)[6],
						int map_size);
extern unsigned char tp9950_format_get(void);
extern unsigned char tp9950_state_get(void);

ak_mutex_t sensor_drv_mutex;
static vector sensor_vector;
static int sensor_frame_fps = 0;

static bool is_sensor_task_run  = 0; 

const vector* sensor_vector_get(void)
{
	return &sensor_vector;
}
unsigned int sensor_frame_rate_get(void)
{
	return sensor_frame_fps;
}	



static bool sensor_info_set(unsigned char mode)
{
	if(mode == TP2802_PAL)
	{
		sensor_frame_fps = 25;
		sensor_vector = (vector){512,288};
		return true;
	}
	if(mode == TP2802_NTSC)
	{
		sensor_frame_fps = 30;
		sensor_vector = (vector){432,240};
		return true;
	}
	if((mode == TP2802_720P25V2) ||(mode == TP2802_720P30V2))
	{
		sensor_frame_fps = TP2802_720P25V2?25:30;
		sensor_vector = (vector){640,360};
		return true;
	}
	if((mode == TP2802_1080P25)||(mode == TP2802_1080P30))
	{
		sensor_frame_fps = TP2802_1080P25?25:30;
		sensor_vector = (vector){640,360};
		return true;
	}

	return false;
}



static bool video_sensor_core_open(unsigned char mode)
{
	if(sensor_info_set(mode)== false)
	{
		return false;
	}
	
	if(ak_vi_open(VIDEO_DEV0)) 
	{
		printf("vi device open failed\n");	
		return false;
	}
	/*******************************************
	将向 "tp9950_g_volatile_ctrl()" 获取有效ID
	*******************************************/
	//video_sensor_pixel_init(mode);//
	//ak_vi_load_sensor_cfg(VIDEO_DEV0,video_sensor_pixel_init(mode));
	
	VI_DEV_ATTR    dev_attr;
	memset(&dev_attr, 0, sizeof(VI_DEV_ATTR));
	dev_attr.dev_id = VIDEO_DEV0;
	dev_attr.crop.left = 0;
	dev_attr.crop.top = 0;
	dev_attr.crop.width = 1280;//960;
	dev_attr.crop.height = 1080;

	dev_attr.max_width = 1280;//960;
	dev_attr.max_height = 1080;
	dev_attr.frame_rate = sensor_frame_fps;
	dev_attr.sub_max_width = 18;//640;
	dev_attr.sub_max_height = 18;//1080;//360;

	RECTANGLE_S res;
    if (ak_vi_get_sensor_resolution(VIDEO_DEV0, &res))
	{
        printf("Can't get dev[%d]resolution\n", VIDEO_DEV0);
        ak_vi_close(VIDEO_DEV0);
		return false;
    } 
	else
	{
       dev_attr.crop.width = res.width;
       dev_attr.crop.height = res.height;
    }
/*
	if(sensor_pixel_check(mode,&sensor_vector) == false)
	{
		ak_vi_close(VIDEO_DEV0);
		printf("set pixel fail %d\n", VIDEO_DEV0);
		return false;
	}
*/

	/*********************************************************
	将向 “tp9950_sensor_get_resolution_func”,获取有效得分辨率
	**********************************************************/
	if (ak_vi_set_dev_attr(VIDEO_DEV0, &dev_attr))
	{
		printf("vi device set device attribute failed!\n");
		ak_vi_close(VIDEO_DEV0);
		return false;
	}

	

	/***********************
	用于显示
	***********************/
	VI_CHN_ATTR chn_attr;
	memset(&chn_attr, 0, sizeof(VI_CHN_ATTR));
	chn_attr.chn_id = VIDEO_CHN0;
	chn_attr.res.width = sensor_vector.width;
	chn_attr.res.height = sensor_vector.height;
	chn_attr.frame_depth = 4;
	chn_attr.frame_rate = sensor_frame_fps;
	if (ak_vi_set_chn_attr(VIDEO_CHN0, &chn_attr))
	{
		printf( "vi device set channel [%d] attribute failed!\n", VIDEO_CHN0);
		ak_vi_close(VIDEO_DEV0);
		return false;
	}

	
	/***********************************************************************
	setp1:将向 tp9950_sensor_get_parameter_func,获取接口为DVP
	setp2:将向 tp9950_sensor_get_parameter_func,未知
	setp3:将向 tp9950_sensor_get_parameter_func,未知
	setp4:将向 tp9950_sensor_set_power_on_func 复位开启tp9950的电源
	setp5:将向 tp9950_sensor_init_func 设置一些参数(帧数，制式类型(SENSOR_TYPE)
		  制式的分辨率（SENSOR_OUTPUT_WIDTH,SENSOR_OUTPUT_HEIGHT）原厂需要刷写屏参)
		  并且设置 set_timing();
	**************************************************************************/
	if (ak_vi_enable_dev(VIDEO_DEV0))
	{
		printf("vi device enable device  failed!\n");
		ak_vi_close(VIDEO_DEV0);
		return false;
	}

	if(ak_vi_enable_chn(VIDEO_CHN0))
	{
		printf("vi channel[%d] enable failed!\n",VIDEO_CHN0);
		ak_vi_close(VIDEO_DEV0);
		return false;
	}

	return 0;
}

static void video_sensor_core_close(void)
{
	ak_vi_disable_chn(VIDEO_CHN0);
	ak_vi_disable_dev(VIDEO_DEV0); 
	ak_vi_close(VIDEO_DEV0);
}




bool video_sensor_open(unsigned char mode)
{
	ak_thread_mutex_lock(&sensor_drv_mutex);
	if( is_sensor_task_run)
	{
		printf("task working \n\r");
		ak_thread_mutex_unlock(&sensor_drv_mutex);
		return false;
	}
	
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");

	if(video_sensor_core_open(mode))
	{
		printf("open sensor core fail \n\r");
		ak_thread_mutex_unlock(&sensor_drv_mutex);
		return false;
	}
	
	mjpeg_encode_api.open();

	audio_input_api.open(AK_AUDIO_SAMPLE_RATE_8000,AUDIO_CHANNEL_MONO);
	
	is_sensor_task_run = true;	
	ak_thread_mutex_unlock(&sensor_drv_mutex);
	return true;
}

bool video_sensor_close(void)
{
	ak_thread_mutex_lock(&sensor_drv_mutex);
	if(is_sensor_task_run == 0)
	{
		printf("task not working \n\r");
		ak_thread_mutex_unlock(&sensor_drv_mutex);
		return false;
	}
	
	mjpeg_encode_api.close();
	audio_input_api.close();

	video_sensor_core_close();
	
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
	
	is_sensor_task_run = false;
	ak_thread_mutex_unlock(&sensor_drv_mutex);
	return true;
}


bool sensor_video_init(void(*reset_callback)(void),
							   int(*det_callback)(void),
							   bool(*write_callback)(int*bri,int*cont,int*color),
							   unsigned char (* bright_map)[6],
							   unsigned char (* cont_map)[6],
							   unsigned char (* color_map)[6],
							   int map_size)
{
	ak_thread_mutex_init(&sensor_drv_mutex, NULL);
	tp9950_init(reset_callback,det_callback,write_callback,bright_map,cont_map,color_map,map_size);
	return true;
}




bool sensor_video_check(void)
{
	unsigned char rx_state = tp9950_state_get();
	if(rx_state != TP9950_STATE_DET)
	{
		return false;
	}

	unsigned char format = tp9950_format_get();
	if(format == INVALID_FORMAT)
	{
		return false;
	}

/*
	if(sensor_pixel_check(format) == false)
	{
		return false;
	}
*/
	if(is_sensor_task_run == false)
	{
		return false;
	}
	return true;
}


bool sensor_video_open(void)
{
	extern void tp9950_open(void);
	tp9950_open();
	return true;
}

bool sensor_video_close(void)
{
	extern void tp9950_close(void);
	tp9950_close();
	return true;
}

