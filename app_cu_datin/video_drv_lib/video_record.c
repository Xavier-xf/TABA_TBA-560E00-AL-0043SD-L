#include "video_record.h"

static leo_record_video_func* record_video_api = NULL;

bool record_video_init(void)
{
	record_video_api = &mjpeg_video_mux;
	return true;
}

bool record_video_open(bool has_audio,const char* file)
{
	return record_video_api->record_start(has_audio,file);
}

bool record_video_quit(void)
{
	return record_video_api->record_stop();
}

bool record_video_duration_get(int* duration)
{
	return record_video_api->record_duration_get(duration);
}


