#include "ak_mem.h"
#include <dirent.h>
#include "ak_thread.h"
#include "ak_common.h"
#include "string.h"
#include <sys/types.h>    
#include <sys/stat.h>   
#include <unistd.h>
#include "ak_ai.h"
#include <stdbool.h>
#include "queue.h"
#include "os_sys_api.h"
#include "audio_play_api.h"
#include "audio_input.h"

static int ai_handle_id = -1;

static ak_pthread_t ai_thread_id = 0;

static bool ai_thread_run = false;

static enum ak_audio_sample_rate audio_input_rate;
static enum ak_audio_channel_type audio_input_channel;


static bool ai_capture_start = false;

static void setup_default_audio_argument(void *audio_args, char args_type)
{
    struct ak_audio_nr_attr     default_ai_nr_attr      = {-40, 0, 1};
    struct ak_audio_agc_attr    default_ai_agc_attr     = {24576, 4, 0, 80, 0, 1};
    struct ak_audio_aec_attr    default_ai_aec_attr     = {0, /*1024*/1024, 1024, 0, /*512*/640, 1};
    struct ak_audio_aslc_attr   default_ai_aslc_attr    = {9830, 0, 0};

    struct ak_audio_nr_attr     default_ao_nr_attr      = {-40, 0, 1};
    struct ak_audio_aslc_attr   default_ao_aslc_attr    = {9830, 0, 0};
    
    switch(args_type)
    {
    case 1:
        *(struct ak_audio_nr_attr*)audio_args = default_ai_nr_attr;
        break;
    case 2:
        *(struct ak_audio_agc_attr*)audio_args = default_ai_agc_attr;
        break;
    case 3:
        *(struct ak_audio_aec_attr*)audio_args = default_ai_aec_attr;
        break;
    case 4:
        *(struct ak_audio_aslc_attr*)audio_args = default_ai_aslc_attr;
        break;
    case 5:
        *(struct ak_audio_nr_attr*)audio_args = default_ao_nr_attr;
        break;
    case 6:
        *(struct ak_audio_aslc_attr*)audio_args = default_ao_aslc_attr;
        break;
    default:
        break;
    }

    return;
}



static bool audio_input_device_open(enum ak_audio_sample_rate rate,enum ak_audio_channel_type ch)
{
	//audio_play_stop();

	struct ak_audio_in_param ai_param;
    ai_param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
    ai_param.pcm_data_attr.channel_num = ch;
    ai_param.pcm_data_attr.sample_rate = rate;//AK_AUDIO_SAMPLE_RATE_8000;
    ai_param.dev_id = DEV_ADC;
	if(ak_ai_open(&ai_param,&ai_handle_id))
	{
		return false;
	}

	ak_ai_set_source(ai_handle_id, AI_SOURCE_MIC);

   	ak_ai_set_gain(ai_handle_id,3);// enable_agc
 	ak_ai_set_volume(ai_handle_id, 0);// enable_agc

	struct ak_audio_nr_attr nr_attr;
	setup_default_audio_argument(&nr_attr, 1);
	ak_ai_set_nr_attr(ai_handle_id, &nr_attr);
	ak_ai_enable_nr(ai_handle_id, AUDIO_FUNC_ENABLE);

	struct ak_audio_agc_attr agc_attr;
	setup_default_audio_argument(&agc_attr, 2);
	ak_ai_set_agc_attr(ai_handle_id, &agc_attr);
	ak_ai_enable_agc(ai_handle_id, AUDIO_FUNC_ENABLE);
	
	struct ak_audio_aec_attr aec_attr;
	setup_default_audio_argument(&aec_attr, 3);
	ak_ai_set_aec_attr(ai_handle_id,&aec_attr);
	ak_ai_enable_aec(ai_handle_id, AUDIO_FUNC_DISABLE);
	
	ak_ai_enable_eq(ai_handle_id, AUDIO_FUNC_DISABLE);
	
	//ak_ai_set_frame_length(ai_handle_id, 1280);
	return true;
}



