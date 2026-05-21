#ifndef LAYOUT_OUTPUT_H
#define LAYOUT_OUTPUT_H
#include "ui_api.h"
#include "dialog_box.h"
#include "layout_base.h"

#define OUTPUT_ADMIN_ID_SET 249
typedef enum
{
	OUPUT_OUT_FOCUS,
	OUPUT_UNIT_FOCUS,
	TOTAL_OUTPUT_FOCUS
} OUTPUT_MAIN_FOUCUS;

typedef enum
{
	ENUM_OUTPUT_DIALOG_BOX1 = 0,
	ENUM_OUTPUT_DIALOG_BOX2,
	ENUM_OUTPUT_DIALOG_BOX_TOTAL
} ENUM_OUTPUT_DIALOG_BOX;

typedef enum
{
	OUTPUT_STATUS_NONE,
	OUTPUT_STATUS_SUCCESS,
	OUTPUT_STATUS_ERROR
} OUTPUT_STATUS;

typedef struct
{
	OUTPUT_MAIN_FOUCUS main;
	ENUM_OUTPUT_DIALOG_BOX dialog_box;
	OUTPUT_STATUS status;
} STR_OUTPUT_FOUCUS;

typedef struct
{
	unsigned char brancher_seq[3]; // 分支器序号
	unsigned char admin_id[4];	   // 管理员ID
	char output_status_count;	   // 状态计数器
} STR_DeviceAuthInfo;
typedef struct
{
	STR_WidgetShow widget_show;
	STR_WidgetDialogBox **dialog_box;
	TouchKeyRegisterCallback key_register;
	STR_OUTPUT_FOUCUS cur_focus;
	STR_DeviceAuthInfo auth_info;
} STR_OutPUTClass;

extern STR_OutPUTClass OutPUTClass;
extern layout layout_OutPUT;

#endif