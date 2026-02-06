#include "mad.h"
#include "ak_thread.h"
#include "audio_play_api.h"
#include "string.h"
#include "ak_mem.h"
#include <sys/time.h>

extern bool audio_output_open(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate,int vol,int gain);
extern bool audio_output_write(unsigned char* data,int len);
extern bool audio_output_close(void);

static ak_cond_t audio_play_cond;
static ak_mutex_t audio_play_mutex;

static bool audio_play_ing = false;
static int audio_play_index = -1;

static audio_info* audio_play_info;
static int audio_play_number = 0;

static int audio_play_volume = 0;

static void(*audio_play_start_callback)(void) = NULL;
static void(*audio_play_finish_callback)(void) = NULL;


static bool audio_play_state = false;

extern unsigned char* rom_base_addres;

static void audio_play_pcm_wait(void)
{
	int time_delay = 40;
	while((audio_play_ing == true) && (time_delay--))
	{
		ak_sleep_ms(1);
	}
}

static bool audio_play_pcm(const audio_info* pinfo)
{
	if( audio_play_start_callback != NULL)
	{
		audio_play_start_callback();
	}
	//audio_output_volume_set(audio_play_volume - 80, 6);

	unsigned char* addres = rom_base_addres + pinfo->addres;
	int send_size = pinfo->size;
	int read_size = 0;
	int free_size = 0;
	while(send_size > 0)
	{
		if(send_size > 4096)
		{
			free_size = 4096;
		}
		else
		{
			free_size = 4096 - send_size;
		}
		if(audio_output_write(&addres[read_size] ,free_size /*pinfo->size > 4096?4096:pinfo->size*/))
		{
			audio_play_ing = false;
			return false;
		}
		read_size += free_size;
		send_size -= free_size;
	}
	
	audio_play_pcm_wait();
	return true;
}



typedef struct{
  unsigned char const *start;
  unsigned long length;
}mp3_mad;


static enum mad_flow mp3_input(void *data, struct mad_stream *stream)
{
	mp3_mad *buffer = data;
	if (!buffer->length)
	{
		return MAD_FLOW_STOP;
	}
	mad_stream_buffer(stream, buffer->start, buffer->length);
	buffer->length = 0;
	return MAD_FLOW_CONTINUE;
}


static inline signed int mp3_scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static enum mad_flow mp3_output(void *data,struct mad_header const *header,struct mad_pcm *pcm)
{
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;

	if(audio_play_ing == false)
	{
		return MAD_FLOW_STOP;
	}

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
	right_ch  = pcm->samples[1];

	#define MP3_OUTPUT_FRAME_SIZE 1180
	unsigned char sample_buffer[MP3_OUTPUT_FRAME_SIZE] = {0};
	int sample_read_size = 0;

	while (nsamples--) 
	{
		signed int sample;

		/* output sample(s) in 16-bit signed little-endian PCM */
		if(audio_play_ing == 0)
		{
			return MAD_FLOW_STOP;
		}
			
		sample = mp3_scale(*left_ch++);
		sample_buffer[sample_read_size++] = (sample >> 0) & 0xff;
		sample_buffer[sample_read_size++] = (sample >> 8) & 0xff;

		if (nchannels == 2)
		{
			sample = mp3_scale(*right_ch++);
			sample_buffer[sample_read_size++] = (sample >> 0) & 0xff;
			sample_buffer[sample_read_size++] = (sample >> 8) & 0xff;
		}

		/* output sample(s) in 16-bit signed little-endian PCM */
		if(audio_play_ing == false)
		{
			return MAD_FLOW_STOP;
		}
		if(sample_read_size == MP3_OUTPUT_FRAME_SIZE)
		{
			audio_output_write((unsigned char*)sample_buffer,sample_read_size);
			sample_read_size = 0;
		}
		
	}

	
	if(sample_read_size)
	{
		audio_output_write((unsigned char*)sample_buffer,sample_read_size);
	}
	if(audio_play_ing == false)
	{
		return MAD_FLOW_STOP;
	}
	return MAD_FLOW_CONTINUE;
}


