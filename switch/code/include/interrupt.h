#ifndef _INTERRUPT_H
#define _INTERRUPT_H
#include ".\include\Type.h"


extern uint16_t tick_1msec;
extern void Timer0_Interrupt_Enable(uint16_t uSecTime);
extern void delay_ms(uint16_t time);

/*******************************TIMER***********************************/

#define TID_10mSEC              1
#define TID_20mSEC              2
#define TID_30mSEC              3
#define TID_40mSEC              4
#define TID_50mSEC              5
#define TID_60mSEC              6
#define TID_70mSEC              7
#define TID_80mSEC              8
#define TID_90mSEC              9
#define TID_100mSEC             10
#define TID_200mSEC             20
#define TID_300mSEC             30
#define TID_800mSEC             80
#define TID_1SEC				100
#define TID_2SEC				200

typedef struct
{
    uint16_t wCount;
    void (*func)(void);
} TIMER;

enum SetTimerID
{
    TIMER0, 
	TIMER1, 
	TIMER2,
	TIMER3,
    TIMER_TOTAL
};

extern TIMER Timer[TIMER_TOTAL];
extern uint32_t cpu_count;

extern void set_timer_init(void);
extern void set_timer(uint8_t id, uint16_t Interval, void (*func)(void));
extern void kill_timer(uint8_t id);
extern void sys_timer_check();
extern void sys_timer_process(enum SetTimerID id);

/***********************************************************************/






#endif
