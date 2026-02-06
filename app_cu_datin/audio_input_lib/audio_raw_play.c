#include "audio_raw_api.h"
#include "ak_thread.h"
#include <fcntl.h>
#include "queue.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

bool audio_output_open(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate,int vol,int gain);
bool audio_output_write(unsigned char* data,int len);
bool audio_output_close(void);


static bool audio_raw_play_run = false;
static ak_pthread_t audio_raw_play_thread_id = 0;
static char audio_play_file[128] = {0};

static enum ak_audio_sample_rate audio_sample_rate;
static enum ak_audio_channel_type audio_sample_ch;

static void* audio_raw_play_task(void* arg)
{
	int fd = open(audio_play_file,O_RDONLY);

	audio_output_open( audio_sample_ch,audio_sample_rate,20,5);

	unsigned char buffer[1024] = {0};
	int read_len = 0;
	while((fd >= 0) && (audio_raw_play_run == true))
	{
		if(( read_len = read(fd,buffer,1024))>0)
		{
			audio_output_write(buffer ,read_len);
		}
		else
		{
			break;
		}
		//printf("===============>>read_len =- %d \n\r",read_len);
		ak_sleep_ms(5);
	}
	audio_output_close();
	
	while(audio_raw_play_run == true)
	{
		ak_sleep_ms(10);
	}
	
	close(fd);
	ak_thread_exit();
	return NULL;
}


bool audio_raw_play_start(const char* file,enum ak_audio_sample_rate rate,enum ak_audio_channel_type ch)
{
	if(audio_raw_play_run == true)
	{
		return false;
	}

	memset(audio_play_file,0,sizeof(audio_play_file));
	strcpy(audio_play_file,file);

	audio_sample_rate = rate;
	audio_sample_ch = ch;

	audio_raw_play_run = true;
	ak_thread_create(&audio_raw_play_thread_id, audio_raw_play_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	return true;
}

bool audio_raw_play_stop(void)
{
	if(audio_raw_play_run == false)	
	{
		return false;
	}
	audio_raw_play_run = false;
	ak_thread_join(audio_raw_play_thread_id);
	audio_raw_play_thread_id = 0;

	return true;
}

bool audio_raw_play_check(void)
{
	return audio_raw_play_run;
}

