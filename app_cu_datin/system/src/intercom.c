#include "intercom.h"
#include "uart.h"
#include "layout_base.h"
#include "gpio_control.h"
#include "string.h"
static struct ak_timeval g_monitor_2min_start_time = {0, 0};
static void intercom_unlock_start_process(void)
{
	Intercom.send_cmd(CMD_ACK, 0x01, CMD_NULL, CMD_NULL, CMD_NULL);
	intercom_open_door(true);
}

static void intercom_unlock_end_process(void)
{
	Intercom.send_cmd(CMD_ACK, 0x01, CMD_NULL, CMD_NULL, CMD_NULL);
	intercom_open_door(false);
}

static void intercom_monitor_start_process(void)
{
	Intercom.send_cmd(CMD_ACK, 0x01, CMD_NULL, CMD_NULL, CMD_NULL);
	Intercom.status = INT_TALK;
	camera_led_gpio_control(true);
	camera_power_gpio_control(true);
	amplifier_gpio_control(true);
	mic_mute_gpio_control(true);

	ak_get_ostime(&g_monitor_2min_start_time);

	if (whichAway != FROM_OUTDOOR)
	{
		whichAway = FROM_INDOOR;
	}
	if (whichAway == FROM_OUTDOOR)
	{
		whichAway = CONNECTED;
	}
}

static void intercom_monitor_end_process(void)
{
	Intercom.send_cmd(CMD_ACK, 0x01, CMD_NULL, CMD_NULL, CMD_NULL);
	ak_sleep_ms(10);
	Intercom.send_cmd(CMD_READ_MONITOR_STATUS, CMD_NULL, CMD_NULL, CMD_NULL, CMD_NULL);
	Intercom.status = INT_READ_MONITOR_STATUS;

	whichAway = FROM_NONE;
}

static void intercom_ack_process(unsigned char data1)
{
	if (data1 == 0x01)
	{
		return;
	}
	if (Intercom.status == INT_SET_HOME_ID)
	{
		HomeIdSetClass.set_status = HOME_ID_SET_STATUS_SUCCESS;
	}
}

static bool room_number_exist(unsigned int room)
{
	unsigned int i = 0;
	for (i = 0; i < get_int_conf(SET_HOME_ID_INDEX); i++)
	{
		if (UserData.home_id[i] == room)
		{
			return false;
		}
	}
	return true; // 这个数字还没有被保存
}

static void save_number(unsigned int number)
{
	if (number > 0)
	{
		int index = get_int_conf(SET_HOME_ID_INDEX);
		if (room_number_exist(number))
		{
			UserData.home_id[index] = number;
			LOG_WHITE("save UserData home_id[%d] = %d\n\r", index, number);
			index++;
			user_data_save();
			set_int_conf(SET_HOME_ID_INDEX, index);
		}
	}
}

static void intercom_read_home_id_process(unsigned char data1, unsigned char data2, unsigned char data3)
{
	int home_number = data2 * 100 + data3; // 存在分支器上的数字
	unsigned char brancher_id = OutPUTClass.auth_info.brancher_seq[0] * 100 +
								OutPUTClass.auth_info.brancher_seq[1] * 10 +
								OutPUTClass.auth_info.brancher_seq[2] * 1;
	LOG_WHITE(" number:[%d] data2:[%#x], data3[%#x],brancher_id:[%d]\n", home_number, data2, data3, brancher_id);

	if (10000 < home_number)
	{
		LOG_WHITE("the number read:[%d]\n\r", home_number);
		HomeIdSetClass.read_id_show = READ_HOME_ID_SHOW;
	}
	else
	{
		switch (data1)
		{
		case HOME_ID1:
			memset(HomeIdSetClass.dialog_box[0]->font.string1, 0, 10);
			sprintf(HomeIdSetClass.dialog_box[0]->font.string1, "%d", home_number);
			HomeIdSetClass.set_id[HOME_ID_SET_M1_DIALOG_BOX] = home_number;
			save_number(home_number);
			Intercom.send_cmd(CMD_READ_HOME_ID, HOME_ID2, brancher_id, CMD_NULL, CMD_NULL);
			break;
		case HOME_ID2:
			memset(HomeIdSetClass.dialog_box[1]->font.string1, 0, 10);
			sprintf(HomeIdSetClass.dialog_box[1]->font.string1, "%d", home_number);
			HomeIdSetClass.set_id[HOME_ID_SET_M2_DIALOG_BOX] = home_number;
			save_number(home_number);
			Intercom.send_cmd(CMD_READ_HOME_ID, HOME_ID3, brancher_id, CMD_NULL, CMD_NULL);
			break;
		case HOME_ID3:
			memset(HomeIdSetClass.dialog_box[2]->font.string1, 0, 10);
			sprintf(HomeIdSetClass.dialog_box[2]->font.string1, "%d", home_number);
			HomeIdSetClass.set_id[HOME_ID_SET_M3_DIALOG_BOX] = home_number;
			save_number(home_number);
			Intercom.send_cmd(CMD_READ_HOME_ID, HOME_ID4, brancher_id, CMD_NULL, CMD_NULL);
			break;
		case HOME_ID4:
			memset(HomeIdSetClass.dialog_box[3]->font.string1, 0, 10);
			sprintf(HomeIdSetClass.dialog_box[3]->font.string1, "%d", home_number);
			HomeIdSetClass.set_id[HOME_ID_SET_M4_DIALOG_BOX] = home_number;
			save_number(home_number);
			HomeIdSetClass.read_id_show = READ_HOME_ID_SHOW;
			break;
		}
	}
}

