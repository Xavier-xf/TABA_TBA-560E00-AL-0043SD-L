#include <stdio.h>
#include "ak_thread.h"
#include <stdbool.h>
#include "ui_api.h"
#include "ak_common_audio.h"
#include "queue.h"
#include "video_record.h"
#include "string.h"
#include "audio_input.h"
#include "ak_mem.h"
#include "os_sys_api.h"
#include "video_encodec.h"
#include "avilib.h"
#include "file_api.h"
#include <unistd.h>

const vector* sensor_vector_get(void);
unsigned int sensor_frame_rate_get(void);


static ak_pthread_t record_mjpeg_thread_id = 0;

static bool record_mjpeg_run = false;

static bool record_mjpeg_thread_run = false;

static bool record_have_audio = false;

static char record_mjpeg_path[128] = {0};

static int record_mjpeg_duration = 0;

static void* avi_head_write(avi_t * avi_head,double fps)
{
	int width = sensor_vector_get()->width;
	int height = sensor_vector_get()->height;
	//int fps = sensor_frame_rate_get();

	AVI_set_video(avi_head,  width , height , fps, "MJPG");
	AVI_set_audio(avi_head, 1, 8000, 16, 1,128);
	avi_update_header(avi_head);
	return avi_head;
}


static bool mjpeg_frist_record_video(avi_t* avi_handle)
{
	unsigned char* video_data = NULL;
	unsigned int video_len = 0;
	unsigned long long video_pts = 0;

	unsigned char* audio_data = NULL;
	unsigned int audio_len = 0;
	unsigned long long audio_pts = 0;

	/*
	*写入头，第一帧应该是视频，但是前提保证音视频到达同步的队列才开始写
	*保证音视频同步的话，有两种情况，
	* 1.video_pts > audio_pts 视频在前，音频在后。则直接将视频写入
	* 2.video_pts < audio_pts 视频在后，音频在前。则需要过滤前面的视频帧，
	* 获取到视频在音频前面的第一帧画面*/
	while(record_mjpeg_run == true)
	{

		if(video_data == NULL)
		{
			mjpeg_encode_api.read(&video_data,&video_len,&video_pts);
			//printf("video pts:%llu \n\r",video_pts);
		}

		if(audio_data == NULL)
		{
			audio_input_api.read(&audio_data,&audio_len,&audio_pts);
			//printf("audio:pts:%llu \n\r",audio_pts);
		}


		if((video_data == NULL)||(audio_data == NULL))
		{
			usleep(10);
			continue;
		}
		
		if(abs(video_pts - audio_pts) <40000)
		{
			AVI_write_frame(avi_handle, (char*)video_data,video_len, 1);
			return true;
		}
		else if(video_pts > audio_pts)
		{
			audio_data = NULL;
		}
		else
		{
			video_data = NULL;
		}
	}
	return true;
}



