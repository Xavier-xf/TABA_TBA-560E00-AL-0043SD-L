#include "video_play_api.h"
#include "ak_thread.h"
#include "avilib.h"
#include "stdbool.h"
#include "ak_ao.h"
#include "video_output.h"
#include "ak_mem.h"
#include "os_sys_api.h"
#include "queue.h"
#include <string.h>
#include "mjpeg_video_play.h"

bool video_play_start(const char* file,bool is_pause)
{
	mjpeg_video_play_api.play(file,is_pause);
	return true;
}




bool video_play_stop(void)
{
	mjpeg_video_play_api.stop();
	return true;
}



void video_play_init(void)
{
	
}


bool video_duration_get(int*cur,int*total)
{
	mjpeg_video_play_api.duration_get(cur,total);
	return true;
}

void video_play_state_set(VIDEO_PLAY_STATE state)
{
	if((state == VIDEO_PLAY_PAUSE)||(state == VIDEO_PLAY_ING))
	{
		mjpeg_video_play_api.pause();
	}
	else
	{
		mjpeg_video_play_api.stop();
	}
}

VIDEO_PLAY_STATE video_play_state_get(void)
{
	return mjpeg_video_play_api.get_status();
}



