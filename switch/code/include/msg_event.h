#ifndef __MSG_EVENT_H
#define __MSG_EVENT_H




typedef enum{
    EVENT_TYPE_NONE = 0X00,
	EVENT_TYPE_FLOOR_NUMBER,
	EVENT_TYPE_TIMER,
	EVENT_TYPE_CALL,
	EVENT_TYPE_UNLOCK_START,
	EVENT_TYPE_UNLOCK_END,
	EVENT_TYPE_MONITOR_START,
	EVENT_TYPE_MONITOR_END,
	EVENT_TYPE_OPEN_AUDIO_VIDEO,
	EVENT_TYPE_CLOSE_AUDIO_VIDEO,
    EVENT_TYPE_TOTAL
}EVENT_TYPE;

typedef enum{
	HOME_INDEX_1  = 1,
	HOME_INDEX_2,
	HOME_INDEX_3,
	HOME_INDEX_4,
	HOME_INDEX_TOTAL
}MONITOR_SEND_TYPE;

typedef struct{

    EVENT_TYPE type;
    unsigned int arg1;
    unsigned int arg2;
}Event;


extern void msg_run(void);
extern char msg_event_post(Event* ev);
	
#endif
