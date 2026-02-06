#ifndef AUDIO_INPUT_H
#define AUDIO_INPUT_H
#include "stdbool.h"
#include "ak_ai.h"

typedef struct
{

	bool (*open)(enum ak_audio_sample_rate,enum ak_audio_channel_type ch);

	bool (*read)(unsigned char**buffer,unsigned int* len,unsigned long long* pts);

	bool (*start)(void);

	bool (*stop)(void);

	bool (*close)(void);
}leo_audio_input_func;

extern leo_audio_input_func audio_input_api;


#endif

