#include "layout_base.h"
#include "string.h"
#include "gpio_control.h"
#include "uart.h"
#include "ak_common.h"
#include "leo_audio_play.h"

/*********************************************************************************************************
 * 函 数 名 : printf_card_data
 * 功能说明 : 打印卡片数据的长度和每个字节的十六进制值，用于调试刷卡数据
 * 形    参 : buf - 存放卡片原始数据的缓冲区
 *            length - 卡片数据的有效长度
 * 返 回 值 : 无
 * 备    注 : 仅在数据长度大于0时执行打印操作
 *********************************************************************************************************/
static void printf_card_data(char *buf, int length)
{
	if (length > 0)
	{
		printf("------>>> length = %d\n\r", length);
		for (int i = 0; i < length; i++)
		{
			printf("------>>> buf[%d] = 0x%x\n\r", i, buf[i]);
		}
	}
}

/*********************************************************************************************************
 * 函 数 名 : swiping_card_data_convert_string
 * 功能说明 : 将卡片原始二进制数据转换为"ID:xxxx..."格式的字符串
 * 形    参 : buf - 存放卡片原始二进制数据的缓冲区
 *            string_buf - 存放转换后字符串的缓冲区（需至少32字节）
 *            length - 卡片原始数据的有效长度
 * 返 回 值 : 无
 * 备    注 : 转换后的字符串为十六进制格式，前缀固定为"ID:"
 *********************************************************************************************************/
static void swiping_card_data_convert_string(char *buf, char *string_buf, int length)
{
	char string[32] = "ID:";
	char temp_buf[5];
	for (int i = 0; i < length; i++)
	{

		sprintf(temp_buf, "%02x", buf[i]);
		strcat(string, temp_buf);
	}
	memset(string_buf, 0, 32);
	strcpy(string_buf, string);
}

/*********************************************************************************************************
 * 函 数 名 : validity_check_card
 * 功能说明 : 校验刷卡数据对应的ID是否在已存储的合法卡片列表中
 * 形    参 : string_buf - 转换后的卡片ID字符串（"ID:xxxx..."格式）
 * 返 回 值 : bool - 校验通过返回true，未匹配到合法卡片返回false
 * 备    注 : 遍历USER_CARD_TOTAL个已存储卡片ID进行比对
 *********************************************************************************************************/
static bool validity_check_card(char *string_buf)
{
	for (int i = 0; i < USER_CARD_TOTAL; i++)
	{ // 因为存入四位数的号码，所以要将其数值从 10000  再*10   => 100000
		if (strcmp(string_buf, get_card_id_data(i)) == 0)
		{
			return true;
		}
	}
	return false;
}

static char *get_temp_card_id_data(int i)
{
	return (char *)SwipingCard.string_buf[i];
}

static bool validity_check_temp_card(char *string_buf)
{
	for (int i = 0; i < 10; i++)
	{
		if (strcmp(string_buf, get_temp_card_id_data(i)) == 0)
		{
			return true;
		}
	}
	return false;
}

/*********************************************************************************************************
 * 函 数 名 : get_room_number
 * 功能说明 : 根据卡片ID字符串匹配对应的房间号和卡片编号并打印
 * 形    参 : string_buf - 转换后的卡片ID字符串（"ID:xxxx..."格式）
 * 返 回 值 : 无
 * 备    注 : 遍历10000个卡片ID数据，通过取余和除法计算房间号与卡片编号
 *********************************************************************************************************/
static void get_room_number(char *string_buf)
{
	for (int i = 0; i < 10000; i++)
	{
		if (strcmp(string_buf, get_card_id_data(i)) == 0)
		{
			int card_id = i % 10;
			int room_numb = (i - card_id) / 10;
			LOG_WHITE("card %d %d\n", room_numb, card_id);
		}
	}
}

/*********************************************************************************************************
 * 函 数 名 : unit_number_exist
 * 功能说明 : 检查指定的单元号是否存在于已配置的单元号列表中
 * 形    参 : unit_number - 待检查的单元号数值
 * 返 回 值 : bool - 单元号存在返回true，不存在返回false
 * 备    注 : 遍历范围由配置项UNIT_NUMBER_INDEX对应的数值决定
 *********************************************************************************************************/
bool unit_number_exist(int unit_number)
{
	for (unsigned char i = 0; i < get_int_conf(UNIT_NUMBER_INDEX); i++)
	{
		if (UserData.unit_number[i] == unit_number)
		{
			return true;
		}
	}
	return false;
}

/*********************************************************************************************************
 * 函 数 名 : save_card_id
 * 功能说明 : 将卡片ID字符串保存到当前房号对应的卡片存储位置
 * 形    参 : string_buf - 转换后的卡片ID字符串（"ID:xxxx..."格式）
 * 返 回 值 : bool - 保存成功返回true，当前房号下10个卡槽已满返回false
 * 备    注 : 房号由CardManageClass.home_id数组的四位数字拼接计算得出
 *********************************************************************************************************/
static bool save_card_id(char *string_buf)
{
	int home_id = CardManageClass.room_card_info.home_id[0] * 1000 +
				  CardManageClass.room_card_info.home_id[1] * 100 +
				  CardManageClass.room_card_info.home_id[2] * 10 +
				  CardManageClass.room_card_info.home_id[3];

	// LOG_WHITE("home_id:[%d]		[%d]\n", home_id, string_buf);

	for (int i = 0; i < 10; i++)
	{

		if (strlen(get_card_id_data(10 * home_id + i)) == 0 && strlen(get_temp_card_id_data(i)) == 0)
		{
			// set_card_id_data(10 * home_id + i, string_buf);
			memset(SwipingCard.string_buf[i], 0, sizeof(SwipingCard.string_buf[i]));
			memcpy(SwipingCard.string_buf[i], string_buf, sizeof(SwipingCard.string_buf[i]));
			LOG_WHITE("card number is %d\n", 10 * home_id + i);

			return true;
		}
	}
	return false;
}

