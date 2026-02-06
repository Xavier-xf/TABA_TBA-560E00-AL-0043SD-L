#include <stdio.h>
#include <string.h>
#include "ak_thread.h"
#include "ak_common.h"
#include "os_sys_api.h"
#include "ui_api.h"
#include "gpio_api.h"
#include "queue.h"

#define OS_SYS_VERSION "OS_SYS LIB:build:21/07/01 VER:0.0.1"

typedef enum
{
    EVENT_TYPE_NONE,
    EVENT_TYPE_ENTER,
    EVENT_TYPE_TOUCH,
    EVENT_TYPE_KEY,
    EVENT_TYPE_RECORD,
    EVENT_TYPE_SD,
    EVENT_TYPE_INTERPHONE,
    EVENT_TYPE_CALL,
    EVENT_TYPE_ALARM,
    EVENT_TYPE_HOOK,
}event_type;


typedef struct
{
    event_type type;

    unsigned int arg1;
    unsigned int arg2;
}event_t;

#define EVENT_QUEUE_MAX 32
static event_t os_event_queue[EVENT_QUEUE_MAX]; 


static int os_event_index = 0;
static int os_event_total = 0;


static const layout* playout = NULL;

static ak_mutex_t os_mutex;

extern void key_touch_handle_destory(void);
extern bool key_touch_event_handle(int key_index,int key_state);
extern bool standby_timer_check(void);


static void(*os_record_event_func)(char ,bool ) = NULL;
void record_event_register(void(*func)(char,bool))
{
	ak_thread_mutex_lock(&os_mutex);
	os_record_event_func = func;
	ak_thread_mutex_unlock(&os_mutex);
}



static void(*os_interphone_event_func)(int arg1,int arg2) = NULL;
void interphone_event_register(void(*callback)(int arg1,int arg2))
{
	ak_thread_mutex_lock(&os_mutex);
	os_interphone_event_func = callback;
	ak_thread_mutex_unlock(&os_mutex);
}



static void(*os_door1_call_event_func)(void) = NULL;
static void(*os_door2_call_event_func)(void) = NULL;
void door_call_event_register(void(*func1)(void),void(*func2)(void))
{
	ak_thread_mutex_lock(&os_mutex);
	os_door1_call_event_func = func1;
	os_door2_call_event_func = func2;
	ak_thread_mutex_unlock(&os_mutex);
}



static void(*os_hook_down_event_func)(void) = NULL;
static void(*os_hook_up_event_func)(void) = NULL;
void hook_event_register(void(*hook_up)(void),void(*hook_down)(void))
{
	ak_thread_mutex_lock(&os_mutex);
	os_hook_down_event_func = hook_down;
	os_hook_up_event_func = hook_up;
	ak_thread_mutex_unlock(&os_mutex);
}



static void (*os_alarm_event_func)(int index) = NULL;
void alarm_event_register(void(*func)(int))
{
	ak_thread_mutex_lock(&os_mutex);
	os_alarm_event_func = func;
	ak_thread_mutex_unlock(&os_mutex);
}



static bool os_event_get(event_t* ev)
{
    ak_thread_mutex_lock(&os_mutex);
    if(os_event_total == 0)
    {
        ak_thread_mutex_unlock(&os_mutex);
        return false;
    }

    int index = os_event_index;
    os_event_index = (os_event_index+1)%EVENT_QUEUE_MAX;
    os_event_total -= 1;
    *ev = os_event_queue[index];
    ak_thread_mutex_unlock(&os_mutex);
    return (ev->type != EVENT_TYPE_NONE)?true:false;
}


static bool os_event_set(const event_t*ev)
{
    ak_thread_mutex_lock(&os_mutex);
    if(os_event_total >= EVENT_QUEUE_MAX)
    {
        ak_thread_mutex_unlock(&os_mutex);
        return false;
    }
    int index = (os_event_index + os_event_total)%EVENT_QUEUE_MAX;
    os_event_queue[index] = *ev;
    os_event_total +=1;
    ak_thread_mutex_unlock(&os_mutex);
    return true;
}

