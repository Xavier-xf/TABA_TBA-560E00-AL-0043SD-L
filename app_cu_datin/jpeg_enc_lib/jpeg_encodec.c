#include "jpeg_encodec_api.h"
#include "ak_mem.h"
#include "ak_thread.h"
#include "ak_vdec.h"
#include "ak_vo.h"
#include "ak_tde.h"
#include "ak_venc.h"
#include "string.h"
#include "memory.h"
#include "ak_thread.h"
#include "string.h"
#include "sensor_video_api.h"
#include "ui_api.h"
#include "video_encodec.h"

static char jpeg_file_path[128] = {0};

static bool jpeg_encodec_task_run = false;

static void(*jpeg_encodec_finish_callback)(void) = NULL;

static void jpeg_data_write(unsigned char* jpeg_data,unsigned int jpeg_len)
{
	FILE* fp = fopen(jpeg_file_path, "wb");
    fwrite(jpeg_data,jpeg_len , 1, fp);
    fclose(fp);
	
    system("sync");
	printf("\n\njpeg encodec:%s success! \n\n\n\r",jpeg_file_path);

}


static bool jpeg_video_write(void)
{
	unsigned char * jpeg_data = NULL;
	unsigned int jpeg_len = 0;
	unsigned long long pts;

	mjpeg_encode_api.start();

	int titck_count = 100;
	while(titck_count--)
	{
		if(mjpeg_encode_api.read(&jpeg_data,&jpeg_len,&pts) == true)
		{
			jpeg_data_write(jpeg_data,jpeg_len);
			break;
		}
		ak_sleep_ms(1);
	}

	mjpeg_encode_api.stop();
	
	return true;
}


static void* jpeg_encodec_task(void* arg)
{
	jpeg_video_write();
	ak_sleep_ms(300);
	if(jpeg_encodec_finish_callback != NULL)
	{
		jpeg_encodec_finish_callback();
		jpeg_encodec_finish_callback = NULL;
	}

	jpeg_encodec_task_run = false;
	ak_thread_exit();
	return NULL;
}



bool jpeg_encodec_start(const char* file_path,void(*finish_callback)(void)){

	if(check_jpeg_encodec_run() == true)
	{
		return false;
	}

	if(sensor_video_check() == false)
	{
		return false;
	}

	memset(jpeg_file_path,0,sizeof(jpeg_file_path));
	strcpy(jpeg_file_path,file_path);
	jpeg_encodec_finish_callback = finish_callback;
	
	ak_pthread_t thread_id;
	jpeg_encodec_task_run = true;
	ak_thread_create(&thread_id, jpeg_encodec_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(thread_id);
	return true;
}



bool check_jpeg_encodec_run(void){

	if(jpeg_encodec_task_run == false){

		return false;
	}
	return true;
}

