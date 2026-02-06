#ifndef LAYOUT_DIAL_H
#define LAYOUT_DIAL_H
#include "ui_api.h"
#include "dialog_box.h"
#include "layout_base.h"

typedef enum
{
	DIAL_NUMBER_DIALOG_BOX,
	DIAL_TOTAL_DIALOG_BOX
} ENUM_DIAL_DIALOG_BOX;

typedef struct
{
	STR_WidgetShow widget_show;
	STR_WidgetDialogBox **dialog_box;
	TouchKeyRegisterCallback key_register;
	unsigned char home_id[4];
} STR_DialClass;

extern STR_DialClass DialClass;

extern void dial_home_id_init(void);

extern void dial_input_add_number(unsigned char number);

#endif