static void* mjpeg_record_task(void* arg)
{
	unsigned char* video_data = NULL;
	unsigned int video_len = 0;
	unsigned long long video_pts = 0;

	unsigned char* audio_data = NULL;
	unsigned int audio_len = 0;
	unsigned long long audio_pts = 0;

	unsigned long long pre_video_pts = 0;
	//unsigned long long pre_audio_pts =0;

	unsigned long long video_frame_count = 0;
	//unsigned long long audio_frame_count = 0;

	//unsigned long long audio_frame_accumulation = 0;
	unsigned long long video_frame_accumulation = 0;

	char temp_file_path[128] = {0};
	sprintf(temp_file_path,"%stemp",record_mjpeg_path);
	avi_t* avi_handle = AVI_open_output_file(temp_file_path);
	avi_head_write(avi_handle,30.0);

	audio_input_api.start();
	mjpeg_encode_api.start();

	mjpeg_frist_record_video(avi_handle);

	while(record_mjpeg_run == true)
	{
		if(video_data == NULL)
		{
			mjpeg_encode_api.read(&video_data,&video_len,&video_pts);
		}

		if( audio_data == NULL)
		{
			audio_input_api.read(&audio_data,&audio_len,&audio_pts);
		}

		if((video_data == NULL)||(audio_data == NULL))
		{
			usleep(10);
			continue;
		}

		/*
		*如果video_pts > audio_pts :音频在后，视频在前。则需要直接写入音频帧。
		 如果video_pts < audio_pts :音频在前，视频在后，则直接将过滤掉视频帧，直到获取到大于音频帧的数据*/
		if(video_pts > audio_pts)
		{
			if(record_have_audio == false)
			{
				memset(audio_data,0,audio_len);
			}
			AVI_write_audio(avi_handle, (char*)audio_data, audio_len);
			audio_data = NULL;

			//if(pre_audio_pts)
			//{
			//	audio_frame_accumulation += (audio_pts -pre_audio_pts)/1000;
			//	audio_frame_count++;
			//}
			//pre_audio_pts = audio_pts;
			
		}
		else 
		{
			AVI_write_frame(avi_handle, (char*)video_data, video_len,1);
			video_data = NULL;

			if(pre_video_pts)
			{
				video_frame_accumulation += (video_pts -pre_video_pts)/1000;
				video_frame_count++;
				
 				record_mjpeg_duration = video_frame_accumulation;
			}
			pre_video_pts = video_pts;
		}
		usleep(1);
	}

	audio_input_api.stop();
	mjpeg_encode_api.stop();
	
	avi_handle->fps = 1000.0/((video_frame_accumulation+0.1)/(video_frame_count+0.1));
	printf("\n\n\nEncode to AVI Finish. video frame:%lffps drution:%dms \n\n\n\n",avi_handle->fps,record_mjpeg_duration);
	AVI_close(avi_handle);
		
	/******************
	校验视频是否错误
	*******************/
	rename(temp_file_path,record_mjpeg_path);
	media_bad_path_check(record_mjpeg_path);

	/******************
	同步记录的视频
	******************/
	system("sync");
	
	ak_thread_exit();
	return NULL;
}



static bool mjpeg_video_mux_start(bool has_audio,const char* file)
{
	if((record_mjpeg_run == true)||(record_mjpeg_thread_run == true))
	{
		return false;
	}

	memset(record_mjpeg_path,0,sizeof(record_mjpeg_path));
	strncpy(record_mjpeg_path,file,sizeof(record_mjpeg_path));
	
 	record_mjpeg_duration = 0;
	
	record_have_audio = has_audio;
	record_mjpeg_run = true;
	record_mjpeg_thread_run = true;

	ak_thread_create(&record_mjpeg_thread_id, mjpeg_record_task, NULL,ANYKA_THREAD_NORMAL_STACK_SIZE , -1);
	return true; 
}


static void* mjpeg_thread_join_task(void*arg)
{
	record_mjpeg_run = false;
	ak_thread_join(record_mjpeg_thread_id);
	record_mjpeg_thread_run = false;
	ak_pthread_t* thread_join_id = (ak_pthread_t*)arg;
	*thread_join_id = 0;
	
	ak_thread_exit();
	return NULL;
}


static bool mjpeg_video_mux_stop(void)
{
	static ak_pthread_t mjpeg_thread_join_id = 0;
	printf("stop video record \n\r");
	if((record_mjpeg_thread_run == false)||(record_mjpeg_run == false)||(mjpeg_thread_join_id != 0))
	{
		return false;
	}

	ak_thread_create(&mjpeg_thread_join_id, mjpeg_thread_join_task, &mjpeg_thread_join_id, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	ak_thread_detach(mjpeg_thread_join_id);
	ak_sleep_ms(1);
	return true;
}




static bool mjpeg_video_duration_get(int* duration)
{
	if((record_mjpeg_thread_run == false)||(record_mjpeg_run == false))
	{
		return false;
	}
	*duration = record_mjpeg_duration;
	return true;
}



leo_record_video_func mjpeg_video_mux = {

	.record_start = mjpeg_video_mux_start,

	.record_stop = mjpeg_video_mux_stop,

	.record_duration_get = mjpeg_video_duration_get,
};


