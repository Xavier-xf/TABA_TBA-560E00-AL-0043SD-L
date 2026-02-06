#include "gpio_api.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "ak_common.h"
#include <poll.h>
#include "ak_thread.h"
#include "ak_mem.h"

#define GPIO_PATH(pin) "/sys/class/gpio/gpio%d",pin
#define GPIO_EXPORT_PATH "/sys/class/gpio/export"

#define GPIO_DIR_PATH(pin) "/sys/class/gpio/gpio%d/direction",pin

#define GPIO_VAL_PATH(pin) "/sys/class/gpio/gpio%d/value",pin

#define GPIO_EDGE_PATH(pin) "/sys/class/gpio/gpio%d/edge",pin

#define GPIO_PWM_PIN_PATH(no) "/sys/class/pwm/pwmchip0/pwm%d",no
#define GPIO_PWM_PIN_EXPORT "/sys/class/pwm/pwmchip0/export"
#define GPIO_PWM_PIN_ENABLE(no) "/sys/class/pwm/pwmchip0/pwm%d/enable",no

#define GPIO_PWM_PIN_PERIOD(no) "/sys/class/pwm/pwmchip0/pwm%d/period",no
#define GPIO_PWM_PIN_DUTY_CYCLE(no) "sys/class/pwm/pwmchip0/pwm%d/duty_cycle",no



static bool gpio_export_check(const int pin)
{
    char buffer[64] = {0};
    sprintf(buffer,GPIO_PATH(pin));
    if(access(buffer,F_OK) == 0)
    {
        return true;
    }
    return false;
}


static bool gpio_export_enable(const int pin)
{
    if(gpio_export_check(pin) == true)
    {
        return true;
    }

    int fd = open(GPIO_EXPORT_PATH,O_WRONLY);
    if(fd < 0)
    {
        printf("open %s fail\n\r",GPIO_EXPORT_PATH);
        return false;
    }

    char buffer[3] = {0};
    int len = sprintf(buffer,"%d",pin);
    if(write(fd,buffer,len) < 0)
    {
        printf("write fail to export gpio%d!\n\r",pin);
        close(fd);
        return false;
    }
    close(fd);
    return true;
}


static int gpio_direction_open(const int pin)
{
    char buffer[64] = {0};
    sprintf(buffer,GPIO_DIR_PATH(pin));
    int fd = open(buffer,O_WRONLY);
    return fd;
}

static bool gpio_direction_set(const int pin , GPIO_DIR dir)
{
    int fd = gpio_direction_open(pin);
    if(fd < 0)
    {
    	printf("gpio%d set direction fail \n\r",pin);
        return false;
    }

    char* dir_str = dir == GPIO_IN?"in":"out";
    write(fd,dir_str,strlen(dir_str));
    close(fd);
    return true;
}

