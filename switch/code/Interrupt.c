#include ".\include\interrupt.h"
#include ".\include\REG_MG82F6D64.H"
#include "string.h"
#include ".\include\msg_event.h"
#include ".\include\kevin_function.h"




/*==============================================================================
author: Dary.
date : 	2020年3月13日19:16:26
func :	Timer
==============================================================================*/

uint16_t     timer0_cnt;

void Timer0_Interrupt_Enable(uint16_t uSecTime)
{
	TMOD |= 0x01;  
	timer0_cnt = 65536 - uSecTime;	 

	TH0 = (uint8_t)(timer0_cnt >> 8);
	TL0 = (uint8_t)(timer0_cnt);

	ET0 = 1;	//开启定时器
	TR0 = 1;	//开启中断
}

uint16_t tick_1msec;
uint16_t tick_10msec;

void TIMER0_Int(void) interrupt 1 //定时器0对应中断1
{ 
    uint8_t sfrpi_backup;
	static uint8_t count10ms = 0;

    sfrpi_backup = SFRPI;
	TH0 = (uint8_t)(timer0_cnt >> 8);
	TL0 = (uint8_t)(timer0_cnt);

	tick_1msec = 1;

	if(tick_10msec==0)
	{
		if(++count10ms >= 10)
		{	
			count10ms = 0;
			tick_10msec = 1;
		}
	}
	
    SFRPI = sfrpi_backup;
}


TIMER Timer[TIMER_TOTAL];	

void set_timer_init(void)
{   
	Timer0_Interrupt_Enable(1000);//开启1ms定时器
    memset(Timer, 0, sizeof(Timer[TIMER_TOTAL]));
}

void set_timer(uint8_t id, uint16_t Interval, void (*func)(void))
{
    Timer[id].wCount = Interval;
    Timer[id].func = func;
}

void kill_timer(uint8_t id)
{
	Timer[id].wCount = 0;
}


uint32_t cpu_count = 1;
static void system_cpu_count(void){
	if(tick_1msec){
		tick_1msec = 0;
		if(cpu_count < 4294967295){
			cpu_count++;
		}else{
			cpu_count = 1;
		}
	}
}

/***********************************************************************
@Function:    定时器事件检测
@Input:       (void)    
@Output:      (void)
@Description: 定时时间到后，将事件存入系统事件队列
@Author:      kevin
@Date:        2021-08-24
***********************************************************************/
void sys_timer_check(void)
{ 
	uint8_t id;
	static uint8_t t100m = 0;
	Event event = {0};

	system_cpu_count();
	
	if(!tick_10msec)
		return;
	

	tick_10msec = 0;
	if(++t100m > 10)
	{
		t100m = 0;
	 	WDTCR = (0xe6 | 0x10);
	}

	for(id = TIMER0; id < TIMER_TOTAL; id++) 
	{        
        if(Timer[id].wCount != 0) 
		{
            Timer[id].wCount--;
            if(Timer[id].wCount == 0) 
			{
				event.type = EVENT_TYPE_TIMER;
				event.arg1 = id;
				msg_event_post(&event);
            }
        }
    } 
}

void sys_timer_process(enum SetTimerID id){

	Timer[id].func();

}

void delay_ms(uint16_t time)
{
	uint16_t old_time = tick_1msec;

	while(tick_1msec != old_time + time);
}









