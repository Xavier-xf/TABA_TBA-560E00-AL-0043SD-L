#ifndef LAYOUT_TIME_SET_H
#define LAYOUT_TIME_SET_H
#include "ak_common.h"


typedef enum{
	YEAR_SET_FOCUS,
	MONTH_SET_FOCUS,
	DAY_SET_FOCUS,
	HOUR_SET_FOCUS,
	MINUTE_SET_FOCUS,
	TOTAL_TIME_SET_FOCUS
}TIME_SET_MAIN_FOUCUS;

typedef enum{
	TIME_SET_MAIN_LAYER,
	TIME_SET_ADJUST_LAYER,
}TIME_SET_FOCUS_LAYER;

typedef enum{
	TIME_ADD,
	TIME_SUB
}TIME_SET;


typedef struct{
	TIME_SET_MAIN_FOUCUS main;			//主层的焦点
	TIME_SET_FOCUS_LAYER layer;       	//当前焦点所在层
}STR_TimeSetFocus;



typedef struct{
	STR_WidgetShow widget_show;
	TouchKeyRegisterCallback key_register;
	DataInitCallback data_init;
	STR_TimeSetFocus cur_focus;
	struct ak_date date;
	int **date_array;
}STR_TimeSetClass;


extern STR_TimeSetClass TimeSetClass;
extern layout layout_time_set;





#endif





