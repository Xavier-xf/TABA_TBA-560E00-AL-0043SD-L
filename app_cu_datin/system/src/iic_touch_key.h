#ifndef IIC_TOUCH_KEY_H
#define IIC_TOUCH_KEY_H




#define TOUCH_KEY_ADDRES 0x05

#define TOUCH_KEY_NUMBER        16
#define TOUCH_KEY_0_VALUE		0x18
#define TOUCH_KEY_1_VALUE 		0x11
#define TOUCH_KEY_2_VALUE 		0x10
#define TOUCH_KEY_3_VALUE 		0x09
#define TOUCH_KEY_4_VALUE 		0x13
#define TOUCH_KEY_5_VALUE		0x12
#define TOUCH_KEY_6_VALUE 		0x04
#define TOUCH_KEY_7_VALUE 		0x15
#define TOUCH_KEY_8_VALUE 		0x16
#define TOUCH_KEY_9_VALUE 		0x03
#define TOUCH_KEY_STAR_VALUE 	0x17
#define TOUCH_KEY_POUND_VALUE 	0x02
#define TOUCH_KEY_UP_VALUE 		0x07
#define TOUCH_KEY_DOWN_VALUE 	0x06
#define TOUCH_KEY_RING_VALUE 	0x00
#define TOUCH_KEY_GUARD_VALUE 	0x01



#define TOUCH_KEY_LONG_TIME 1000     

#define TOUCH_KEY_INT_GPIO 56


#define GPIO_VAL_PATH(pin) "/sys/class/gpio/gpio%d/value",pin




extern void iic_touch_key_init(void);



#endif
