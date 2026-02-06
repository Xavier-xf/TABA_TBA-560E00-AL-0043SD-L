#ifndef MOTION_DETECT_H
#define MOTION_DETECT_H

typedef struct
{
	bool(*open)(int sensivity,unsigned char threshold);
	
	bool(*get_frame)(const struct video_input_frame* frame,const vector* vector);

	bool(*check_motion)(void);
	
	bool(*close)(void);

}leo_motion_detect_func;

extern leo_motion_detect_func motion_detect_api;

#endif

