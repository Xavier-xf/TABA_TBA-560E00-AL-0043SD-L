#ifndef OS_SYS_API_H
#define OS_SYS_API_H
#include <stdbool.h>

typedef struct
{
    void(*init)(void);

    void(*enter)(void);

    void(*quit)(void);

    void(*timer)(void);
}layout;



unsigned long long os_get_ms(void);
unsigned long long os_get_us(void);

/********************
页面跳转接口
********************/
void os_layout_goto(const layout* layout);

/********************
投递按键事件发送
********************/
bool touch_key_event(int key_index,int key_state);

/***********
初始化主任务
************/
void os_evnet_init(void);


/******************
开启主任务工作
******************/
void os_start(const layout* layout);

/*************************************************************************************
开启定时待机
	timeout:超时待机，单位毫秒，<= 0，沿用上次的事件，
	timeout_callback:超时处理函数,timeout_callback = NULL,使用上一次的超时处理函数
**************************************************************************************/
bool standby_timer_open(int timeout,void(*timeout_callback)(void));


/********************
关闭超时处理。
********************/
bool standby_timer_close(void);


/********************
待机超时重置
********************/
bool standby_timer_reset(void);

bool timed_event_check_init(void(*timed_check_func)(void));
void timed_event_check_start(void);
void timed_event_check_stop(void);

/********************
投递记录事件发送
********************/
bool record_finish_event(char type,bool is_finish);
/****************************
记录 事件处理函数的注册
*****************************/
void record_event_register(void(*callback)(char,bool));

/********************
投递interphone事件
********************/
bool interphone_event(int arg1,int arg2);
/****************************
interphone 事件处理函数的注册
*****************************/
void interphone_event_register(void(*callback)(int,int));


/*******************
call事件处理函数注册
********************/
void door_call_event_register(void(*door1_func)(void),void(*door2_func)(void));


/********************
hook事件处理函数注册
********************/
void hook_event_register(void(*hook_up)(void),void(*hook_down)(void));



/*******************
警报事件处理函数注册
********************/
void alarm_event_register(void(*func)(int));

#endif

