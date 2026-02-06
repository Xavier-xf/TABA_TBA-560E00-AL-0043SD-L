#include "os_sys_api.h"
#include "ui_api.h"
#include "rom.h"
#include "layout_base.h"

#define INIT_UNLOCK_PASSWORD "1 2 3 4 5 6"
#define INIT_SYSTEM_PASSWORD "1 2 3 4 5 6"
#define USER_CONF_FILE "/app/data/data.ini"
#define FACTORY_CONF_FILE "/app/data/factory.ini"

#define SCREENWIDTH (480)
#define SCREENHIGHT (270)

static int info_delay_count = 0;
static int reset_progress = 0; // 进度百分比
static int reset_stage = 0;    // 重置阶段
static int is_resetting = 0;   // 是否正在重置系统
static pthread_mutex_t mutex;

// 工厂重置的各个阶段
typedef enum
{
    RESET_STAGE_IDLE = 0,    // 空闲
    RESET_STAGE_CHECK_FILES, // 检查文件
    RESET_STAGE_COPY_CONFIG, // 复制配置文件
    RESET_STAGE_CLEAR_CARDS, // 清除卡数据
    RESET_STAGE_SAVE_DATA,   // 保存数据
    RESET_STAGE_SYNC,        // 同步文件系统
    RESET_STAGE_COMPLETE,    // 完成
    RESET_STAGE_MAX
} ResetStage;

static void resetPassWord(void)
{
    pthread_mutex_lock(&mutex);
    set_string_conf(UNLOCK_PASSWORD, INIT_UNLOCK_PASSWORD);
    set_string_conf(SET_PASSWORD, INIT_SYSTEM_PASSWORD);
    pthread_mutex_unlock(&mutex);
}

static void factorySettingWithProgress(void)
{
    switch (reset_stage)
    {
    case RESET_STAGE_IDLE:
        reset_stage = RESET_STAGE_CHECK_FILES;
        reset_progress = 0;
        break;

    case RESET_STAGE_CHECK_FILES:
        // 检查文件是否存在
        if (access(USER_CONF_FILE, F_OK) != 0)
        {
            LOG_WHITE("file" USER_CONF_FILE " not exist!\n");
        }
        if (access(FACTORY_CONF_FILE, F_OK) != 0)
        {
            LOG_WHITE("file" FACTORY_CONF_FILE " not exist!\n");
        }
        reset_progress = 10;
        reset_stage = RESET_STAGE_COPY_CONFIG;
        break;

    case RESET_STAGE_COPY_CONFIG:
        // 删除用户配置文件并复制工厂配置文件
        pthread_mutex_lock(&mutex);
        system("rm -rf " USER_CONF_FILE);
        system("cp " FACTORY_CONF_FILE " " USER_CONF_FILE);
        pthread_mutex_unlock(&mutex);
        reset_progress = 30;
        reset_stage = RESET_STAGE_CLEAR_CARDS;
        break;

    case RESET_STAGE_CLEAR_CARDS:
        // 复位时删除已添加的卡
        pthread_mutex_lock(&mutex);
        int index = get_int_conf(UNIT_NUMBER_INDEX);
        for (int i = 0; i < index; i++)
        {
            UserData.unit_number[i] = -1;
        }
        deleteAllCard();
        card_id_data_save();
        user_data_save();
        set_int_conf(UNIT_NUMBER_INDEX, 0);
        pthread_mutex_unlock(&mutex);
        reset_progress = 60;
        reset_stage = RESET_STAGE_SAVE_DATA;
        break;

    case RESET_STAGE_SAVE_DATA:
        // 初始化用户配置
        user_configure_init();
        reset_progress = 80;
        reset_stage = RESET_STAGE_SYNC;
        break;

    case RESET_STAGE_SYNC:
        // 同步文件系统
        system("sync");
        reset_progress = 90;
        reset_stage = RESET_STAGE_COMPLETE;
        break;

    case RESET_STAGE_COMPLETE:
        // 等待显示完成，然后重启
        reset_progress = 100;
        break;

    default:
        break;
    }
}

