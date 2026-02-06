#include "mjpeg_video_play.h"
#include "avilib.h"
#include "ak_thread.h"
#include "stdio.h"
#include "ak_mem.h"
#include "thumb_decodec.h"
#include "ak_ao.h"
#include "queue.h"
#include "os_sys_api.h"


#define AVI_BUFFER_MAX 1024*1024
#define AVI_AUDIO_FRAME_SIZE 512

#define PLAY_VIDEO_STATE_IDLE 	0X00
//#define PLAY_VIDEO_STATE_STOP 	0X01
#define PLAY_VIDEO_STATE_PLAY 	0X02
#define PLAY_VIDEO_STATE_PAUSE 	0X03
static unsigned char mjpeg_video_play_state = PLAY_VIDEO_STATE_IDLE;

const vector* screen_vector_get(void);

bool audio_output_open(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate,int vol,int gain);
bool audio_output_write(unsigned char* data,int len);
bool audio_output_close(void);


static bool mjpeg_demux_run = false;

static ak_pthread_t mjpeg_demux_thread_id = 0;

static char avi_file_path[128] = {0};

static bool mjpeg_play_one_frame = false;


static int video_frame_index = 0;
static int audio_frame_index = 0;
static unsigned long long clock_base_time = 0;

static unsigned long long mjpeg_video_duration = 0;
static unsigned long long mjpeg_audio_duration = 0;


static bool video_play_eof = false;

static int video_frame_total = 0;


static bool mjpeg_video_one_frame_display(avi_t* avi_handle, char* frame_buffer,const position* pos)
{
	AVI_set_video_position(avi_handle,0);

	long frame_size = AVI_frame_size(avi_handle,0);
	
	int key_frame = 0;
	AVI_read_frame(avi_handle,frame_buffer,&key_frame);

	mjpeg_stream_send((unsigned char*)frame_buffer,frame_size);
	
	mjpeg_frame_display(pos);

	return true;
}



static void* mjpeg_frame_display_task(void* arg)
{
	bool* run = (bool*)arg;

	position pos = {{0},{0}};
	pos.vector =*(const vector*)screen_vector_get();

	while((*run) == true)
	{
		if(mjpeg_video_play_state == PLAY_VIDEO_STATE_PAUSE)
		{
			ak_sleep_ms(10);
			continue;
		}
		
		queue_data* data = queue_data_pop(QUEUE_CHANNEL_VIDEO_2);
		if((data != NULL)&&(data->addres != NULL))
		{
			mjpeg_stream_send(data->addres,data->size);

			struct ak_vdec_frame frame = {0};
			if(ak_vdec_get_frame(thumb_device_handle_get(), &frame) == 0)
			{
				unsigned long long v_pts = clock_base_time + video_frame_index*mjpeg_video_duration;
				unsigned long long cur_pts = os_get_ms();
				if(((cur_pts >= v_pts)&&((cur_pts - v_pts) < 25))||((cur_pts < v_pts)&&((v_pts - cur_pts) < 100)))
				{
					thumb_frame_display(&pos,&frame);
				}
				else if(cur_pts < v_pts)
				{
					ak_sleep_ms(v_pts - cur_pts);
					thumb_frame_display(&pos,&frame);
				}
				else
				{
					printf("skip video frame \n\r");
				}
				ak_vdec_release_frame(thumb_device_handle_get(), &frame);
				video_frame_index++;	
			}
		}
		
		ak_sleep_ms(1);
	}
	printf("mjpeg frame display finish \n\r");
	ak_thread_exit();
	return NULL;
}

