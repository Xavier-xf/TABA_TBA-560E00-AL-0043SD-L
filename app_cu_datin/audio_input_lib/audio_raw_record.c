#include "audio_raw_api.h"
#include "ak_thread.h"
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "audio_input.h"
#include "ak_mem.h"

static bool audio_record_run = false;

static ak_pthread_t audio_record_thread_id = 0;

static char audio_record_file[128] = {0};

static void* audio_raw_record_task(void* arg)
{
	audio_input_api.open(AK_AUDIO_SAMPLE_RATE_8000,AUDIO_CHANNEL_MONO);

	int fd = open(audio_record_file,O_CREAT|O_WRONLY);

	unsigned char* pdata = NULL;
	unsigned int data_len = 0;
	while(audio_record_run == true)
	{
		if(audio_input_api.read(&pdata,&data_len,false) == true)
		{
			if(fd >= 0)
			{
				write(fd,pdata,data_len);
			}

			ak_mem_free(pdata);
		}
		else
		{
			ak_sleep_ms(1);
		}
	}
	close(fd);

	audio_input_api.close();
	
	ak_thread_exit();
	return NULL;
}



bool audio_raw_record_start(const char* file,enum ak_audio_sample_rate rate,enum ak_audio_channel_type ch)
{
	if(audio_record_run == true)
	{
		return false;
	}

	memset(audio_record_file,0,sizeof(audio_record_file));
	strcpy(audio_record_file,file);

	audio_record_run = true;
	ak_thread_create(&audio_record_thread_id, audio_raw_record_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	return true;
}

bool audio_raw_record_stop(void)
{
	if(audio_record_run == false)	
	{
		return false;
	}
	audio_record_run = false;
	ak_thread_join(audio_record_thread_id);
	audio_record_thread_id = 0;
	return true;
}

bool audio_raw_record_check(void)
{
	return audio_record_run;
}