/*************************************
投递触摸事件
	注意点:如果是按下事件，需要将
队列中已有的触摸事件给清除，防止
触摸事件滞后响应
*************************************/
static bool touch_and_key_event_release(void)
{
    ak_thread_mutex_lock(&os_mutex);
    if(os_event_total == 0)
    {
        ak_thread_mutex_unlock(&os_mutex);
        return false;
    }

    int index = os_event_index;
    int i = 0;
    for(i = 0 ; i < os_event_total ; i++)
    {
        if((os_event_queue[index].type == EVENT_TYPE_TOUCH)||(os_event_queue[index].type == EVENT_TYPE_KEY))
        {
            os_event_queue[index].type = EVENT_TYPE_NONE;
        }
    }
    ak_thread_mutex_unlock(&os_mutex);
    return true;
}


static void os_event_release(void)
{
    os_event_index = 0;
    os_event_total = 0;
   // os_event_queue[0].type = EVENT_TYPE_ENTER;
}



#if 0
static void touch_event_release(void)
{
	ak_thread_mutex_lock(&os_mutex);
    if (os_event_total == 0){
        ak_thread_mutex_unlock(&os_mutex);
        return ;
    }

	int queue_index = os_event_index;
	int i;
	for( i = 0 ; i < os_event_total ; i++)
	{
		queue_index= (queue_index + 1) % EVENT_QUEUE_MAX;
		if(os_event_queue[queue_index].type == EVENT_TYPE_TOUCH)
		{
			os_event_queue[queue_index].type = EVENT_TYPE_NONE;
		}
	}
    ak_thread_mutex_unlock(&os_mutex);
}
#endif
#if 0
static void key_event_release(void)
{
	ak_thread_mutex_lock(&os_mutex);
    if (os_event_total == 0){
        ak_thread_mutex_unlock(&os_mutex);
        return ;
    }

	int queue_index = os_event_index;
	int i;
	for( i = 0 ; i < os_event_total ; i++)
	{
		queue_index= (queue_index + 1) % EVENT_QUEUE_MAX;
		if(os_event_queue[queue_index].type == EVENT_TYPE_KEY)
		{
			os_event_queue[queue_index].type = EVENT_TYPE_NONE;
		}
	}
    ak_thread_mutex_unlock(&os_mutex);
}
#endif



void os_evnet_init(void)
{
	printf(" *************** %s start ***************\n", __FUNCTION__);
    ak_thread_mutex_init(&os_mutex,NULL);
    memset(os_event_queue,0,sizeof(EVENT_QUEUE_MAX));
    os_event_index = 0;
    os_event_total = 0;

	queue_data_init(QUEUE_CHANNEL_VIDEO_1);
	queue_data_init(QUEUE_CHANNEL_VIDEO_2);
	queue_data_init(QUEUE_CHANNEL_AUDIO_1);
	
	extern	void video_play_init(void);
	video_play_init();


	extern bool record_video_init(void);
	record_video_init();
	printf(" *************** %s done ***************\n", __FUNCTION__);
}


//bool gui_bg_recovery(void);
static void os_event_pross(const event_t* ev)
{
    switch(ev->type)
    {
#if 0
        case EVENT_TYPE_ENTER:
            if((playout != NULL)&&(playout->enter != NULL))
            {
            	///*
				position pos = {{0,0}};
				pos.vector = *((const vector*)screen_vector_get());
				draw_rect(&pos, 0x00 , GUI_NORMAL_LAYER);
				//*/
				//gui_bg_recovery();
                playout->enter();
            }
        break;
#endif
		case EVENT_TYPE_TOUCH:
			{
				unsigned char status = ev->arg1;

				point point;
			    point.x = (ev->arg2>>16)&0xFFFF;
			    point.y = ev->arg2&0xFFFF;
				button_touch_event_handle(status,&point);
			}
		break;		
		case EVENT_TYPE_KEY:
			{
				key_touch_event_handle(ev->arg1,ev->arg2);
			}
		break;
		case EVENT_TYPE_RECORD:
			if(os_record_event_func != NULL)
			{
				os_record_event_func(ev->arg1,ev->arg2);
			}
		break;
		case EVENT_TYPE_INTERPHONE:
			if(os_interphone_event_func != NULL)
			{
				os_interphone_event_func(ev->arg1,ev->arg2);
			}
			break;
		case EVENT_TYPE_CALL:
			{
				if((ev->arg1 == 1)&&(os_door1_call_event_func != NULL))
				{
					os_door1_call_event_func();
				}
				else if((ev->arg1 == 2)&&(os_door2_call_event_func != NULL))
				{
					os_door2_call_event_func();
				}
			}
		break;
		case EVENT_TYPE_HOOK:
			{
				if((ev->arg1 == GPIO_LEVEL_LOW)&&(os_hook_down_event_func != NULL))
				{
					os_hook_down_event_func();
				}
				else if((ev->arg1 == GPIO_LEVEL_HIGH)&&(os_hook_up_event_func != NULL))
				{
					os_hook_up_event_func();
				}
			}
		break;
		case EVENT_TYPE_ALARM:
			if(os_alarm_event_func != NULL)
			{
				os_alarm_event_func(ev->arg1);
			}
		break;
        default:
            printf("os event type fail\n\r");
		break;
    }
}