static bool audio_input_capture(void)
{
	struct frame pcm_frame = {0};
	if(ak_ai_get_frame(ai_handle_id, &pcm_frame,0) == 0)
	{
		if(ai_capture_start == true)
		{
			queue_data_push(QUEUE_CHANNEL_AUDIO_1,pcm_frame.data,pcm_frame.len,os_get_us(),false);
		}
		ak_ai_release_frame(ai_handle_id, &pcm_frame);
		return true;
	}
	return false;
}


static void* audio_input_task(void* arg)
{
	audio_input_device_open(audio_input_rate,audio_input_channel);

	ak_ai_start_capture(ai_handle_id);
	
	//bool capture_status_pre = ai_capture_start;

	//bool audio_input_capture_open = false;

	printf("audio input open finish \n\r");
	while(ai_thread_run == true)
	{
		//if(ai_capture_start != capture_status_pre)
		//{
			//capture_status_pre = ai_capture_start;
			//if(ai_capture_start == true)
			//{
				
				//audio_input_capture_open = true;
				//printf("audio capture start... \n\r");
			//}
			//else
			//{
				//ak_ai_stop_capture(ai_handle_id);
				//audio_input_capture_open = false;
				//printf("audio capture stop... \n");
				//queue_data_release(QUEUE_CHANNEL_AUDIO_1);
			//}
		//}
		//else if(audio_input_capture_open == true)
		//{
			audio_input_capture();
		//}
		ak_sleep_ms(1);
	}

	//if(audio_input_capture_open == true)
	//{
		ak_ai_stop_capture(ai_handle_id);
	//}

	ak_ai_close(ai_handle_id);
	ai_handle_id = -1;
	ak_thread_exit();
	return NULL;
}



static bool audio_input_open(enum ak_audio_sample_rate rate,enum ak_audio_channel_type ch)
{
	if(ai_thread_run == true)
	{
		return false;
	}

	audio_input_rate = rate;
	audio_input_channel = ch;
	
	queue_data_release(QUEUE_CHANNEL_AUDIO_1);

	ai_capture_start = false;
	ai_thread_run = true;
	ak_thread_create(&ai_thread_id, audio_input_task, NULL,ANYKA_THREAD_NORMAL_STACK_SIZE , -1);
	return true;
}


static bool audio_input_read(unsigned char** buffer,unsigned int* len,unsigned long long* pts)
{
	*buffer = NULL;
	
	if((ai_handle_id == -1)||(ai_thread_run == false))
	{
		return false;
	}

	queue_data* node = queue_data_pop(QUEUE_CHANNEL_AUDIO_1);
	if((node == NULL)||(node->addres == NULL))
	{
		return false;
	}

	*buffer = node->addres;
	*len = node->size;
	*pts = node->ts;
	return true;
}


static bool audio_input_close(void)
{
	if(ai_thread_run == false)
	{
		return false;
	}
	ai_thread_run = false;
	ak_thread_join(ai_thread_id);
	ai_thread_id = 0;
	queue_data_release(QUEUE_CHANNEL_AUDIO_1);
	return true;
}


static bool audio_input_start(void)
{
	if((ai_thread_run == false)||(ai_handle_id == -1) ||(ai_capture_start == true))
	{
		printf("audio input capture fail \n\r");
		return false;
	}

	ai_capture_start = true;
	
	printf("audio input capture success \n\r");
	return true;
}

static bool audio_input_stop(void)
{
	if((ai_thread_run == false)||(ai_handle_id == -1) ||(ai_capture_start == false))
	{
		return false;
	}
	ai_capture_start = false;
	return true;
}

leo_audio_input_func audio_input_api = {

	.open = audio_input_open,

	.read = audio_input_read,

	.start = audio_input_start,

	.stop = audio_input_stop,

	.close = audio_input_close,
};