static void* audio_frame_play_task(void*arg)
{
	bool*run = (bool*)arg;
	
	audio_output_open(AUDIO_CHANNEL_MONO, AK_AUDIO_SAMPLE_RATE_8000 , 20, 5);

	while((*run) == true)
	{
		if(mjpeg_video_play_state == PLAY_VIDEO_STATE_PAUSE)
		{
			ak_sleep_ms(10);
			continue;
		}
		
		queue_data* data = queue_data_pop(QUEUE_CHANNEL_AUDIO_1);
		if((data != NULL)&&(data->addres != NULL))
		{			
			audio_output_write((unsigned char*)data->addres,data->size);

			unsigned long long a_pts = clock_base_time + audio_frame_index*mjpeg_audio_duration;
			unsigned long long cur_pts = os_get_ms();
			if( cur_pts >=  a_pts)
			{			
				audio_frame_index++;
				clock_base_time = os_get_ms() - audio_frame_index*mjpeg_audio_duration;
			}
		}
		ak_sleep_ms(1);
	}
	
	audio_output_close();

	ak_thread_exit();
	return NULL;
}

static unsigned long long mjpeg_video_frame_duration_get(avi_t* handle)
{
	return 1000/(AVI_frame_rate(handle)+0.1);
}

static unsigned long long mjpeg_audio_frame_duration_get(avi_t* handle)
{
	long sample = AVI_audio_rate(handle);
	printf("audio smaple:%ld \n\r",sample);
	long channel = AVI_audio_channels(handle);
	printf("audio channel:%ld \n",channel);
	long byte = AVI_audio_bits(handle);	
	printf("audio byte:%ld \n\r",byte);
	return (AVI_AUDIO_FRAME_SIZE*8*1000)/(sample*byte*channel+0.1);
}


static void* mjpeg_demux_task(void* arg)
{
	char *pdata = (char*)ak_mem_alloc(MODULE_ID_VDEC, AVI_BUFFER_MAX);
	
	position pos = {{0},{0}};
	pos.vector =*(const vector*)screen_vector_get();


	printf("mjpeg video :%s play \n\r",avi_file_path);
	avi_t* avi_handle = AVI_open_input_file(avi_file_path,1);

	if(mjpeg_play_one_frame == true)
	{
		mjpeg_video_one_frame_display(avi_handle,pdata,&pos);
	}

	queue_data_release(QUEUE_CHANNEL_VIDEO_2);
	ak_pthread_t disp_thread_id;
	bool display_thread_run = true;
	ak_thread_create(&disp_thread_id,mjpeg_frame_display_task,&display_thread_run,ANYKA_THREAD_NORMAL_STACK_SIZE,-1);


	queue_data_release(QUEUE_CHANNEL_AUDIO_1);
	ak_pthread_t audio_thread_id;
	bool audio_thread_run = true;
	ak_thread_create(&audio_thread_id,audio_frame_play_task, &audio_thread_run , ANYKA_THREAD_NORMAL_STACK_SIZE, -1);

	AVI_set_video_position(avi_handle,0);

	video_frame_index = audio_frame_index = 0;

	mjpeg_video_duration = mjpeg_video_frame_duration_get(avi_handle);
	printf("video frame duration:%llu ms\n\r",mjpeg_video_duration);


	mjpeg_audio_duration = mjpeg_audio_frame_duration_get(avi_handle);
	printf("audio frame duration:%llu ms \n\r",mjpeg_audio_duration);

	video_frame_total = AVI_video_frames(avi_handle);
	printf("video frames:%d \n\r",video_frame_total);

	video_play_eof = false;

	clock_base_time = os_get_ms();
	
	int data_len = 0;
	while(mjpeg_demux_run == true)
	{
		if(mjpeg_video_play_state == PLAY_VIDEO_STATE_PAUSE)
		{
			ak_sleep_ms(10);
			continue;
		}
	
		if(queue_data_full_check(QUEUE_CHANNEL_AUDIO_1) == false)
		{
			if((data_len = (int)AVI_read_audio(avi_handle, pdata, AVI_AUDIO_FRAME_SIZE)) > 0)
			{
				queue_data_push(QUEUE_CHANNEL_AUDIO_1, (unsigned char*)pdata, data_len, 0,0);
			}
		}


		if(queue_data_full_check(QUEUE_CHANNEL_VIDEO_2) == false)
		{
			int key = 0;
			if((data_len = (int)AVI_read_frame(avi_handle, pdata, &key)) > 0)
			{
				queue_data_push(QUEUE_CHANNEL_VIDEO_2, (unsigned char*)pdata, data_len, 0,0);
			}
		}

		//printf("cur index:%d:%d \n",video_frame_index,video_frames);
		if((video_frame_index >= video_frame_total) &&
			(queue_data_empty_check(QUEUE_CHANNEL_AUDIO_1) == true)&&
			(queue_data_empty_check(QUEUE_CHANNEL_VIDEO_2) == true))
		{
			video_play_eof = true;
			mjpeg_video_play_state = PLAY_VIDEO_STATE_PAUSE;
			video_frame_index = audio_frame_index = 0;
			AVI_set_video_position(avi_handle,0);
			AVI_set_audio_position(avi_handle,0);
			printf("video play eof \n\r");
		}
		ak_sleep_ms(1);
	}
	ak_mem_free(pdata);

	display_thread_run = false;
	ak_thread_join(disp_thread_id);

	audio_thread_run = false;
	ak_thread_join(audio_thread_id);
	queue_data_release(QUEUE_CHANNEL_AUDIO_1);
	queue_data_release(QUEUE_CHANNEL_VIDEO_2);

	AVI_close(avi_handle);
	
	ak_thread_exit();
	return NULL;
}