/*********************************************************************************************************
 * 函 数 名 : card_data_connection
 * 功能说明 : 拼接分段接收的卡片数据，确保凑齐12字节完整数据
 * 形    参 : length - 输入：本次接收的数据长度；输出：拼接后的完整数据长度
 *            input_buf - 输入：本次接收的数据缓冲区；输出：拼接后的12字节完整数据
 * 返 回 值 : 无
 * 备    注 : 使用静态变量缓存分段数据，凑齐12字节后重置缓存
 *********************************************************************************************************/
static void card_data_connection(int *length, char *input_buf)
{
	static char buf[12] = {0};
	static unsigned int add_length = 0;

	if (add_length == 0)
	{
		memcpy(buf + add_length, input_buf, *length);
		add_length += *length;
	}
	else
	{
		memcpy(buf + add_length, input_buf, *length);
		add_length += *length;
		if (add_length == 12)
		{
			memset(input_buf, 0, 12);
			memcpy(input_buf, buf, 12);
			*length = 12;
		}
		add_length = 0;
	}
}

/*********************************************************************************************************
 * 函 数 名 : swiping_card_event_detect
 * 功能说明 : 检测刷卡事件并处理不同模式下的业务逻辑（空闲、解锁、添加卡片等）
 * 形    参 : card_fd_serial - 刷卡模块对应的串口文件描述符
 * 返 回 值 : 无
 * 备    注 : 核心刷卡处理函数，包含数据接收、模式切换、音效播放、开门控制等逻辑
 *********************************************************************************************************/
void swiping_card_event_detect(int card_fd_serial)
{

	char data_buf[12] = {0};
	static char string_buf[32] = {0};
	int length;
	struct ak_timeval cur_cpu_time;

	length = uart_receive(card_fd_serial, data_buf, 12, 100);
	if ((length > 0) && (length != 12))
	{
		card_data_connection(&length, data_buf);
	}
	printf_card_data(data_buf, length);

	switch (SwipingCard.mode)
	{
	case CARD_IDLE_MODE:
		if (length == 12)
		{
			standby_timer_reset();
			swiping_card_data_convert_string(data_buf, string_buf, length);
			SwipingCard.mode = CARD_SWIPING_CARD_MODE;
		}
		break;
	case CARD_SWIPING_CARD_MODE:
		if (validity_check_card(string_buf) == true)
		{

			LOG_WHITE("string_buf : %s\n", string_buf);
			get_room_number(string_buf);

			swiping_card_sound_play();
			card_pwd_open_door(true);

			ak_get_ostime(&SwipingCard.unlock_time);
			SwipingCard.mode = CARD_UNLOCK_MODE;
		}
		else
		{
			warn_sound_play();
			ak_get_ostime(&SwipingCard.busy_time);
			SwipingCard.mode = CARD_SWIPING_CARD_PROCESS_MODE;
		}
		break;
	case CARD_ADD_CARD_MODE:
		if (length == 12)
		{
			standby_timer_reset();
			swiping_card_data_convert_string(data_buf, string_buf, length);
			if (validity_check_card(string_buf) == true || validity_check_temp_card(string_buf) == true)
			{
				warn_sound_play();
				ak_get_ostime(&SwipingCard.busy_time);
				SwipingCard.mode = CARD_ADD_CARD_PROCESS_MODE;
			}
			else
			{
				if (save_card_id(string_buf) == true)
				{
					CardManageClass.room_card_info.room_card_num++;
					swiping_card_sound_play();
				}
				else
				{
					warn_sound_play();
				}
				ak_get_ostime(&SwipingCard.busy_time);
				SwipingCard.mode = CARD_ADD_CARD_PROCESS_MODE;
			}
			memset(SwipingCard.string_buf[10], 0, sizeof(SwipingCard.string_buf[10]));
			memcpy(SwipingCard.string_buf[10], string_buf, sizeof(SwipingCard.string_buf[10]));
			SwipingCard.success_show = true;
		}
		break;
	case CARD_UNLOCK_MODE:
		ak_get_ostime(&cur_cpu_time);
		if (ak_diff_ms_time(&cur_cpu_time, &SwipingCard.unlock_time) >= 1000)
		{
			card_pwd_open_door(false);
			SwipingCard.mode = CARD_IDLE_MODE;
		}
		break;
	case CARD_SWIPING_CARD_PROCESS_MODE:
		LOG_WHITE("CARD_BUSY_MODE\n\r");
		ak_get_ostime(&cur_cpu_time);
		if (ak_diff_ms_time(&cur_cpu_time, &SwipingCard.busy_time) >= 300)
		{
			SwipingCard.mode = CARD_IDLE_MODE;
		}
		break;
	case CARD_ADD_CARD_PROCESS_MODE:
		ak_get_ostime(&cur_cpu_time);
		if (ak_diff_ms_time(&cur_cpu_time, &SwipingCard.busy_time) >= 500)
		{
			SwipingCard.mode = CARD_ADD_CARD_MODE;
		}
		break;
	}
}

STR_SwipingCardClass SwipingCard = {
	CARD_IDLE_MODE,
	{0},
	{0},
	false,
	{{0}}};