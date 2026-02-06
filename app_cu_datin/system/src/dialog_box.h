#ifndef LAYOUT_DIALOG_BOX_H
#define LAYOUT_DIALOG_BOX_H
#include "ui_api.h"

typedef struct
{
	position pos;
	unsigned int color;
	unsigned char index;
	unsigned char max_index;
} STR_Cursor;

typedef struct
{
	text text1;
	text text2;
	text text3;
	char *string1;
	char *string2;
	char *string3;
} STR_DialogFont;

typedef struct
{
	icon box;			 // 图标
	STR_DialogFont font; // 字体大小
	STR_Cursor cursor;	 // 光标
} STR_WidgetDialogBox;	 // 输入框

extern void widget_dialog_box_init(STR_WidgetDialogBox *widget_dialog_box,
								   icon *box,
								   STR_DialogFont *font,
								   STR_Cursor *cursor);

extern void widget_dialog_box_display(STR_WidgetDialogBox *widget_dialog_box);

#endif