static enum mad_flow mp3_error(void *data,struct mad_stream *stream,struct mad_frame *frame)
{
	return MAD_FLOW_CONTINUE;
}




static void audio_play_mp3_wait(void)
{
	int time_delay = 100;
	while((audio_play_ing == true)&&(time_delay--))
	{
		ak_sleep_ms(1);
	}
}

static bool audio_play_mp3(const audio_info* pinfo)
{
	mp3_mad mp3_buffer;
	struct mad_decoder decodec;

	mp3_buffer.start = rom_base_addres + pinfo->addres;
	mp3_buffer.length = pinfo->size;
	mad_decoder_init(&decodec,&mp3_buffer,mp3_input,0,0,mp3_output,mp3_error,0);
	if( audio_play_start_callback != NULL)
	{
		audio_play_start_callback();
	}
	//audio_output_volume_set(audio_play_volume - 80, 6);

	mad_decoder_run(&decodec,MAD_DECODER_MODE_SYNC);

	audio_play_mp3_wait();

	mad_decoder_finish(&decodec);

	//if(audio_play_ing == false)
	//{
	//	audio_output_close();
	//}
	return true;
}



static bool audio_play_start(void)
{
	audio_output_open(audio_play_info[audio_play_index].ch, audio_play_info[audio_play_index].rate,audio_play_volume-80,5);
	if(audio_play_info[audio_play_index].type == AK_AUDIO_TYPE_PCM)
	{
		audio_play_pcm(&audio_play_info[audio_play_index]);
	}
	else
	{
		audio_play_mp3(&audio_play_info[audio_play_index]);
	}

	return true;
}


static void audio_play_timeout_get(struct timespec * timeout)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long ns = tv.tv_usec*1000+ 100*1000*1000;
	timeout->tv_nsec = ns%(1000*1000*1000);
	timeout->tv_sec =  tv.tv_sec + ns/(1000*1000*1000);
}

static void* audio_play_task(void* arg)
{
	struct timespec time_out;
	while(1)
	{
		ak_thread_mutex_lock(&audio_play_mutex);
		audio_play_timeout_get(&time_out);
		ak_thread_cond_timedwait(&audio_play_cond,&audio_play_mutex,&time_out);
		if((audio_play_ing == false)||(audio_play_index < 0) ||(audio_play_index > audio_play_number))
		{			
			ak_thread_mutex_unlock(&audio_play_mutex);
			continue;
		}

		audio_play_state = true;
		audio_play_start();		
		audio_play_state = false;
		if(audio_play_finish_callback != NULL)
		{
			audio_play_finish_callback();
		}
		
		audio_play_ing = false;
		ak_thread_mutex_unlock(&audio_play_mutex);
	}
	ak_thread_exit();
	return NULL;
}


bool audio_play_init(audio_info* pinfo,int num)
{
	ak_pthread_t thread_id;
	ak_thread_cond_init(&audio_play_cond);
	ak_thread_mutex_init(&audio_play_mutex,NULL);

	audio_play_info = (audio_info*)ak_mem_alloc(MODULE_ID_AO, sizeof(audio_info)*num);
	memcpy(audio_play_info,pinfo,sizeof(audio_info)*num);
	audio_play_number = num;
	ak_thread_create(&thread_id, audio_play_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE , -1);
	return true;
}

void audio_play(int index,int vol,void(*start_callback)(void),void(*finish_callback)(void))
{
	audio_play_ing = false;
	ak_thread_mutex_lock(&audio_play_mutex);
	audio_play_index = index;
	audio_play_volume = vol;
	audio_play_ing = true;
	audio_play_state = true;
	audio_play_start_callback = start_callback;
	audio_play_finish_callback = finish_callback;
	ak_thread_cond_signal(&audio_play_cond);
	ak_thread_mutex_unlock(&audio_play_mutex);
}

bool is_audio_play_ing(void)
{
	return audio_play_state;
}

void audio_play_stop(void)
{
	audio_play_ing = false;
	ak_thread_mutex_lock(&audio_play_mutex);
	audio_play_index = -1;
	audio_play_state = false;	
	audio_output_close();
	ak_thread_mutex_unlock(&audio_play_mutex);
}

