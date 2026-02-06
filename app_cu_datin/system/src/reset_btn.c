#include "gpio_control.h"
#include "gpio_api.h"
#include "fcntl.h"
#include "ak_thread.h"

#include "layout_base.h"

/******* 控制按钮的引脚 ******/
#define RESET_BTN_CTRL 60

#define RESET_BTN_PRESS_LEVEL 1   // 被按下
#define RESET_BTN_DEFAULT_LEVEL 2 // 默认状态

/*************************************************************************
 * @brief  获取重置按钮的电平
 * @date   2022-08-18 19:57
 * @author xiaoele
 * @return GPIO_LEVEL level   默认状态下为 [2]  按下的时候为 [1]
 **************************************************************************/
static int getResetBtnLevel(void)
{
    GPIO_LEVEL level;
    gpio_read(RESET_BTN_CTRL, &level);
    return level;
}

/*************************************************************************
 * @brief  进行密码初始化
 * @date   2022-08-18 19:58
 * @author xiaoele
 **************************************************************************/
static void initPassWord(void)
{
    message = RESET_PASSWORD;
    os_layout_goto(&layout_info);
}

/*************************************************************************
 * @brief  按钮线程处理的事件
 * @date   2022-08-18 20:00
 * @author xiaoele
 * @return NULL
 **************************************************************************/
static void *resetBtnTask(void *arg)
{
    int btn_level;
    int count = 0;
    while (1)
    {
        btn_level = getResetBtnLevel();
        switch (btn_level)
        {
        case RESET_BTN_PRESS_LEVEL:
            count++;
            break;
        case RESET_BTN_DEFAULT_LEVEL:
            count = 0;
            break;
        }

        if (count == 10)
        {
            count = 0;
            LOG_WHITE("reset button was pressed for 5 seconds,now reset the password\n");
            initPassWord();
        }

        ak_sleep_ms(500);
    }

    ak_thread_exit();
    return NULL;
}


/*************************************************************************
 * @brief  创建线程监听重置按钮
 * @date   2022-08-18 19:59
 * @author xiaoele
 **************************************************************************/
void reset_button_init(void)
{
    ak_pthread_t pthread_id;

    ak_thread_create(&pthread_id, resetBtnTask, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
}
