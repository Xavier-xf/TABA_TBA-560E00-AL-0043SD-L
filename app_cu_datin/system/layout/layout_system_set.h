#ifndef LAYOUT_SYSTEM_SET_H
#define LAYOUT_SYSTEM_SET_H

typedef enum
{
	LANGUAGE_SET_FOCUS,
	TIME_SET_FOCUS,
	SYSTEM_SET_RESET_FOCUS,
	TOTAL_SYSTEM_SET_FOCUS
} SYSTEM_SET_MAIN_FOUCUS;

typedef enum
{
	SYSTEM_SET_MAIN_LAYER,
	SYSTEM_SET_LANG_LAYER,
	SYSTEM_SET_RESET_LAYER
} SYSTEM_SET_FOCUS_LAYER;

typedef struct
{

	SYSTEM_SET_MAIN_FOUCUS main;  // 主层的焦点
	SYSTEM_SET_FOCUS_LAYER layer; // 当前焦点所在层
} STR_SystemSetFocus;

typedef enum
{
	SYSTEM_SET_SURE_YES_FOUCUS,
	SYSTEM_SET_SURE_NO_FOUCUS
} SYSTEM_SET_SURE_FOUCUS;

typedef struct
{
	STR_WidgetShow widget_show;
	STR_WidgetDialogBox *dialog_box;
	TouchKeyRegisterCallback key_register;
	STR_SystemSetFocus cur_focus;
	SYSTEM_SET_SURE_FOUCUS sure;
} STR_SystemSetClass;

extern STR_SystemSetClass SystemSetClass;
extern layout layout_system_set;

#endif
