#ifndef _KEVIN_FUNCTION_H
#define _KEVIN_FUNCTION_H
#include ".\include\REG_MG82F6D64.H"
#include ".\include\Type.h"

// xiang新增拨码开关引脚定义
#define DIP_SW1 P14
#define DIP_SW2 P15
#define DIP_SW3 P16
#define DIP_SW4 P17
#define DIP_SW5 P20
#define DIP_SW6 P21
#define DIP_SW7 P22
#define DIP_SW8 P23



#define POWER_LED	P11
#define UNLOCK_GPIO P01
#define RXD_GPIO    P12

#define POWER_HOME1_GPIO P25 
#define POWER_HOME2_GPIO P26
#define POWER_HOME3_GPIO P27 
#define POWER_HOME4_GPIO P32 

#define AUDIO_VIDEO_CH1_GPIO P14
#define AUDIO_VIDEO_CH2_GPIO P15
#define AUDIO_VIDEO_CH3_GPIO P16
#define AUDIO_VIDEO_CH4_GPIO P17

#define HOME1_RING_GPIO P33
#define HOME2_RING_GPIO P34
#define HOME3_RING_GPIO P35
#define HOME4_RING_GPIO P36

#define HOME1_CALL_GPIO P37
#define HOME2_CALL_GPIO P40
#define HOME3_CALL_GPIO P41
#define HOME4_CALL_GPIO P42





/************************************************************
						CAN 总线指令集
************************************************************/
#define CMD_START 					0xA0
#define CMD_ACK   					0xAA
#define CMD_DIAL        			0xB0
#define CMD_SET_HOME_ID 			0xB1
#define CMD_UNLOCK_START   			0xB2
#define CMD_UNLOCK_END   			0xB3
#define CMD_MONITOR_START   		0xB4
#define CMD_MONITOR_END     		0xB5
#define CMD_READ_HOME_ID    		0xB6
#define CMD_SEND_HOME_ID    		0xB7
#define CMD_READ_MONITOR_STATUS 	0xB8
#define CMD_SEND_MONITOR_STATUS 	0xB9

#define CMD_END   					0xF0

#define CMD_LENGTH_BASE 			0x03
#define CMD_NULL   					0xFF

/************************************************************/



/*************************************************************
	                  用户数据保存IAP地址
*************************************************************/
typedef enum{
	HOME1_ID_ADDR = 0x7200,
 	HOME2_ID_ADDR = 0x7204,
 	HOME3_ID_ADDR = 0x7208,
	HOME4_ID_ADDR = 0x7212,

	HOME_ID_DEFAULT_ADDR = 0x7216
}USER_DATA_ADDR;
/************************************************************/


typedef enum{
	false,
	true
}BOOL;


typedef enum{
	LOW_LEVEL,
	HIGH_LEVEL
}LEVEL;

typedef enum{
	HOME_ID1 = 0x01,
	HOME_ID2,
	HOME_ID3,
	HOME_ID4,
	TOTAL_HOME_ID
}HOME_ID;	


typedef enum{
	MONITOR_IDLE,
	MONITOR_START_WAIT_ACK,
	MONITOR_END_WAIT_ACK,
	MONITOR_BUSY
}MONITOR_STATUS;

typedef enum{
	UNLOCK_IDLE,
	UNLOCK_START_WAIT_ACK,
	UNLOCK_END_WAIT_ACK,
	UNLOCK_BUSY
}UNLOCK_STATUS;


typedef enum{
    UART_DEVICE_1,
    UART_DEVICE_MAX
} UART_DEVICE;


typedef struct{
	uint8_t tx_in_index;
	uint8_t tx_out_index;
	uint8_t tx_count;
	uint8_t tx_falg;
	uint8_t rx_in_index;
	uint8_t rx_out_index;

} UART_CLASS;
extern UART_CLASS UartClass;

typedef struct{
	
	uint8_t send_count;
	uint8_t home1_id[4];
	uint8_t home2_id[4];
	uint8_t home3_id[4];
	uint8_t home4_id[4];
	MONITOR_STATUS monitor_status;
	UNLOCK_STATUS unlock_status;

}INTERCOM_CLASS;
extern INTERCOM_CLASS Intercom;


typedef struct{
	uint8_t home1_id[4];
	uint8_t home2_id[4];
	uint8_t home3_id[4];
	uint8_t home4_id[4];

	uint8_t home_id_default[4];
}USER_DATA;
extern USER_DATA UserData;


extern void gpio_init(void);
extern void uart1_init(void);
extern void fun_delay_ms(unsigned int ms);
extern void open_audio_video_ch(HOME_ID ch);
extern void close_audio_video_ch(HOME_ID ch);
extern void call_home(HOME_ID ch);
extern void open_floor_ch(void);
extern void user_data_init(void);
extern void send_can_cmd_encode(uint8_t cmd, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4);
extern char receive_can_cmd_decode(uint8_t *cmd, uint8_t *data1, uint8_t *data2, uint8_t *data3, uint8_t *data4);
extern void uart_send_byte(uint8_t data1);
extern void save_user_data(USER_DATA_ADDR addr, uint8_t *home_id);
extern void InitPort(void);


extern void DelayXms(u16 xMs);

// xiang声明全局变量
extern uint8_t base_address;
extern uint8_t dip_switch_value;
extern uint8_t prev_base_address; 
// xiang函数声明
void read_dip_switch(void);
u8 reverse_bits(u8 value);

#endif
