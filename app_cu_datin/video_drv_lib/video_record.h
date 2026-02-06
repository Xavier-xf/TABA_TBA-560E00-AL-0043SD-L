#ifndef RECORD_VIDEO_H
#define RECORD_VIDEO_H
#include <stdbool.h>
#include <stdlib.h>

typedef struct
{
	bool (*record_start)(bool has_audio,const char* file_name);

	bool (*record_stop)(void);

	bool (*record_duration_get)(int* duration);
	
}leo_record_video_func;

extern leo_record_video_func mjpeg_video_mux;

#endif

