#include "ak_ao.h"
#include <stdbool.h>
#include "ak_thread.h"

static int audio_output_handle_id = -1;

static enum ak_audio_sample_rate auido_output_rate = AK_AUDIO_SAMPLE_RATE_64000;
static enum ak_audio_channel_type audio_ouput_channel = AUDIO_CHANNEL_RESERVED;

static ak_mutex_t audio_output_mutex;
bool audio_output_close(void);

static bool audio_output_mutex_init(void)
{
	static bool is_first_init = true;
	if(is_first_init == true)
	{
		is_first_init = false;
		ak_thread_mutex_init(&audio_output_mutex,NULL);
	}
	return true;
}

static bool audio_output_devices_open(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate)
{
	struct ak_audio_out_param param;
	param.dev_id = DEV_DAC;
	param.pcm_data_attr.channel_num = ch;
	param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
	param.pcm_data_attr.sample_rate = rate;
	printf("=============>>%d \n\r",param.dev_id);
	return ak_ao_open(&param, &audio_output_handle_id)?false:true;
}
static bool audio_output_restart_devices(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate)
{
	struct ak_audio_out_param param;
	param.dev_id = DEV_DAC;
	param.pcm_data_attr.channel_num = ch;
	param.pcm_data_attr.sample_bits = AK_AUDIO_SMPLE_BIT_16;
	param.pcm_data_attr.sample_rate = rate;
	return ak_ao_reset_params(audio_output_handle_id, &param)?false:true;
}
static bool audio_output_devices_open_check(void)
{
	return (audio_output_handle_id == -1)?false:true;
}
static bool audio_output_volume_set(int vol,int gain)
{
	if(audio_output_devices_open_check() == false)
	{
		return false;
	}
	ak_ao_set_gain(audio_output_handle_id, gain);
	ak_ao_set_volume(audio_output_handle_id, vol);
	ak_ao_set_speaker(audio_output_handle_id,1);
	//ak_ao_enable_eq(audio_output_handle_id,0);
	//ak_ao_enable_nr(audio_output_handle_id,0);
	//ak_ao_enable_hs(audio_output_handle_id,0);
	return true;
}




bool audio_output_open(enum ak_audio_channel_type ch,enum ak_audio_sample_rate rate,int vol,int gain)
{
	audio_output_mutex_init();
	
	ak_thread_mutex_lock(&audio_output_mutex);
	if((audio_output_devices_open_check() == false)||(auido_output_rate != rate)||(audio_ouput_channel != ch))
	{
		if(audio_output_devices_open_check() == true)
		{
			audio_output_restart_devices(ch,rate);
			//audio_output_close();
		}
		else
		{
			audio_output_devices_open(ch,rate);
		}
		auido_output_rate = rate;
		audio_ouput_channel = ch;
	}
	audio_output_volume_set(vol,gain);
	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}


bool audio_output_write(unsigned char* data,int len)
{
	ak_thread_mutex_lock(&audio_output_mutex);

	if(audio_output_devices_open_check() == false)
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}
	int play_len = 0;
	if(ak_ao_send_frame(audio_output_handle_id, data, len, &play_len))
	{
		ak_thread_mutex_unlock(&audio_output_mutex);
		return false;
	}

	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}


bool audio_output_close(void)
{
	ak_thread_mutex_lock(&audio_output_mutex);
	if(audio_output_handle_id != -1)
	{
		ak_ao_close(audio_output_handle_id);
		audio_output_handle_id = -1;
	}
	
	auido_output_rate = AK_AUDIO_SAMPLE_RATE_64000;
	audio_ouput_channel = AUDIO_CHANNEL_RESERVED;
	ak_thread_mutex_unlock(&audio_output_mutex);
	return true;
}


