#ifndef LAYOUT_STANDBY_H
#define LAYOUT_STANDBY_H



typedef enum{
	WELOCOME_BG,
	RING_BG,
	GUAID_BG,
	CLOCK_BG,
	STANDBG_TOTAL_BG
}STANDBY_BG_IMG;



typedef struct{
	WidgetShowCallback bg_show;
	TouchKeyRegisterCallback key_register;
	DataInitCallback data_init;
	STANDBY_BG_IMG bg_index;
}STR_StandbyClass;


extern STR_StandbyClass StandbyClass;
extern layout layout_standby;

extern void goto_layout_standby(void);










#endif







