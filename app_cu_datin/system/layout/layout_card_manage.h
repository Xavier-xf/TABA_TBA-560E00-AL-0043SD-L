#ifndef LAYOUT_CARD_MANAGE_H
#define LAYOUT_CARD_MANAGE_H

#include <stdbool.h>

typedef enum
{
	UNIT_FOCUS,
	TAG_FOCUS,
	ERASE_FOCUS,
	SAVE_FOCUS,
	TOTAL_CARD_MANAGE_FOCUS
} CARD_MANAGE_MAIN_FOUCUS;

typedef enum
{
	CARD_MANAGE_MAIN_LAYER,
	CARD_MANAGE_MAIN_LAYER_CONFIRM
} CARD_MANAGE_FOUCUS_LAYER;
typedef enum
{
	CARD_MANAGE_STATUS_NONE,
	CARD_MANAGE_STATUS_ERROR,
	CARD_MANAGE_STATUS_SUCCESS,
	CARD_MANAGE_STATUS_DELETE_CARD,
	CARD_MANAGE_STATUS_SAVE_CARD,
	CARD_MANAGE_STATUS_TAG_ERROR,
	TOTAL_CARD_MANAGE_STATUS
} CARD_MANAGE_STATUS;

typedef struct
{
	CARD_MANAGE_MAIN_FOUCUS main;
	CARD_MANAGE_FOUCUS_LAYER layer;
	CARD_MANAGE_STATUS status;
} STR_CARD_MANAGE_FOUCUS;

typedef struct
{
	unsigned char home_id[4];	   // 房间/家庭ID
	char card_number_status_count; // 卡号状态计数
	int room_card_num;			   // 房间卡号
} STR_ROOM_CARD_INFO;			   // 核心命名：STR_ + 房间 + 卡 + 信息
typedef struct
{
	STR_WidgetShow widget_show;
	STR_WidgetDialogBox *dialog_box;
	TouchKeyRegisterCallback key_register;
	STR_CARD_MANAGE_FOUCUS cur_focus;
	STR_ROOM_CARD_INFO room_card_info;
} STR_CardManage;

// ========== 焦点-坐标映射表相关定义 ==========
typedef struct
{
	CARD_MANAGE_MAIN_FOUCUS focus_type;
	int x; // 显示x坐标
	int y; // 显示y坐标
} FocusPosMap;

extern STR_CardManage CardManageClass;
extern layout layout_card_manage;
extern bool card_manage_fill_tag_by_card_id(char *card_id);

#endif