static bool gpio_level_set(const int pin,GPIO_LEVEL level)
{
    char buffer[64] = {0};
    sprintf(buffer,GPIO_VAL_PATH(pin));
    
    int fd = open(buffer,O_WRONLY);
    if(fd < 0)
    {
        return false;
    }

    if(write(fd,(level == GPIO_LEVEL_LOW)?"0":"1",1) < 0)
    {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}


static bool gpio_level_read(const int pin,GPIO_LEVEL* level)
{
    char buffer[64] = {0};
    sprintf(buffer,GPIO_VAL_PATH(pin));

    int fd = open(buffer,O_RDONLY);
    if(fd < 0)
    {
        return false;
    }

    char value_str[3] = {0};
    if(read(fd,value_str,3) < 0 )
    {
        close(fd);
        return false;
    }
    close(fd);
    *level = (value_str[0] == '0')?GPIO_LEVEL_LOW:GPIO_LEVEL_HIGH;
    return true;
}


bool gpio_set(const int pin,GPIO_LEVEL level)
{
    if(gpio_export_enable(pin) == false)
    {
        return false;
    }

    if(gpio_direction_set(pin,GPIO_OUT) == false)
    {
        return false;
    }


    if(gpio_level_set(pin,level) == false)
    {
        return false;
    }

    return true;
}


bool gpio_read(const int pin,GPIO_LEVEL* level)
{
    if(gpio_export_enable(pin) == false)
    {
        return false;
    }

    if(gpio_direction_set(pin,GPIO_IN) == false)
    {
        return false;
    }

    if(gpio_level_read(pin,level) == false)
    {
    	printf("gpio%d read level fail \n\r",pin);
        return false;
    }
    return true;
}


static bool gpio_edge_set(const int pin,GPIO_EDGE edge)
{
    char buffer[128] = {0};
    sprintf(buffer,GPIO_EDGE_PATH(pin));
    
    int fd = open(buffer,O_RDWR);
    if(fd < 0)
    {
        return false;
    }

    char* pstr = (edge == ISR_RISING)?"rising":(edge == ISR_FALLING)?"falling":"none";
    if(write(fd,pstr,strlen(pstr)) < 0)
    {   
        close(fd);
        return false;
    }

    close(fd);
    return true;
}


bool gpio_edge(const int pin , GPIO_EDGE edge)
{
    if(gpio_export_enable(pin) == false)
    {
        return false;
    }

    if(gpio_direction_set(pin,GPIO_IN) == false)
    {
        return false;
    }

    if(gpio_edge_set(pin,edge) == false)
    {
        return false;
    }

    return true;
}




static bool pwm_export_check(const int pwm_no)
{
    char buffer[64] = {0};
    sprintf(buffer,GPIO_PWM_PIN_PATH(pwm_no));

    if(access(buffer,F_OK) == 0)
    {
        return true;
    }
    return false;
}

static bool pwm_export_enable(const int pwm_no)
{
	if(pwm_export_check(pwm_no) == true)
	{
		return true;
	}
	
    int fd = open(GPIO_PWM_PIN_EXPORT,O_WRONLY);
    if(fd < 0)
    {
        return false;
    }

    char buffer[3] = {0};
    sprintf(buffer,"%d",pwm_no);
    if(write(fd,buffer,1)<0)
    {
        close(fd);
        return false;
    }
    close(fd);

    return true;
}

static bool pwm_open(const int pwm_no)
{
    char buffer[64] = {0};
    sprintf(buffer,GPIO_PWM_PIN_ENABLE(pwm_no));
    int fd = open(buffer,O_WRONLY);
    return fd;
}


static bool pwm_enable(const int pwm_no,bool enable)
{
    int fd = pwm_open(pwm_no);
    if(fd < 0)
    {
        return false;
    }

    char buffer[3] = {0};
    sprintf(buffer,"%s",enable?"1":"0");
    if(write(fd,buffer,1) < 0)
    {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}



static int pwm_period_open(const int pwm_no)
{
    char buffer[64] = {0};
    sprintf(buffer,GPIO_PWM_PIN_PERIOD(pwm_no));
    
    int fd = open(buffer,O_WRONLY);
    return fd;
}


static bool pwm_period_set(const int pwm_no,unsigned int period)
{
    int fd = pwm_period_open(pwm_no);
    if(fd < 0)
    {
        return false;
    }

    char buffer[32] = {0};
    sprintf(buffer,"%d",period);
    if(write(fd,buffer,strlen(buffer))<0)
    {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}


static int pwm_sycle_open(const int pwm_no)
{
    char buffer[64] = {0};
    sprintf(buffer,GPIO_PWM_PIN_DUTY_CYCLE(pwm_no));
    int fd = open(buffer,O_WRONLY);
    return fd;
}


static bool pwm_duty_cycle_set(const int pwm_no,unsigned int duty_cycle)
{
    int fd = pwm_sycle_open(pwm_no);
    if(fd < 0)
    {
        return false;
    }

    char buffer[32] = {0};
    sprintf(buffer,"%d",duty_cycle);
    if(write(fd,buffer,strlen(buffer)) < 0)
    {
        close(fd);
        return false;
    }
    close(fd);
    return true;
}



bool pwm_set(const int pwm_no,unsigned int duty_cycle,unsigned int period)
{
    if(pwm_export_enable(pwm_no) == false)
    {
        return false;
    }

    bool enable = duty_cycle?true:false;
    if(pwm_enable(pwm_no,enable) == false)
    {
        return false;
    }

    if(enable == false)
    {
        return true;
    }

    if(pwm_period_set(pwm_no,period) == false)
    {
        return false;
    }

    if(pwm_duty_cycle_set(pwm_no,duty_cycle) == false)
    {
        return false;
    }
    return true;
}




typedef struct
{
	struct pollfd* fds;
	int  fds_to_pins[20];

	isr_gpio_info isr_gpio_group_info;
	
	GPIO_LEVEL* isr_gpio_group_level; 

	int isr_gpio_alarm_num;
	int isr_gpio_det_num;
}gpio_trigger;

static gpio_trigger isr_pin_group;


extern bool door_call_event(int door_channel);
extern bool hook_trigger_event(int arg);
extern bool alarm_trigger_event(int index);

static bool gpio_isr_trigger_event(int index,GPIO_LEVEL level)
{

	if((isr_pin_group.fds_to_pins[index] == isr_pin_group.isr_gpio_group_info.door1_call_pin)&&(isr_pin_group.isr_gpio_group_info.door1_trigger_level & level))
	{
		door_call_event(1);
		return true;
	}

	if((isr_pin_group.fds_to_pins[index] == isr_pin_group.isr_gpio_group_info.door2_call_pin)&&(isr_pin_group.isr_gpio_group_info.door2_trigger_level & level))
	{
		door_call_event(2);
		return true;
	}

	if((isr_pin_group.fds_to_pins[index] == isr_pin_group.isr_gpio_group_info.hook_pin))
	{
		hook_trigger_event(level);
		return true;
	}

	int i = 0 ;
	for(i = 0 ; i < isr_pin_group.isr_gpio_alarm_num ; i++)
	{
		if((isr_pin_group.fds_to_pins[index] == isr_pin_group.isr_gpio_group_info.alarm_det_pin_group[i])&&(isr_pin_group.isr_gpio_group_info.alarm_trigger_level_group[i] & level))
		{
			alarm_trigger_event(i);
		}
	}
	
	return false;
}


static void* gpio_isr_task(void* arg)
{
	int i = 0;
	char buffer[4] ={0};
	while(1)
	{
		//if(poll(isr_pin_group.fds,isr_pin_group.isr_gpio_det_num,20) > 0)
		//{
			for(i = 0 ; i < isr_pin_group.isr_gpio_det_num; i++)
			{
				//if(isr_pin_group.fds[i].revents & POLLIN)
				//{
					lseek(isr_pin_group.fds[i].fd,0,SEEK_SET);
					if(read(isr_pin_group.fds[i].fd,buffer,3) >0)
					{
						if((buffer[0] == '0')&&(isr_pin_group.isr_gpio_group_level[i] != GPIO_LEVEL_LOW))
						{
							isr_pin_group.isr_gpio_group_level[i] = GPIO_LEVEL_LOW;
							gpio_isr_trigger_event(i,GPIO_LEVEL_LOW);
						}
						else if((buffer[0] == '1')&&(isr_pin_group.isr_gpio_group_level[i] != GPIO_LEVEL_HIGH))
						{
							isr_pin_group.isr_gpio_group_level[i] = GPIO_LEVEL_HIGH;
							gpio_isr_trigger_event(i,GPIO_LEVEL_HIGH);
						}
					}
				//}
			}
			ak_sleep_ms(20);
		//}
	}
	ak_thread_exit();
	return NULL;
}



bool gpio_isr_init(const isr_gpio_info* gpio_info)
{
	int i = 0 ;
	int index = 0;
	
	isr_pin_group.isr_gpio_group_info = *gpio_info;
	isr_pin_group.isr_gpio_alarm_num = 0;
	isr_pin_group.isr_gpio_det_num = 0;
	
	if(isr_pin_group.isr_gpio_group_info.door1_call_pin >= 0)
	{
		isr_pin_group.fds_to_pins[index++] = isr_pin_group.isr_gpio_group_info.door1_call_pin;
	}
	if(isr_pin_group.isr_gpio_group_info.door2_call_pin >= 0)
	{
		isr_pin_group.fds_to_pins[index++] = isr_pin_group.isr_gpio_group_info.door2_call_pin;
	}
	if(isr_pin_group.isr_gpio_group_info.hook_pin >= 0)
	{
		isr_pin_group.fds_to_pins[index++] = isr_pin_group.isr_gpio_group_info.hook_pin;
	}

	for(i = 0 ; i < isr_pin_group.isr_gpio_group_info.alarm_det_pin_num ; i++)
	{
		if(isr_pin_group.isr_gpio_group_info.alarm_det_pin_group[i] < 0)
		{
			break;
		}
		isr_pin_group.fds_to_pins[index++] = isr_pin_group.isr_gpio_group_info.alarm_det_pin_group[i];
		isr_pin_group.isr_gpio_alarm_num++;
	}
	
	if(index == 0)
	{
		return false;
	}

	isr_pin_group.fds = (struct pollfd*)ak_mem_alloc(MODULE_ID_APP, sizeof(struct pollfd)*index);
	isr_pin_group.isr_gpio_group_level = (GPIO_LEVEL*)ak_mem_alloc(MODULE_ID_APP, sizeof(struct pollfd)*index);


	char buffer[64] = {0};
	for( i = 0 ; i < index ; i++)
	{
		gpio_edge(isr_pin_group.fds_to_pins[i], ISR_BOTH);
		gpio_level_read(isr_pin_group.fds_to_pins[i],&isr_pin_group.isr_gpio_group_level[i]);

		memset(buffer,0,sizeof(buffer));
	    sprintf(buffer,GPIO_VAL_PATH(isr_pin_group.fds_to_pins[i]));
	    isr_pin_group.fds[i].fd = open(buffer,O_RDONLY);
	 	//isr_pin_group.fds[i].events = POLLIN;
	}
   
	isr_pin_group.isr_gpio_det_num = index;
	ak_pthread_t thread_id;
	ak_thread_create(&thread_id, gpio_isr_task, NULL , ANYKA_THREAD_NORMAL_STACK_SIZE,  -1);
	return true;
}


