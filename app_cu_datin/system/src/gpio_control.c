#include "gpio_control.h"
#include "gpio_api.h"
#include "layout_base.h"

static int camera_power_state = 0;

/*************************************************************************
 * @brief  获取camera的状态
 * @date   2022-08-17 21:11
 * @author
 * @return camera_power_state   1->LOW 2>HIGH
 **************************************************************************/
int get_camera_power_state(void)
{
    return camera_power_state;
}

bool back_light_gpio_control(bool status)
{
    bool ret = false;
    if (status == true)
    {
        ret = gpio_set(IO_BACK_LIGHT_CTRL, GPIO_LEVEL_HIGH);
    }
    else
    {
        ret = gpio_set(IO_BACK_LIGHT_CTRL, GPIO_LEVEL_LOW);
    }
    return ret;
}

/*************************************************************************
 * @brief  内线通话开锁 需要打开摄像头后才能开锁
 * @date   2022-11-02 13:55
 * @author xiaoele
 **************************************************************************/
bool intercom_open_door(bool status)
{

    if (get_camera_power_state() == GPIO_LEVEL_LOW && status == true)
    {
        LOG_WHITE("unlock fail because camera not open \n");
        return false;
    }

    bool ret = false;
    if (status == true)
    {
        ret = gpio_set(IO_LOCK_CTRL, GPIO_LEVEL_HIGH);
    }
    else
    {
        ret = gpio_set(IO_LOCK_CTRL, GPIO_LEVEL_LOW);
    }
    return ret;
}

/*************************************************************************
 * @brief  刷卡/输入密码开锁
 * @date   2022-11-02 13:53
 * @author xiaoele
 **************************************************************************/
bool card_pwd_open_door(bool status)
{
    bool ret = false;
    if (status == true)
    {
        ret = gpio_set(IO_LOCK_CTRL, GPIO_LEVEL_HIGH);
    }
    else
    {
        ret = gpio_set(IO_LOCK_CTRL, GPIO_LEVEL_LOW);
    }
    return ret;
}

bool camera_led_gpio_control(bool status)
{
    bool ret = false;
    if (status == true)
    {
        ret = gpio_set(IO_CAMERA_LED_CTRL, GPIO_LEVEL_HIGH);
    }
    else
    {
        ret = gpio_set(IO_CAMERA_LED_CTRL, GPIO_LEVEL_LOW);
    }
    return ret;
}

bool camera_power_gpio_control(bool status)
{
    bool ret = false;
    if (status == true)
    {
        ret = gpio_set(IO_CAMERA_POWER_CTRL, GPIO_LEVEL_HIGH);
        camera_power_state = GPIO_LEVEL_HIGH;
    }
    else
    {
        ret = gpio_set(IO_CAMERA_POWER_CTRL, GPIO_LEVEL_LOW);
        camera_power_state = GPIO_LEVEL_LOW;
    }
    return ret;
}

bool mute_34118_gpio_control(bool status)
{
    bool ret = false;
    if (status == true)
    {
        ret = gpio_set(IO_34118_MUTE_CTRL, GPIO_LEVEL_LOW);
    }
    else
    {
        ret = gpio_set(IO_34118_MUTE_CTRL, GPIO_LEVEL_HIGH);
    }
    return ret;
}

bool amplifier_gpio_control(bool status)
{
    bool ret = false;
    if (status == true)
    {
        ret = gpio_set(IO_AMPLIFIER_CTRL, GPIO_LEVEL_LOW);
    }
    else
    {
        ret = gpio_set(IO_AMPLIFIER_CTRL, GPIO_LEVEL_HIGH);
    }
    return ret;
}

/*************************************************************************
 * @brief  控制mic的 打开与关闭
 * @date   2022-09-21 14:34
 * @author xiaoele
 * @param  true 高电平
 * @param  false 低电平
 * @return ret
 **************************************************************************/
bool mic_mute_gpio_control(bool status)
{
    return gpio_set(GPIO_MIC_MUTE_PIN, status == true ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

bool volume_gpio_control(char volume)
{
    bool ret = false;
    switch (volume)
    {
    case 1:
        ret = gpio_set(IO_VOLUME1_CTRL, GPIO_LEVEL_HIGH);
        ret = gpio_set(IO_VOLUME2_CTRL, GPIO_LEVEL_HIGH);
        break;
    case 2:
        ret = gpio_set(IO_VOLUME1_CTRL, GPIO_LEVEL_HIGH);
        ret = gpio_set(IO_VOLUME2_CTRL, GPIO_LEVEL_LOW);
        break;
    case 3:
        ret = gpio_set(IO_VOLUME1_CTRL, GPIO_LEVEL_LOW);
        ret = gpio_set(IO_VOLUME2_CTRL, GPIO_LEVEL_HIGH);
        break;
    case 4:
        ret = gpio_set(IO_VOLUME1_CTRL, GPIO_LEVEL_LOW);
        ret = gpio_set(IO_VOLUME2_CTRL, GPIO_LEVEL_LOW);
        break;
    }

    return ret;
}
