#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H
#include "stdbool.h"




#define IO_BACK_LIGHT_CTRL 82   

#define IO_LOCK_CTRL 79       

#define IO_CAMERA_LED_CTRL 59 

#define IO_CAMERA_POWER_CTRL 7  

#define IO_34118_MUTE_CTRL 63	 

#define IO_AMPLIFIER_CTRL 66      

#define IO_VOLUME1_CTRL 32        

#define IO_VOLUME2_CTRL 68		

#define GPIO_MIC_MUTE_PIN  61


extern bool back_light_gpio_control(bool status);
extern bool intercom_open_door(bool status);
extern bool camera_led_gpio_control(bool status);
extern bool camera_power_gpio_control(bool status);
extern bool mute_34118_gpio_control(bool status);
extern bool amplifier_gpio_control(bool status);
extern bool volume_gpio_control(char volume);
extern bool mic_mute_gpio_control(bool status);

extern int get_camera_power_state(void);
bool card_pwd_open_door(bool status);



#endif