static void intercom_read_monitor_status_process(unsigned char data1)
{
	// 有楼层分支器监控
	if (Intercom.status == INT_READ_MONITOR_STATUS && data1 == 1)
	{
		Intercom.status = INT_TALK;
		LOG_WHITE("有楼层分支器监控\n");
	}
}

static void monitor_status_check(void)
{
	static struct ak_timeval start_cpu_time = {0, 0};
	struct ak_timeval current_cpu_time, g_monitor_2min_end_time;

	if (Intercom.status == INT_READ_MONITOR_STATUS)
	{
		ak_get_ostime(&current_cpu_time);
		if (ak_diff_ms_time(&current_cpu_time, &start_cpu_time) >= 800)
		{
			Intercom.status = INT_IDLE;
			camera_led_gpio_control(false);
			camera_power_gpio_control(false);
			mic_mute_gpio_control(false);
		}
	}
	else
	{
		ak_get_ostime(&start_cpu_time);
	}

	if (Intercom.status == INT_TALK)
	{
		ak_get_ostime(&g_monitor_2min_end_time);

		if (ak_diff_ms_time(&g_monitor_2min_end_time, &g_monitor_2min_start_time) >= MONITOR_2MIN_TIMEOUT_MS)
		{
			Intercom.status = INT_IDLE;
			camera_led_gpio_control(false);
			camera_power_gpio_control(false);
			mic_mute_gpio_control(false);
			LOG_WHITE("Monitor 2 minutes timeout, force close!\n");
		}
		standby_timer_reset();
	}
}

void intercom_event_detect(void)
{
	unsigned char cmd, data1, data2, data3, data4;

	if (Intercom.receive_cmd(&cmd, &data1, &data2, &data3, &data4))
	{
		// LOG_WHITE("cmd = %#x  %#x, %#x, %#x, %#x \n", cmd, data1, data2, data3, data4);
		switch (cmd)
		{
		case CMD_UNLOCK_START:
			LOG_BLUE(">>> unlock start\n\r");
			intercom_unlock_start_process();
			break;
		case CMD_UNLOCK_END:
			LOG_BLUE(">>> unlock end\n\r");
			intercom_unlock_end_process();
			break;
		case CMD_MONITOR_START:
			LOG_BLUE(">>> monitor start\n\r");
			intercom_monitor_start_process();
			break;
		case CMD_MONITOR_END:
			LOG_BLUE(">>> monitor end\n\r");
			intercom_monitor_end_process();
			break;
		case CMD_SEND_HOME_ID:
			LOG_BLUE(">>>read to home id\n\r");
			intercom_read_home_id_process(data1, data2, data3);
			break;
		case CMD_SEND_MONITOR_STATUS:
			LOG_BLUE(">>>read to monitor status\n\r");
			intercom_read_monitor_status_process(data1);
			break;
		case CMD_ACK:
			// LOG_WHITE(">>>receive ack\n\r");
			intercom_ack_process(data1);
			break;
		}
	}

	monitor_status_check();
}

