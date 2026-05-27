#ifndef INTERCOM_H
#define INTERCOM_H

#define MONITOR_2MIN_TIMEOUT_MS 120000
/************************************************************
						CAN 总线指令集
************************************************************/
#define CMD_START 0xA0
#define CMD_ACK 0xAA
#define CMD_DIAL 0xB0
#define CMD_SET_HOME_ID 0xB1
#define CMD_UNLOCK_START 0xB2
#define CMD_UNLOCK_END 0xB3
#define CMD_MONITOR_START 0xB4
#define CMD_MONITOR_END 0xB5
#define CMD_READ_HOME_ID 0xB6
#define CMD_SEND_HOME_ID 0xB7
#define CMD_READ_MONITOR_STATUS 0xB8
#define CMD_SEND_MONITOR_STATUS 0xB9
#define CMD_HEARTBEAT 0xBA

#define CMD_END 0xF0

#define CMD_LENGTH_BASE 0x03
#define CMD_NULL 0xFF
#define INTERCOM_HEARTBEAT_INTERVAL_MS 1000
#define INTERCOM_HEARTBEAT_DEFER_MS 500
#define INTERCOM_BUS_BUSY_TIMEOUT_MS 4000

/************************************************************/

typedef enum
{
	INT_IDLE,
	INT_WAIT_ACK,
	INT_SET_HOME_ID,
	INT_TALK,
	INT_READ_MONITOR_STATUS
} INTERCOM_STATUS;

typedef enum
{
	HOME_ID1 = 0x01,
	HOME_ID2,
	HOME_ID3,
	HOME_ID4
} HOME_ID;

typedef void (*SendCanCmdCallback)(unsigned char cmd, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4);
typedef char (*ReceiveCanCmdCallback)(unsigned char *cmd, unsigned char *data1, unsigned char *data2, unsigned char *data3, unsigned char *data4);
typedef void (*CallHomeCallback)(void);
typedef void (*SetHomeIdCallback)(void);
typedef void (*ReadHomeIdCallback)(void);

typedef struct
{
	INTERCOM_STATUS status;
	SendCanCmdCallback send_cmd;
	ReceiveCanCmdCallback receive_cmd;
	CallHomeCallback call;
	SetHomeIdCallback set_id;
	ReadHomeIdCallback read_id;
} STR_IntercomClass;

extern STR_IntercomClass Intercom;

extern void intercom_event_detect(void);

#endif
