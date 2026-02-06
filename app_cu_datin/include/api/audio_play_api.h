#ifndef AUDIO_PLAY_API_H
#define AUDIO_PLAY_API_H
#include "ak_ao.h"
#include <stdbool.h>

typedef struct
{
	long addres;
	int size;
	enum ak_audio_channel_type ch;
	enum ak_audio_sample_rate rate;
	enum ak_audio_type type;
}audio_info;

bool audio_play_init(audio_info* pinfo,int num);

void audio_play(int index,int vol,void(*start_callback)(void),void(*finish_callback)(void));

bool is_audio_play_ing(void);

void audio_play_stop(void);

#endif