/********************************************************************************
@Function:    CAN发送命令编码
@Input:       (unsigned char)  cmd：命令       data1-4 ：要发送的数据
@Output:      (void)
@Description: 将要发送的命令数据转化为标准协议
			  起始+长度+命令+数据+校验+结束
@Author:      kevin
@Date:        2021-09-24
*********************************************************************************/
static void send_can_cmd_encode(unsigned char cmd, unsigned char data1, unsigned char data2, unsigned char data3, unsigned char data4)
{

	char buf[10] = {0};
	unsigned char index = 0;
	unsigned char length = CMD_LENGTH_BASE;
	unsigned char check_code = 0;
	bool data1_null, data2_null, data3_null, data4_null;

	if (data1 != CMD_NULL)
	{
		length++;
		data1_null = false;
	}
	else
	{
		data1 = 0x00;
		data1_null = true;
	}

	if (data2 != CMD_NULL || cmd == CMD_SET_HOME_ID)
	{
		length++;
		data2_null = false;
	}
	else
	{
		data2 = 0x00;
		data2_null = true;
	}

	if (data3 != CMD_NULL || cmd == CMD_SET_HOME_ID)
	{
		length++;
		data3_null = false;
	}
	else
	{
		data3 = 0x00;
		data3_null = true;
	}

	if (data4 != CMD_NULL)
	{
		length++;
		data4_null = false;
	}
	else
	{
		data4 = 0x00;
		data4_null = true;
	}

	check_code = (length + cmd + data1 + data2 + data3 + data4) & 0xFF;

	buf[index] = CMD_START;
	index++;
	buf[index] = length;
	index++;
	buf[index] = cmd;
	index++;
	if (data1_null == false)
	{
		buf[index] = data1;
		index++;
	}
	if (data2_null == false)
	{
		buf[index] = data2;
		index++;
	}
	if (data3_null == false)
	{
		buf[index] = data3;
		index++;
	}
	if (data4_null == false)
	{
		buf[index] = data4;
		index++;
	}
	buf[index] = check_code;
	index++;
	buf[index] = CMD_END;

#if 0
	for (char i = 0; i <= index; i++)
	{
		printf("---------->> buf[%d] = %x\n\r", i, buf[i]);
	}
#endif
	uart_put(buf, (length + 0x02));
}

/**********************************************************************************
@Function:    CAN接收命令解码
@Input:       (unsigned char)  cmd：命令       data1-4 ：接收到的数据
@Output:      (char) return -1:接收到的命令不符合协议
					 return  1：接收的命令符合协议
@Description: 将接收到的数据进行解码，判断符合协议后赋值给命令和数据
@Author:      kevin
@Date:        2021-09-04
***********************************************************************************/
static char receive_can_cmd_decode(unsigned char *cmd, unsigned char *data1, unsigned char *data2, unsigned char *data3, unsigned char *data4)
{

	unsigned char length = 0x00;
	unsigned char check = 0x00;
	unsigned char temp;
	char buf[10] = {0x00};

	temp = uart_get(buf, 1);
	if (!temp)
	{
		return -1;
	}
	// 判断如果读到的数据不是“命令开始”,如果不是则返回不取
	if (buf[0] != CMD_START)
	{
		return -1;
	}
	// 此处延时可以防止偶尔读取不到数据
	ak_sleep_ms(10);

	// 读取数据长度
	temp = uart_get(buf, 1);
	if (!temp)
	{
		return -1;
	}
	else
	{
		length = buf[0]; // 0xA0
	}

	temp = uart_get(buf, length);
	if (!temp)
	{
		return -1;
	}

	// 判断如果读到的数据最后一位不是“命令结束”，如果不是则返回不取
	if (buf[length - 1] != CMD_END)
	{
		LOG_WHITE("读到的数据最后一位不是[命令结束]  返回 -1 不取 \n");
		return -1;
	}

	switch (length)
	{
	case 0x03:
		*cmd = buf[0];
		check = (length + *cmd) & 0xFF;
		break;
	case 0x04:
		*cmd = buf[0];
		*data1 = buf[1];
		check = (length + *cmd + *data1) & 0xFF;
		break;
	case 0x05:
		*cmd = buf[0];
		*data1 = buf[1];
		*data2 = buf[2];
		check = (length + *cmd + *data1 + *data2) & 0xFF;
		break;
	case 0x06:
		*cmd = buf[0];
		*data1 = buf[1];
		*data2 = buf[2];
		*data3 = buf[3];
		check = (length + *cmd + *data1 + *data2 + *data3) & 0xFF;
		break;
	case 0x07:
		*cmd = buf[0];
		*data1 = buf[1];
		*data2 = buf[2];
		*data3 = buf[3];
		*data4 = buf[4];
		check = (length + *cmd + *data1 + *data2 + *data3 + *data4) & 0xFF;
		break;
	}

	// 通过校验位判断数据是否发送正确，如果正确返回1，否则返回-1
	if (check != buf[length - 2])
	{
		return -1;
	}
	else
	{
		if (*cmd == CMD_SEND_HOME_ID)
			LOG_WHITE("%#X, %#X, %#X, %#X, %#X \n", *cmd, *data1, *data2, *data3, *data4);
		return 1;
	}
}

