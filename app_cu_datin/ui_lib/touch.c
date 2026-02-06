#include "ui_api.h"
#include "ak_thread.h"
#include "math.h"
#include "tslib.h"
#include "ak_common.h"


#define TOUCH_EVENT_PATH "/dev/input/event0"

static int touch_x_scale = 0;
static int touch_y_scale = 0;

#define TOUCH_LONG_DOWN_DELAY_MS 900

static bool touch_pos_swap = false;

extern bool touch_point_event(unsigned char state,int x,int y);

#if 1

static struct tsdev* touch_input_device;
static bool touch_point_handle(const struct ts_sample* sample,char* state,bool is_action){

	/*****************
	用于记录长按的时间
	******************/
	static struct ak_timeval touch_down_start_timeval;
	struct ak_timeval touch_down_finish_timeval;

	/*******************
	用于记录上一次触摸点
	*******************/
	static point point_pre;

	static char touch_status_pre = TOUCH_STATE_UP;

	bool reslut = false;

	*state = TOUCH_STATE_UP;
	if(is_action == false){

		if(touch_status_pre != TOUCH_STATE_UP){
			ak_get_ostime(&touch_down_finish_timeval);
			if(abs(touch_down_finish_timeval.usec/1000 + touch_down_finish_timeval.sec*1000 - touch_down_start_timeval.usec/1000 - touch_down_start_timeval.sec*1000) > TOUCH_LONG_DOWN_DELAY_MS){
				*state = TOUCH_STATE_LONG_DOWN;
				ak_get_ostime(&touch_down_start_timeval);			
				reslut = true;
			}
		}		
		goto HANDLE_FINISH;
	}

	if(sample->pressure == 0){
		*state = TOUCH_STATE_UP;
		reslut = true;
		goto HANDLE_FINISH;
	}

	/*****************************
	如果按下，则重新获取时间戳
	******************************/
	ak_get_ostime(&touch_down_start_timeval);
	if(touch_status_pre == TOUCH_STATE_UP){
		*state = TOUCH_STATE_DOWN;
		reslut = true;
		goto HANDLE_FINISH;
	}else{
		if((abs(point_pre.x - sample->x) > 5) ||(abs(point_pre.y - sample->y ) > 5)){
			*state = TOUCH_STATE_MOTION;
			reslut = true;
			goto HANDLE_FINISH;
		}
	}
HANDLE_FINISH:
	if(reslut == true){
		touch_status_pre = *state;
	}
	point_pre.x = sample->x;
	point_pre.y = sample->y;
	return reslut;
}


static void* touch_task(void* arg)
{
	struct ts_sample touch_sample;
	bool touch_action = false;
	char touch_status;
	
	while(1)
	{
		touch_action = ts_read(touch_input_device,&touch_sample,1) == 1?true:false;
		
		if((touch_action == true)&&(touch_pos_swap == true))
		{
			int temp_x = touch_sample.x;
			touch_sample.x = touch_sample.y;
			touch_sample.y = temp_x;
			printf("%d(%d,%d) \n\r",touch_sample.pressure,touch_sample.x,touch_sample.y);
		}
		if(touch_point_handle(&touch_sample,&touch_status,touch_action) == true){
			if(touch_x_scale != 100)
			{
				touch_sample.x = touch_sample.x*touch_x_scale/100;
			}
			if(touch_y_scale != 100)
			{
				touch_sample.y = touch_sample.y*touch_y_scale/100;
			}
			touch_point_event(touch_status, touch_sample.x, touch_sample.y);
			//printf("%d(%d,%d) \n\r",touch_status,touch_sample.x,touch_sample.y);
		}
		ak_sleep_ms(1);
	}
	ak_thread_exit();
	return NULL;
}


bool touch_init(int x_scale,int y_scale,bool swap_xy)
{
	touch_input_device = ts_setup(TOUCH_EVENT_PATH,1);
	if(touch_input_device == NULL)
	{
		printf("touch input setup fail \n\r");
		return false;
	}
	touch_x_scale = x_scale;
	touch_y_scale = y_scale;
	touch_pos_swap = swap_xy;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, touch_task,NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	printf("touch thread create success! \n\r");
	return true;
}


