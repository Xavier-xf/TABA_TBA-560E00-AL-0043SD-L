#ifndef LAYOUT_CALLING_H
#define LAYOUT_CALLING_H




typedef enum{
	CALL_USER = 0x01,
	CALL_GUARD,
}CALL_OBJECT;

typedef enum{
	NORMAL = 0x00,
	ANSWER,
	NO_ANSWER,
}ANSWER_STATUS;


typedef struct{
	STR_WidgetShow widget_show;
	CALL_OBJECT call_object;
	ANSWER_STATUS  answer_status;
}STR_CallingClass;


extern STR_CallingClass CallingClass;
extern layout layout_calling;










#endif


