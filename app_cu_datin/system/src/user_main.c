#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <audio_play_api.h>
#include "os_sys_api.h"
#include "file_api.h"
#include "leo_audio_play.h"
#include "iic_touch_key.h"
#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"
#include "gpio_api.h"
#include "uart.h"
#include "watchdog.h"


static void button_down_pro_func(void)
{
	if (whichAway != CONNECTED && SwipingCard.mode != CARD_ADD_CARD_MODE)
	{
		touch_sound_play();
	}
	standby_timer_reset();
}

static void system_info_printf(void)
{
	
    printf("********************************************************\n\r");
    printf("**********    Model:    3760E UI框架	\n\r");
    printf("**********    author:   Leo	\n\r");
    printf("**********    date:     2021-07-02	\n\r");
    printf("**********    ver:      %s	\n\r", SYSTEM_VERSION);
    printf("********************************************************\n\r");	
}

static void system_script_init(void)
{
    // 更严格的检查，确保模块确实未加载
    FILE *fp;
    char line[256];
    int fb_loaded = 0, gui_loaded = 0;
    
    // 检查 ak_fb 模块是否已加载
    fp = popen("lsmod | grep ak_fb", "r");
    if (fp != NULL) {
        if (fgets(line, sizeof(line), fp) != NULL) {
            fb_loaded = 1;
        }
        pclose(fp);
    }
    
    // 检查 ak_gui 模块是否已加载  
    fp = popen("lsmod | grep ak_gui", "r");
    if (fp != NULL) {
        if (fgets(line, sizeof(line), fp) != NULL) {
            gui_loaded = 1;
        }
        pclose(fp);
    }
    
    printf("Module status: ak_fb=%d, ak_gui=%d\n", fb_loaded, gui_loaded);
    
    if (!fb_loaded) {
        system("insmod /usr/modules/ak_fb.ko");
    } else {
        printf("ak_fb.ko already loaded\n");
    }
    
    if (!gui_loaded) {
        system("insmod /usr/modules/ak_gui.ko");
    } else {
        printf("ak_gui.ko already loaded\n");
    }
    
    system("hwclock -s"); // 从硬件时钟设置系统时间
}



/**
 *
 * 系统产品的初始化
 *
 */
static void system_init(void)
{
	system_info_printf();
	vector screen = {480, 272};
	resource font_resource = resource_get(ROM_R_FONT_BYEKAN_TTF);
	ui_init(&screen, &font_resource, button_down_pro_func, 100, 100, 0, false);
	media_file_list_init();
	leo_audio_play_init();
	os_evnet_init();
	iic_touch_key_init();		// 按键板
	intercom_uart_init();		// 
	swiping_card_uart_init();	// 刷卡
	reset_button_init();		// 重置按钮

	if(strcmp(VERSION_STATE, "dev") != 0)
	{
		watchdog_init();		// 看门狗
	}

	standby_timer_open(60000, goto_layout_standby);
	
}

int main(int arc, char **argv)
{
	system_script_init();

	system_init();

	os_start(&layout_logo);

	return 0;
}