static void os_event_loop(void)
{
    event_t ev;
    unsigned long ts_ms = 0;
	unsigned long te_ms = 0;
    while(1)
    {
        if(os_event_get(&ev) == true)
        {
            os_event_pross(&ev);
            if(ev.type == EVENT_TYPE_ENTER)
            {
				system("sync");
				system("echo 3 > /proc/sys/vm/drop_caches");
                ts_ms = os_get_ms();
            }
        }
        else
        {
            te_ms = os_get_ms();
            if(abs(te_ms - ts_ms) > 30)
            {
                if((playout != NULL)&&(playout->timer != NULL))
                {
                    playout->timer();
                }
				
				extern void screen_display();
				screen_display();
				
				ts_ms = te_ms;
            }
            else
            {
            	standby_timer_check();
				extern bool timed_event_check(void);
				timed_event_check();
				
                ak_sleep_ms(1);
            }
        }
    }
}




/****************************************************
 * 
 *                  API 
 * 
 * *************************************************/
unsigned long long os_get_ms(void)
{
    struct ak_timeval tv;
    ak_get_ostime(&tv);
    return tv.usec/1000 + tv.sec*1000;
}

unsigned long long os_get_us(void)
{
    struct ak_timeval tv;
    ak_get_ostime(&tv);
    return tv.usec + tv.sec*1000*1000;
}


void os_layout_goto(const layout* layout)
{
    /************************
     * 此处需要销毁监听事件
     * ***********************/     
    os_event_release();
    button_listens_destroy();
	key_touch_handle_destory();
	if((playout != NULL)&&(playout->quit != NULL))
    {
        playout->quit();
    }
	extern bool gui_layer_channge();
	gui_layer_channge();
	
    playout = layout;
	if((playout != NULL)&&(playout->enter != NULL))
    {
        playout->enter();
    }
}



bool touch_point_event(unsigned char state,int x,int y){

	/*************************
	触摸按下释放前面的触摸事件
	**************************/
	if(state == TOUCH_STATE_DOWN){
        touch_and_key_event_release();
	}
	
	event_t event;
	event.type = EVENT_TYPE_TOUCH;
	event.arg1 = state;
	event.arg2 = (x<<16)|(y&0xFFFFFF);
	os_event_set(&event);
	return true;
}

bool touch_key_event(int key_index,int key_state)
{
	/*************************
	触摸按下释放前面的触摸事件
	**************************/
	if(key_state == KEY_STATE_DOWN)
	{
        touch_and_key_event_release();
	}
	
	event_t event;
	event.type = EVENT_TYPE_KEY;
	event.arg1 = key_index;
	event.arg2 = key_state;
	os_event_set(&event);
	return true;
}


bool record_finish_event(char type,bool is_finish)
{
	event_t event;
	event.type = EVENT_TYPE_RECORD;
	event.arg1 = type;
	event.arg2 = is_finish;
	os_event_set(&event);
	return true;
}

bool interphone_event(int arg1,int arg2)
{
	event_t event;
	event.type = EVENT_TYPE_INTERPHONE;
	event.arg1 = arg1;
	event.arg2 = arg2;
	os_event_set(&event);
	return true;
}

bool door_call_event(int door_channel)
{
	event_t event;
	event.type = EVENT_TYPE_CALL;
	event.arg1 = door_channel;
	os_event_set(&event);
	return true;
}
bool hook_trigger_event(int arg)
{
	event_t event;
	event.type = EVENT_TYPE_HOOK;
	event.arg1 = arg;
	os_event_set(&event);
	return true;
}

bool alarm_trigger_event(int index)
{
	event_t event;
	event.type = EVENT_TYPE_ALARM;
	event.arg1 = index;
	os_event_set(&event);
	return true;
}




static void os_version_display(void)
{
    printf(OS_SYS_VERSION);
    printf("\n");
}




void os_start(const layout* layout)
{
    os_version_display();

    //os_evnet_init();
	
    os_layout_goto(layout);

    os_event_loop();
}