static void intercom_call_home_id(void)
{
	if (CallingClass.call_object == CALL_USER)
	{
		Intercom.send_cmd(CMD_DIAL, DialClass.home_id[0], DialClass.home_id[1], DialClass.home_id[2], DialClass.home_id[3]);
		LOG_WHITE("call [%d %d %d %d]\n\r", DialClass.home_id[0], DialClass.home_id[1], DialClass.home_id[2], DialClass.home_id[3]);
	}
	else
	{
		int admin_num = get_int_conf(ADMIN_CALL_NUMBER);
		if (admin_num > 0 && admin_num < 10000)
		{
			unsigned char data1 = (admin_num / 1000) % 10;
			unsigned char data2 = (admin_num / 100) % 10;
			unsigned char data3 = (admin_num / 10) % 10;
			unsigned char data4 = admin_num % 10;
			Intercom.send_cmd(CMD_DIAL, data1, data2, data3, data4);
			LOG_WHITE("call [%d %d %d %d]\n\r", data1, data2, data3, data4);
		}
	}
	Intercom.status = INT_WAIT_ACK;
	whichAway = FROM_OUTDOOR;

	if (get_camera_power_state() == 2)
	{
		LOG_WHITE("camera is opened\n");

		Intercom.status = INT_TALK;
	}
}

static void intercom_set_home_id(void)
{

	unsigned char home = (unsigned char)HomeIdSetClass.cur_focus + 1;

	unsigned char home_id_high = (HomeIdSetClass.set_id[HomeIdSetClass.cur_focus] >> 8) & 0xFF;
	unsigned char home_id_low = HomeIdSetClass.set_id[HomeIdSetClass.cur_focus] & 0xFF;
	unsigned char brancher_id = OutPUTClass.auth_info.brancher_seq[0] * 100 +
								OutPUTClass.auth_info.brancher_seq[1] * 10 +
								OutPUTClass.auth_info.brancher_seq[2] * 1;
	Intercom.send_cmd(CMD_SET_HOME_ID, home, home_id_high, home_id_low, brancher_id);

	LOG_WHITE("set home id %x, %x, %x, %x ,brancher_id=%d\n", HomeIdSetClass.set_id[HomeIdSetClass.cur_focus], home, home_id_high, home_id_low, brancher_id);
}

static void intercom_read_home_id(void)
{
	LOG_WHITE("send CMD_READ_HOME_ID \n\r");
	unsigned char brancher_id = OutPUTClass.auth_info.brancher_seq[0] * 100 +
								OutPUTClass.auth_info.brancher_seq[1] * 10 +
								OutPUTClass.auth_info.brancher_seq[2] * 1;
	Intercom.send_cmd(CMD_READ_HOME_ID, HOME_ID1, brancher_id, CMD_NULL, CMD_NULL);
}

STR_IntercomClass Intercom = {
	INT_IDLE,				// status
	send_can_cmd_encode,	// send_cmd
	receive_can_cmd_decode, // receive_cmd
	intercom_call_home_id,	// call
	intercom_set_home_id,	// set_id
	intercom_read_home_id,	// read_id
};
