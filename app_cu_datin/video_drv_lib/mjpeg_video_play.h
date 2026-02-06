#ifndef MJPEG_VIDEO_PLAY_H
#define MJPEG_VIDEO_PLAY_H
#include "stdbool.h"
#include "video_play_api.h"

typedef struct
{

	bool (*play)(const char* file,bool is_pause);

	bool (*stop)(void);

	bool (*pause)(void);

	VIDEO_PLAY_STATE (*get_status)(void);

	bool (*duration_get)(int*cur,int* total);
	
}leo_mjpeg_play_func;

extern leo_mjpeg_play_func mjpeg_video_play_api;

#endif