static bool mjpeg_video_play(const char* file,bool is_pause)
{
	if(mjpeg_demux_run == true)
	{
		return false;
	}

	memset(avi_file_path ,0,sizeof(avi_file_path));
	strcpy(avi_file_path,file);

	mjpeg_play_one_frame = is_pause;

	mjpeg_video_play_state = is_pause?PLAY_VIDEO_STATE_PAUSE:PLAY_VIDEO_STATE_PLAY;
	
	mjpeg_demux_run = true;
	ak_thread_create(&mjpeg_demux_thread_id, mjpeg_demux_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE , -1);
	return true;
}



static bool mjpeg_video_stop(void)
{
	if(mjpeg_demux_run == false)
	{
		return false;
	}
	mjpeg_demux_run = false;
	ak_thread_join(mjpeg_demux_thread_id);
	mjpeg_video_play_state = PLAY_VIDEO_STATE_IDLE;
	return true;
}



static bool mjpeg_video_pause(void)
{
	if(mjpeg_video_play_state == PLAY_VIDEO_STATE_IDLE)
	{
		return false;
	}

	if(mjpeg_video_play_state == PLAY_VIDEO_STATE_PAUSE)
	{
		video_play_eof = false;
		clock_base_time = os_get_ms() - audio_frame_index*mjpeg_audio_duration;
		mjpeg_video_play_state = PLAY_VIDEO_STATE_PLAY;
	}
	else if(mjpeg_video_play_state == PLAY_VIDEO_STATE_PLAY)
	{
		mjpeg_video_play_state = PLAY_VIDEO_STATE_PAUSE;
	}
	return true;
}


static VIDEO_PLAY_STATE mjpeg_video_get_status(void)
{
	return mjpeg_video_play_state== PLAY_VIDEO_STATE_PLAY?VIDEO_PLAY_ING:
		   mjpeg_video_play_state==PLAY_VIDEO_STATE_PAUSE?VIDEO_PLAY_PAUSE:VIDEO_PLAY_STOP;
}


static bool mjpeg_video_play_duration_get(int* cur,int *total)
{
	if(mjpeg_video_play_state == PLAY_VIDEO_STATE_IDLE)
	{
		return false;
	}

	*total = (int)video_frame_total*mjpeg_video_duration;
	if(video_play_eof == true)
	{
		*cur = *total;
	}
	else
	{
		*cur = (int)audio_frame_index*mjpeg_audio_duration;
	}

	return true;
}

leo_mjpeg_play_func mjpeg_video_play_api = {

	.play = mjpeg_video_play,

	.stop = mjpeg_video_stop,

	.pause = mjpeg_video_pause,

	.get_status = mjpeg_video_get_status,

	.duration_get = mjpeg_video_play_duration_get,
};

