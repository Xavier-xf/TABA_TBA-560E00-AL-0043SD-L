#include "layout_base.h"
#include "signal.h"

INFO message = NONE;                  // 跳转至信息界面时传递的状态信息
WhereToConnect whichAway = FROM_NONE; // 连接来源标识（NONE/门外呼叫/室内呼叫等）
pthread_mutex_t card_mutex;           // 卡片操作互斥锁，保护卡片相关临界资源

/*********************************************************************************************************
 * 函 数 名 : layout_init
 * 功能说明 : 初始化所有布局模块及卡片操作互斥锁
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 依次初始化信息、拨号、密码、楼层设置、系统设置、卡片管理等布局，初始化卡片互斥锁
 *********************************************************************************************************/
void layout_init(void)
{
    layout_info.init();
    layout_dial.init();
    layout_password.init();
    layout_home_id_set.init();
    layout_system_set.init();
    layout_card_manage.init();
    layout_card_number.init();
    layout_standby.init();
    layout_OutPUT.init();

    pthread_mutex_init(&card_mutex, NULL);
}

/*********************************************************************************************************
 * 函 数 名 : bg_img_init
 * 功能说明 : 初始化全局背景图片
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 背景图片位置为全屏(0,0)，尺寸480x272，资源为TB背景JPG，居中显示
 *********************************************************************************************************/
void bg_img_init(void)
{
    position pos = {{0, 0}, {480, 272}};

    resource res = resource_get(ROM_R_IMG_TB_BG_JPG);
    bg_resource_load(&res, &pos, NULL, CENTER_MIDDLE);
}

/*********************************************************************************************************
 * 函 数 名 : gregorian_to_jalali
 * 功能说明 : 将公历日期转换为波斯历（贾拉利历）日期
 * 形    参 : gy：公历年；gm：公历月；gd：公历日；out：输出数组[0]=波斯年, [1]=波斯月, [2]=波斯日
 * 返 回 值 : 无
 * 备    注 : 实现公历到波斯历的日期转换算法，适配波斯语界面的日期显示
 *********************************************************************************************************/