static void clearTextArea(position *pos)
{
    // 清空文本区域，避免重影
    gui_erase(pos, 0x00);
}

static void restoringFactoryDisplay(void)
{
    position pos = {{0, SCREENHIGHT * 0.2}, {SCREENWIDTH, 50}};

    // 1. 清空文本区域
    clearTextArea(&pos);

    // 2. 初始化并显示文本
    text fac_text;
    text_init(&fac_text, &pos, 20);
    fac_text.align = CENTER_MIDDLE;

    // 显示带百分比的文本
    char display_str[100];
    snprintf(display_str, sizeof(display_str), "%s %d%%",
             (char *)font_str(STR_FACTORY_SETTING), reset_progress);
    text_display(&fac_text, display_str);
}

static void resetingPasswordDisplay(void)
{
    position pos = {{0, SCREENHIGHT * 0.2}, {SCREENWIDTH, 50}};

    // 清空文本区域
    clearTextArea(&pos);

    text pwd_text;
    text_init(&pwd_text, &pos, 20);
    pwd_text.align = CENTER_MIDDLE;
    text_display(&pwd_text, font_str(STR_RESETTING_PASSWORD));
}

static void completeDisplay(void)
{
    position pos = {{0, SCREENHIGHT * 0.4}, {SCREENWIDTH, 50}};

    // 清空文本区域
    clearTextArea(&pos);

    text comp_text;
    text_init(&comp_text, &pos, 30);
    comp_text.align = CENTER_MIDDLE;
    text_display(&comp_text, font_str(STR_COMPLETE));
}

static void clearScreen(void)
{
    position pos = {{0, 0}, {SCREENWIDTH, SCREENHIGHT}};
    gui_erase(&pos, 0x00);
}

static void layout_info_init(void)
{
    info_delay_count = 0;
    reset_progress = 0;
    reset_stage = RESET_STAGE_IDLE;
    is_resetting = 0;
    message = NONE;
    pthread_mutex_init(&mutex, NULL);
}

static void layout_info_enter(void)
{
    LOG_WHITE("enter %d\n", message);

    clearScreen(); // 清空所有UI

    // 显示底部logo
    taba_btn_display();

    switch (message)
    {
    case RESET_PASSWORD:
        resetingPasswordDisplay();
        resetPassWord();
        message = RESET_FINISH;
        break;

    case RESET_SYSTEM:
        is_resetting = 1;
        reset_progress = 0;
        reset_stage = RESET_STAGE_IDLE;
        break;

    default:
        break;
    }
}

static void layout_info_quit(void)
{
    message = NONE;
    info_delay_count = 0;
    reset_progress = 0;
    reset_stage = RESET_STAGE_IDLE;
    is_resetting = 0;
}

static void layout_info_timer(void)
{
    if (is_resetting)
    {
        // 工厂重置进行中，更新进度
        if (reset_stage == RESET_STAGE_COMPLETE)
        {
            // 完成阶段，延迟显示100%
            info_delay_count++;
        }

        // 执行当前阶段的操作
        factorySettingWithProgress();

        // 更新显示
        restoringFactoryDisplay();

        // 确保logo保持显示
        taba_btn_display();

        if (reset_stage == RESET_STAGE_COMPLETE && info_delay_count > 10)
        {
            // 延迟足够时间后，显示完成信息
            completeDisplay();
            info_delay_count++;

            if (info_delay_count > 15)
            { // 再延迟0.5秒
                is_resetting = 0;
                reset_stage = RESET_STAGE_IDLE;
                reset_progress = 0;
                info_delay_count = 0;
                system("reboot");
            }
        }
    }
    else if (message == RESET_FINISH)
    {
        info_delay_count++;
        if (info_delay_count == 60)
        {
            completeDisplay();
        }
        else if (120 < info_delay_count)
        {
            info_delay_count = 0;
            os_layout_goto(&layout_dial);
        }
    }
    usleep(10 * 1000);
}

layout layout_info = {
    .init = layout_info_init,
    .enter = layout_info_enter,
    .quit = layout_info_quit,
    .timer = layout_info_timer};