#ifndef LAYOUT_BASE_H
#define LAYOUT_BASE_H

typedef enum
{
	WIDGET_HIDE,
	WIDGET_SHOW
} BLINK;

typedef void (*WidgetShowCallback)(void);
typedef void (*TouchKeyRegisterCallback)(void);
typedef void (*DataInitCallback)(void);

typedef struct
{
	WidgetShowCallback icon;
	WidgetShowCallback font;
	WidgetShowCallback focus;
	WidgetShowCallback dialog_box;
	WidgetShowCallback progress_bar;
} STR_WidgetShow;

typedef enum
{
	NONE_LONG_PRESS,
	DIAL_STAR_LONG_PRESS,

} KEY_LONG_PRESS_GOTO_LAYOUT;

/* 重大修改.功能修复.局部修复			dev/alpha/beta ->开发/内部/外部测试版 */
/* 主版本号.次版本号.修正版本号.日期版本号_阶段版本号*/
#define VERSION_STATE "dev"
#define VERSION_NUMBER "v2.1.2"
#define SYSTEM_VERSION VERSION_NUMBER "_" VERSION_STATE
/*
v2.1.8_beta1
1. 使字体/语言切换更简单;

 */

#include "os_sys_api.h"
#include "rom.h"
#include "user_data.h"
#include "language.h"
#include "intercom.h"
#include "swiping_card.h"
#include "reset_btn.h"

#include "layout_standby.h"
#include "layout_dial.h"
#include "layout_calling.h"
#include "layout_password.h"
#include "layout_settings.h"
#include "layout_system_set.h"
#include "layout_home_id_set.h"
#include "layout_card_manage.h"
#include "layout_time_set.h"
#include "layout_card_number.h"
#include "layout_swiping_card.h"
#include "layout_info.h"
#include "layout_OutPUT.h"
#include <pthread.h>
#include <unistd.h>

#define COLOR_BLUE_LOG "\033[0;1;34m"
#define COLOR_YELLOW_LOG "\033[0;1;33m"
#define COLOR_GREEN_LOG "\033[0;1;32m"
#define COLOR_RED_LOG "\033[0;1;31m"
#define COLOR_WHITE_LOG "\033[0;1m"
#define COLOR_NONE_LOG "\033[0m"

#define LOG_BLUE(format, ...) printf(COLOR_BLUE_LOG "[%s:%04u]  " format COLOR_NONE_LOG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_YELLOW(format, ...) printf(COLOR_YELLOW_LOG "[%s:%04u]  " format COLOR_NONE_LOG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_GREEN(format, ...) printf(COLOR_GREEN_LOG "[%s:%04u]  " format COLOR_NONE_LOG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_RED(format, ...) printf(COLOR_RED_LOG "[%s:%04u]  " format COLOR_NONE_LOG, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOG_WHITE(format, ...) printf(COLOR_WHITE_LOG "[%s:%04u]  " format COLOR_NONE_LOG, __FUNCTION__, __LINE__, ##__VA_ARGS__)

typedef enum
{
	FROM_NONE = 0x00,
	FROM_OUTDOOR, // 从室外打开
	FROM_INDOOR,  // 从室内打开
	CONNECTED
} WhereToConnect;

typedef enum
{
	NONE = 0x00,
	RESET_PASSWORD, // pcb按下重置按钮 重置 密码
	RESET_SYSTEM,	// 系统设置界面的[reset] 恢复出厂设置
	RESET_FINISH
} INFO;

extern INFO message;
extern WhereToConnect whichAway;
extern pthread_mutex_t card_mutex;

extern void taba_btn_display(void);
extern void gregorian_to_jalali(long gy, long gm, long gd, long out[]);
extern void jalali_to_gregorian(long jy, long jm, long jd, long out[]);

extern void date_set(struct ak_date *date);
extern void layout_init(void);
extern void widget_blink(struct ak_timeval *start_cpu_time,
						 long frequency_ms,
						 BLINK *widget_blink_state,
						 void (*widget_blink)(BLINK));

extern layout layout_logo;
extern layout layout_info;
extern layout layout_dial;

extern void bg_img_init(void);

extern KEY_LONG_PRESS_GOTO_LAYOUT key_long_press_goto_layout;

void font_file_reload(void);

#endif