void gregorian_to_jalali(long gy, long gm, long gd, long out[])
{
    long days;
    {
        long gy2 = (gm > 2) ? (gy + 1) : gy;
        long g_d_m[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
        days = 355666 + (365 * gy) + ((int)((gy2 + 3) / 4)) - ((int)((gy2 + 99) / 100)) + ((int)((gy2 + 399) / 400)) + gd + g_d_m[gm - 1];
    }
    long jy = -1595 + (33 * ((int)(days / 12053)));
    days %= 12053;
    jy += 4 * ((int)(days / 1461));
    days %= 1461;
    if (days > 365)
    {
        jy += (int)((days - 1) / 365);
        days = (days - 1) % 365;
    }
    out[0] = jy;
    if (days < 186)
    {
        out[1] /*jm*/ = 1 + (int)(days / 31);
        out[2] /*jd*/ = 1 + (days % 31);
    }
    else
    {
        out[1] /*jm*/ = 7 + (int)((days - 186) / 30);
        out[2] /*jd*/ = 1 + ((days - 186) % 30);
    }
}

/*********************************************************************************************************
 * 函 数 名 : jalali_to_gregorian
 * 功能说明 : 将波斯历（贾拉利历）日期转换为公历日期
 * 形    参 : jy：波斯年；jm：波斯月；jd：波斯日；out：输出数组[0]=公历年, [1]=公历月, [2]=公历日
 * 返 回 值 : 无
 * 备    注 : 实现波斯历到公历的日期转换算法，处理闰年等特殊情况
 *********************************************************************************************************/
void jalali_to_gregorian(long jy, long jm, long jd, long out[])
{
    jy += 1595;
    long days = -355668 + (365 * jy) + (((int)(jy / 33)) * 8) + ((int)(((jy % 33) + 3) / 4)) + jd + ((jm < 7) ? (jm - 1) * 31 : ((jm - 7) * 30) + 186);
    long gy = 400 * ((int)(days / 146097));
    days %= 146097;
    if (days > 36524)
    {
        gy += 100 * ((int)(--days / 36524));
        days %= 36524;
        if (days >= 365)
            days++;
    }
    gy += 4 * ((int)(days / 1461));
    days %= 1461;
    if (days > 365)
    {
        gy += (int)((days - 1) / 365);
        days = (days - 1) % 365;
    }
    long gd = days + 1;
    long gm;
    {
        long sal_a[13] = {0, 31, ((gy % 4 == 0 && gy % 100 != 0) || (gy % 400 == 0)) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        for (gm = 0; gm < 13 && gd > sal_a[gm]; gm++)
            gd -= sal_a[gm];
    }
    out[0] = gy;
    out[1] = gm;
    out[2] = gd;
}

/*********************************************************************************************************
 * 函 数 名 : executive_command
 * 功能说明 : 执行系统shell命令
 * 形    参 : cmd：要执行的命令字符串
 * 返 回 值 : 成功返回0，失败返回-1
 * 备    注 : 通过管道方式执行命令，使用popen/pclose实现，用于系统日期设置等操作
 *********************************************************************************************************/
static int executive_command(char *cmd)
{
    FILE *pp = popen(cmd, "w"); // 建立管道
    if (!pp)
    {
        return -1;
    }
    pclose(pp);
    return 0;
}

/*********************************************************************************************************
 * 函 数 名 : date_set
 * 功能说明 : 设置系统日期和时间，并同步到硬件时钟
 * 形    参 : date：包含年月日时分秒的日期时间结构体指针
 * 返 回 值 : 无
 * 备    注 : 先通过date命令设置系统时间，再通过hwclock -w同步到硬件时钟
 *********************************************************************************************************/
void date_set(struct ak_date *date)
{
    char date_param[64] = {0};
    char hwclock[16] = "hwclock -w";
    sprintf(date_param, "date -s \"%04d-%02d-%02d %02d:%02d:%02d\"",
            date->year,
            date->month + 1,
            date->day + 1,
            date->hour,
            date->minute,
            date->second);

    executive_command(date_param);
    executive_command(hwclock);
}

/*********************************************************************************
@Function:    部件闪烁
@Input:       start_cpu_time: 部件开始显示或隐藏时读取cpu的时间
              frequency_ms: 闪烁的频率(ms)
              widget_blink_state: 闪烁的状态(部件显示或部件隐藏)
              (*widget_blink)(BLINK): 部件闪烁的回调函数
@Output:      (void)
@Description: 此函数用于闪烁的部件(字体、图标等)
@Author:      kevin
@Date:        2021-10-29
*********************************************************************************/
/*********************************************************************************************************
* 函 数 名 : widget_blink
* 功能说明 : 通用部件闪烁控制函数
* 形    参 : start_cpu_time：闪烁起始时间戳；frequency_ms：闪烁频率(毫秒)；
              widget_blink_state：闪烁状态指针（显示/隐藏）；widget_blink：闪烁回调函数
* 返 回 值 : 无
* 备    注 : 达到频率阈值时切换闪烁状态，并调用回调函数更新部件显示/隐藏状态
*********************************************************************************************************/
void widget_blink(struct ak_timeval *start_cpu_time,
                  long frequency_ms,
                  BLINK *widget_blink_state,
                  void (*widget_blink)(BLINK))
{

    struct ak_timeval cur_cpu_time;

    ak_get_ostime(&cur_cpu_time);
    if (ak_diff_ms_time(&cur_cpu_time, start_cpu_time) >= frequency_ms)
    {
        ak_get_ostime(start_cpu_time);
        if (*widget_blink_state == WIDGET_SHOW)
        {
            *widget_blink_state = WIDGET_HIDE;
            widget_blink(*widget_blink_state);
        }
        else
        {
            *widget_blink_state = WIDGET_SHOW;
            widget_blink(*widget_blink_state);
        }
    }
}

/*************************************************************************
 *@brief   现实taba
 *@param   x,y
 *@date 2022-11-03 10:24
 *@author  xiaoele
 **************************************************************************/
/*********************************************************************************************************
 * 函 数 名 : taba_btn_display
 * 功能说明 : 显示TABA按钮图标
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 图标位置(0,193)，尺寸180x79，资源为TABA图标PNG
 *********************************************************************************************************/
void taba_btn_display(void)
{
    icon star;
    position star_pos = {{0, 193}, {180, 79}};
    resource star_res = resource_get(ROM_R_IMG_TABA_ICON_PNG);
    icon_init(&star, &star_pos, &star_res);

    icon_display(&star);

    position taba_text_pos = {{120, 219}, {112, 44}};
    position electronics_text_pos = {{228, 226}, {150, 30}};

    if (language_get() == language_persian)
    {
        taba_text_pos = (position){{236, 219}, {86, 44}};
        electronics_text_pos = (position){{146, 226}, {126, 30}};
    }

    text taba_text;
    text_init(&taba_text, &taba_text_pos, 36);
    taba_text.font_color = 0xFFFFC800;
    text_display(&taba_text, font_str(STR_LOGO_TABA));

    text electronics_text;
    text_init(&electronics_text, &electronics_text_pos, 20);
    electronics_text.font_color = 0xFFFFC800;
    text_display(&electronics_text, font_str(STR_LOGO_ELECTRONICS));
}

KEY_LONG_PRESS_GOTO_LAYOUT key_long_press_goto_layout = NONE_LONG_PRESS; // 按键长按跳转布局标识

extern void font_file_load(const resource *resource);
/*************************************************************************
 * @brief  重新加载字库文件
 * @date   2022-11-03 10:24
 * @author xiaoele
 **************************************************************************/
/*********************************************************************************************************
 * 函 数 名 : font_file_reload
 * 功能说明 : 根据当前语种重新加载对应的字库文件
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 英语加载BYEKAN字体，波斯语加载BNAZANIN字体，加载前打印语种日志
 *********************************************************************************************************/
void font_file_reload(void)
{
    /* 根据语种选择字库 */
    resource font_resource;
    if (language_get() == language_english)
    {
        LOG_BLUE("is in English \n");
        font_resource.id = ROM_R_FONT_BYEKAN_TTF;
        font_resource.size = ROM_R_FONT_BYEKAN_TTF_SIZE;
    }
    else
    {
        LOG_BLUE("is in Persian \n");
        font_resource.id = ROM_R_FONT_BNAZANIN_TTF;
        font_resource.size = ROM_R_FONT_BNAZANIN_TTF_SIZE;
    }
    font_file_load(&font_resource);
}
