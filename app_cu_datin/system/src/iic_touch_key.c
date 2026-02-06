#include "iic_touch_key.h"
#include "ui_api.h"
#include "i2c_devices_api.h"
#include "ak_thread.h"
#include <poll.h>
#include "gpio_api.h"
#include "fcntl.h"
#include "layout_base.h"
#include <sys/types.h>
#include <unistd.h>
#include "string.h"





static bool is_iic_touch_key_check = false;
const unsigned char TouchKeyValue[TOUCH_KEY_NUMBER] = {

	TOUCH_KEY_0_VALUE,		
 	TOUCH_KEY_1_VALUE, 		
 	TOUCH_KEY_2_VALUE, 		
	TOUCH_KEY_3_VALUE, 		
	TOUCH_KEY_4_VALUE, 		
	TOUCH_KEY_5_VALUE,		
	TOUCH_KEY_6_VALUE, 		
	TOUCH_KEY_7_VALUE, 		
	TOUCH_KEY_8_VALUE, 		
	TOUCH_KEY_9_VALUE, 		
	TOUCH_KEY_STAR_VALUE, 	
 	TOUCH_KEY_POUND_VALUE,	
 	TOUCH_KEY_UP_VALUE, 		
 	TOUCH_KEY_DOWN_VALUE, 	
 	TOUCH_KEY_RING_VALUE, 	
 	TOUCH_KEY_GUARD_VALUE 	
};




static int get_touch_key_index(unsigned char data){
	unsigned char cur_key;

	for (cur_key = 0; cur_key < TOUCH_KEY_NUMBER; cur_key++){
		if(data == TouchKeyValue[cur_key])
			return cur_key;
	}
	return KEY_INDEX_NONE;
}

static struct pollfd fds;
#define GPIO_ISR_NUM 4

static void touch_key_interrupt_check(void){
	char buf[3];

	lseek(fds.fd, 0, SEEK_SET);
	if(read(fds.fd, buf, 3) > 0){
		if(buf[0] == '0'){
			is_iic_touch_key_check = true;
		}else{
			is_iic_touch_key_check = false;
		}
	}
}

/******************  
 * 返回被按下的按键的ID
 * ******************/
int get_touch_index(){
	i2c_data data;
	i2c_read(I2C_DEVICES_4, TOUCH_KEY_ADDRES, &data);
	int key_index = KEY_INDEX_NONE;
	if(is_iic_touch_key_check){
		i2c_read(I2C_DEVICES_4, TOUCH_KEY_ADDRES, &data);
		key_index = get_touch_key_index(data.data);
	}else{
		key_index = KEY_INDEX_NONE;
	}

	return key_index;
}

static void *iic_touch_key_task(void* arg){

	i2c_data data;
	int key_index = KEY_INDEX_NONE;				// 现在按下的按钮
	static int prev_key_index = KEY_INDEX_NONE;		// 上一个按下的按钮
	static struct ak_timeval start_cpu_time;
	static struct ak_timeval cur_cpu_time;

	while(1){
		touch_key_interrupt_check();

		if(is_iic_touch_key_check){
			i2c_read(I2C_DEVICES_4, TOUCH_KEY_ADDRES, &data);
			key_index = get_touch_key_index(data.data);
		}else{
			key_index = KEY_INDEX_NONE;
		}


		#if 1		
		if((prev_key_index == KEY_INDEX_NONE) && (key_index != KEY_INDEX_NONE)){
			// LOG_WHITE("KEY [%d] DOWN\n\r",key_index);
			touch_key_event(key_index, KEY_STATE_DOWN);
			ak_get_ostime(&start_cpu_time);
			prev_key_index = key_index;

		}else if((prev_key_index == key_index) && (key_index != KEY_INDEX_NONE)){
			ak_get_ostime(&cur_cpu_time);	
			if(ak_diff_ms_time(&cur_cpu_time, &start_cpu_time) >= TOUCH_KEY_LONG_TIME){
				// LOG_WHITE("KEY [%d] PRESS\n\r",key_index);
				touch_key_event(key_index, KEY_STATE_LONG_DOWN);
			}

		}else if(prev_key_index != KEY_INDEX_NONE && (key_index == KEY_INDEX_NONE)){
			// LOG_WHITE("KEY [%d] UP\n\r",prev_key_index);
			touch_key_event(prev_key_index, KEY_STATE_UP);
			start_cpu_time.sec = 0;
			start_cpu_time.usec = 0;
			prev_key_index = KEY_INDEX_NONE;
	    }
		#endif

		ak_sleep_ms(10);
	}
	ak_thread_exit();
    return NULL;
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
    *level = (value_str[0] == '0') ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH;
    return true;
}


static void touch_key_interrupt_init(void){
	char buffer[64] = {0};
	GPIO_LEVEL level;
	gpio_edge(TOUCH_KEY_INT_GPIO, ISR_BOTH);
	gpio_level_read(TOUCH_KEY_INT_GPIO, &level);

	memset(buffer, 0, sizeof(buffer));
    sprintf(buffer, GPIO_VAL_PATH(TOUCH_KEY_INT_GPIO));
    fds.fd = open(buffer,O_RDONLY);
}


void iic_touch_key_init(void){
    ak_pthread_t pthread_id;

	touch_key_interrupt_init();
    ak_thread_create(&pthread_id, iic_touch_key_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
}

