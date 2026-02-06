#include "record_video_api.h"
#include "ak_common.h"
#include "ak_venc.h"
#include "sensor_video_api.h"
#include "ak_thread.h"
#include "string.h"
#include "queue.h"
#include "os_sys_api.h"
#include "ui_api.h"
#include "video_encodec.h"

extern const vector* sensor_vector_get(void);
extern unsigned int sensor_frame_rate_get(void);

extern bool venc_queue_pop(unsigned  char**pdata);
extern bool venc_queue_push(const struct video_input_frame* frame,const vector* vetctor);
extern void venc_queue_init();

extern bool venc_queue_deinit(void);


static bool mjpeg_encode_run = false;

static int mjpeg_encode_handle_id = -1;

static ak_pthread_t mjpeg_encode_thread_id = 0;

static bool mjpeg_encode_ready = false;

static bool mjpeg_encodec_init(void)
{
	struct venc_param ve_param;
	ve_param.width  = ((const vector*)sensor_vector_get())->width;
	ve_param.height = ((const vector*)sensor_vector_get())->height;
	ve_param.fps	 = sensor_frame_rate_get();				//fps set
	ve_param.goplen = 50;				 //gop set
	ve_param.target_kbps = 800;//800;		 //k bps
	ve_param.max_kbps	  = 1024;//1024;		 //max kbps
	ve_param.br_mode	  = BR_MODE_VBR;//BR_MODE_VBR;//BR_MODE_CBR; //br mode
	ve_param.minqp 	  = 25; 		 //qp set
	ve_param.maxqp 	  = 50; 		 //qp max value
	ve_param.initqp	   = (ve_param.minqp + ve_param.maxqp)/2;	 //qp value
	ve_param.jpeg_qlevel = JPEG_QLEVEL_DEFAULT;	 //jpeg qlevel
	ve_param.chroma_mode = CHROMA_4_2_0;			 //chroma mode
	ve_param.max_picture_size = 0; 				 //0 means default
	ve_param.enc_level 	   = 50;// 50;				 //enc level
	ve_param.smart_mode	   = 0; 				 //smart mode set
	ve_param.smart_goplen	   = 100;				 //smart mode value
	ve_param.smart_quality    = 50;				 //quality
	ve_param.smart_static_value = 0;				 //value
	ve_param.enc_out_type = MJPEG_ENC_TYPE; 		  //enc type
	ve_param.profile = PROFILE_JPEG;
	
	bool reslut = ak_venc_open(&ve_param, &mjpeg_encode_handle_id)?false:true;  
	return reslut;
}

static void* mjpeg_encode_task(void* arg)
{
	mjpeg_encodec_init();
	
	venc_queue_init();

	unsigned char* pdata = NULL;
	int size = ((const vector*)sensor_vector_get())->width*((const vector*)sensor_vector_get())->height*3/2;
	
	struct video_stream stream;
	while(mjpeg_encode_run == true)
	{
		if((sensor_video_check() == false)||(mjpeg_encode_ready == false))
		{
			ak_sleep_ms(10);
			continue;
		}
		
		if((venc_queue_pop(&pdata) == true)&&(pdata != NULL))
		{
			memset(&stream,0,sizeof(struct video_stream));
			if(ak_venc_encode_frame(mjpeg_encode_handle_id, pdata, size, NULL,&stream) == 0)
			{
				queue_data_push(QUEUE_CHANNEL_VIDEO_2, stream.data, stream.len, os_get_us(), true);
				ak_venc_release_stream(mjpeg_encode_handle_id,&stream);
			}
		}
	}
	venc_queue_deinit();
	
	ak_venc_close(mjpeg_encode_handle_id);
	mjpeg_encode_handle_id = -1;
	ak_thread_exit();
	return NULL;
}


static bool mjpeg_encode_open(void)
{
	if(mjpeg_encode_run == true)
	{
		return false;
	}

	//if(sensor_video_check() == false)
	//{
	//	return false;
	//}
	
	queue_data_release(QUEUE_CHANNEL_VIDEO_2);

	mjpeg_encode_ready = false;
	mjpeg_encode_run = true;
	ak_thread_create(&mjpeg_encode_thread_id, mjpeg_encode_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	return true;
}

static bool mjpeg_encode_read(unsigned char**buffer,unsigned int* len,unsigned long long* pts)
{
	*buffer = NULL;
	if((mjpeg_encode_run == false)||(mjpeg_encode_handle_id == -1))
	{
		return false;
	}

	if(mjpeg_encode_ready == false)
	{
		return false;
	}

	queue_data* node = queue_data_pop(QUEUE_CHANNEL_VIDEO_2);
	if((node == NULL)||(node->addres == NULL))
	{
		return false;
	}

	*buffer = node->addres;
	*len = node->size;
	*pts = node->ts;

	return true;
}


static bool mjpeg_encode_frame_get(const struct video_input_frame* vi_frame,const vector* vector)
{
	if((mjpeg_encode_run == false)||(mjpeg_encode_handle_id == -1))
	{
		return false;
	}	

	if(mjpeg_encode_ready == false)
	{
		return false;
	}
	
	venc_queue_push(vi_frame,vector);	
	return true;
}


static bool mjpeg_encode_close(void)
{
	if(mjpeg_encode_run == false)
	{
		return false;
	}
	
	mjpeg_encode_run = false;
	ak_thread_join(mjpeg_encode_thread_id);
	mjpeg_encode_thread_id = 0;
	mjpeg_encode_ready = false;
	queue_data_release(QUEUE_CHANNEL_VIDEO_2);
	return true;
}


static bool mjpeg_encode_start(void)
{
	if((mjpeg_encode_run == false)||(mjpeg_encode_ready == true))
	{
		return false;
	}

	queue_data_release(QUEUE_CHANNEL_VIDEO_2);
	mjpeg_encode_ready = true;
	return true;
}

static bool mjpeg_encode_stop(void)
{
	if((mjpeg_encode_run == false)||(mjpeg_encode_ready == false))
	{
		return false;
	}
	mjpeg_encode_ready = false;
	return true;
}

leo_mjpeg_encode_func mjpeg_encode_api = 
{
	.open = mjpeg_encode_open,

	.read = mjpeg_encode_read,

	.start = mjpeg_encode_start,

	.stop = mjpeg_encode_stop,
	
	.get_frame = mjpeg_encode_frame_get,

	.close = mjpeg_encode_close

};