#endif
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h> 

static int touch_fd = -1;

/*详细解释一下 struct input_event 结构体的参数
* struct input_event{

	struct timeval time;//按键时间

	__u16 type; 事件类型 
#define EV_SYN			0x00 同步事件
#define EV_KEY			0x01 键盘事件
#define EV_REL			0x02 相对坐标事件-鼠标
#define EV_ABS			0x03 绝对坐标事件-触摸屏
#define EV_MSC			0x04
#define EV_SW			0x05
#define EV_LED			0x11
#define EV_SND			0x12
#define EV_REP			0x14
#define EV_FF			0x15
#define EV_PWR			0x16
#define EV_FF_STATUS		0x17
#define EV_MAX			0x1f
#define EV_CNT			(EV_MAX+1)
	

	__u16 code; 事件代码
	不同的type，code有不同的含义
	
	type = EV_KEY时，code代表键盘以及鼠标上不同的按键，如code = 9，表示此时event上报的是键盘上数字“9”对应的事件；
														 code = 46，表示此时event上报的是键盘上字母"C“对应的事件
	
	type = EV_REL时，code代表轨迹的类型，指示鼠标移动的方向，如code = 3，表示此时event上报的是鼠标向X轴移动的数据；
															 当code = 4时，表示此时event上报的时鼠标向Y轴移动的数据。
	
	type = EV_ABS时，code代表触摸坐标轴，如code = 0x35 ，表示此时的event上报的就是当前触摸点X轴的坐标；
										   code = 0x36，表示此时的event 上报的就是当前触摸点Y轴的坐标。


	__u16 value;事件的值。
	value: 不同的code，value有不同的含义（tpye是根），举几个列子：
	
	type = EV_KEY，code = 9， value = 0：表示键盘上数字”9“被放开
	
	type = EV_KEY，code = 9， value = 1：表示键盘上数字”9“被按下
	
	type = EV_ABS，code = 0x35, value = 128：表示触摸点的X轴坐标为128
	
	type = EV_ABS，code = 0x36，value = 560；表示触摸点的Y轴坐标为560
	
	type = EV_ABS, code = 0x3a, value = 50: 表示触摸点的压力值为50
	
	type = EV_ABS, code = 0x39,  value = 0: 表示该触摸点的ID = 0，在多点触控式与其他触摸点区分。
};

每次事件的上报之后还需要完成一次同步上报，通常情况下，同步有固定的格式：

type = 0，code = 0， value = 0：表示同步

type = 0，code = 2， value = 0；表示MT同步

对于触摸屏来说，上面的分析已经涵盖了同步、坐标、压力、多点触控区分等信息了，但是还缺少接触触摸屏和离开触摸屏两个信息。其实，这两个信息是必不可少的信息，对于不同的触控IC有不用的实现（tpye、code、value）。我使用的触控IC是汇顶科技的gt1x系列，在驱动中通过如下事件区分接触和离开触摸屏事件：

type = 1，code = 330(0x14a) ， value = 1 ：表示接触触摸屏

type = 1，code = 330 ， value = 0 ：表示离开触摸屏


*/

static void* touch_task(void* arg)
{
	struct input_event ts_buf;

	bool x_ready=false, y_ready=false; 

	while(1)
	{
		bzero(&ts_buf, sizeof(ts_buf));
		read(touch_fd, &ts_buf, sizeof(ts_buf));
		printf("type:0x%x code:0x%x value:%d\n", ts_buf.type, ts_buf.code, ts_buf.value);
	}

	ak_thread_exit();
	return NULL;
}




bool touch_init(int x_scale,int y_scale,bool swap_xy)
{
	touch_fd = open(TOUCH_EVENT_PATH,O_RDONLY);
	if( -1 == touch_fd)
	{
		printf("open %s fail \n\r",TOUCH_EVENT_PATH);
		return false;
	}

	
	touch_y_scale = y_scale;
	touch_pos_swap = swap_xy;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, touch_task,NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
	printf("touch thread create success! \n\r");

}
#endif

