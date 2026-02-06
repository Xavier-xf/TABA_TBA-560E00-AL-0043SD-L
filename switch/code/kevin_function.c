#include ".\include\kevin_function.h"
#include ".\include\API_Macro_MG82F6D64.H"
#include ".\include\interrupt.h"
#include ".\include\drv_iap.h"
#include <string.h>
#include <Intrins.h>

//xiang:
uint8_t dip_switch_value = 0;
uint8_t base_address = 0;
uint8_t prev_base_address = 0;  //记录上一次的分支器地址，用于检测变化
// xiang:读取拨码开关状态并反转位序
void read_dip_switch(void) {
    uint8_t raw_value = 0;
    // xiang:读取8位拨码开关状态 
	raw_value |= (DIP_SW1 ? 0 : 1) << 7;
	raw_value |= (DIP_SW2 ? 0 : 1) << 6;
	raw_value |= (DIP_SW3 ? 0 : 1) << 5;
	raw_value |= (DIP_SW4 ? 0 : 1) << 4;
	raw_value |= (DIP_SW5 ? 0 : 1) << 3;
	raw_value |= (DIP_SW6 ? 0 : 1) << 2;
	raw_value |= (DIP_SW7 ? 0 : 1) << 1;
	raw_value |= (DIP_SW8 ? 0 : 1) << 0;
    
	// xiang:位反转 (MSB->LSB)
 	raw_value &= 0xFE;
    dip_switch_value = reverse_bits(raw_value);  // 直接反转高8位
}

// xiang位反转函数 (0bABCDEFGH -> 0bHGFEDCBA)
u8 reverse_bits(u8 value) {
    u8 result = 0;
	u8 i=0;
    for( i = 0; i < 8; i++) {
        result <<= 1;
        result |= (value & 0x01);
        value >>= 1;
    }
    return result;
}



static void close_all_call_home(void){

	HOME1_RING_GPIO = LOW_LEVEL;
	HOME1_CALL_GPIO = LOW_LEVEL;
	HOME2_RING_GPIO = LOW_LEVEL;
	HOME2_CALL_GPIO = LOW_LEVEL;
	HOME3_RING_GPIO = LOW_LEVEL;
	HOME3_CALL_GPIO = LOW_LEVEL;
	HOME4_RING_GPIO = LOW_LEVEL;
	HOME4_CALL_GPIO = LOW_LEVEL;
}



/***********************************************************************
@Function:    呼叫住宅
@Input:       (HOME_ID)要呼叫的用户 
@Output:      (void)
@Description: 
@Author:      kevin
@Date:        2021-08-27
***********************************************************************/
void call_home(HOME_ID ch){
	switch(ch){
		case HOME_ID1:
			HOME1_RING_GPIO = HIGH_LEVEL;
			HOME1_CALL_GPIO = HIGH_LEVEL;
			break;
		case HOME_ID2:
			HOME2_RING_GPIO = HIGH_LEVEL;
			HOME2_CALL_GPIO = HIGH_LEVEL;
			break;
		case HOME_ID3:
			HOME3_RING_GPIO = HIGH_LEVEL;
			HOME3_CALL_GPIO = HIGH_LEVEL;
			break;
		case HOME_ID4:
			HOME4_RING_GPIO = HIGH_LEVEL;
			HOME4_CALL_GPIO = HIGH_LEVEL;
			break;
	}
	set_timer(TIMER1, TID_1SEC, close_all_call_home);
}


/***********************************************************************
@Function:    打开音视频通道
@Input:       (HOME_ID)要打开的音视频通道  
@Output:      (void)
@Description: 
@Author:      kevin
@Date:        2022-02-17
***********************************************************************/
void open_audio_video_ch(HOME_ID ch){
	switch(ch){
		case HOME_ID1:
			AUDIO_VIDEO_CH1_GPIO = HIGH_LEVEL;
			break;
		case HOME_ID2:
			AUDIO_VIDEO_CH2_GPIO = HIGH_LEVEL;
			break;
		case HOME_ID3:
			AUDIO_VIDEO_CH3_GPIO = HIGH_LEVEL;
			break;
		case HOME_ID4:
			AUDIO_VIDEO_CH4_GPIO = HIGH_LEVEL;
			break;
	}
}


/***********************************************************************
@Function:    关闭音视频通道
@Input:       (HOME_ID)要关闭的音视频通道
@Output:      (void)
@Description: 
@Author:      kevin
@Date:        2022-02-17
***********************************************************************/
void close_audio_video_ch(HOME_ID ch){
	switch(ch){
		case HOME_ID1:
			AUDIO_VIDEO_CH1_GPIO = LOW_LEVEL;
			break;
		case HOME_ID2:
			AUDIO_VIDEO_CH2_GPIO = LOW_LEVEL;
			break;
		case HOME_ID3:
			AUDIO_VIDEO_CH3_GPIO = LOW_LEVEL;
			break;
		case HOME_ID4:
			AUDIO_VIDEO_CH4_GPIO = LOW_LEVEL;
			break;
		case TOTAL_HOME_ID:
			AUDIO_VIDEO_CH1_GPIO = LOW_LEVEL;
			AUDIO_VIDEO_CH2_GPIO = LOW_LEVEL;
			AUDIO_VIDEO_CH3_GPIO = LOW_LEVEL;
			AUDIO_VIDEO_CH4_GPIO = LOW_LEVEL;
			break;
	}
}





/***********************************************************************************
函数名称:   void InitPort(void)
功能描述:   IO配置
输入参数:   
返回参数:     
*************************************************************************************/
void InitPort(void)
{
//设置为数字输入模式
	PORT_SetP0OpenDrain(BIT1);
	UNLOCK_GPIO = HIGH_LEVEL;
	PORT_SetP1OpenDrain(BIT2|BIT4|BIT5|BIT6|BIT7);
	RXD_GPIO = HIGH_LEVEL; 
	DIP_SW1 = HIGH_LEVEL;
	DIP_SW2 = HIGH_LEVEL;
	DIP_SW3 = HIGH_LEVEL;
	DIP_SW4 = HIGH_LEVEL;	
	PORT_SetP2OpenDrain(BIT0|BIT1|BIT2|BIT3|BIT5|BIT6|BIT7);
	POWER_HOME1_GPIO = HIGH_LEVEL;
	POWER_HOME2_GPIO = HIGH_LEVEL;
	POWER_HOME3_GPIO = HIGH_LEVEL;
	DIP_SW5 = HIGH_LEVEL;
	DIP_SW6 = HIGH_LEVEL;
	DIP_SW7 = HIGH_LEVEL;
	DIP_SW8 = HIGH_LEVEL;
	PORT_SetP3OpenDrain(BIT2);
	POWER_HOME4_GPIO = HIGH_LEVEL;
	//设置为强推挽模式
	//PORT_SetP1PushPull(BIT1|BIT3|BIT4|BIT5|BIT6|BIT7);
	PORT_SetP1PushPull(BIT1|BIT3);
	PORT_SetP3PushPull(BIT3|BIT4|BIT5|BIT6|BIT7);  
	PORT_SetP4PushPull(BIT0|BIT1|BIT2);
	
	close_all_call_home();
//	close_audio_video_ch(TOTAL_HOME_ID);
}



static void get_user_data(USER_DATA_ADDR addr, uint8_t *home_id){
	uint8_t i;
	
	for(i = 0 ; i < 5 ;i++){
		home_id[i] = IAP_Read(((addr + i) >> 8) & 0xFF, (addr + i) & 0xFF);
	}

}

static void save_home_id(void){
	uint8_t i;
	
	for(i = 0 ; i < 5 ;i++){
		IAP_Program(((HOME1_ID_ADDR+i) >> 8) & 0xFF, (HOME1_ID_ADDR+i) & 0xFF, UserData.home1_id[i]);		  
		IAP_Program(((HOME2_ID_ADDR+i) >> 8) & 0xFF, (HOME2_ID_ADDR+i) & 0xFF, UserData.home2_id[i]);		  
		IAP_Program(((HOME3_ID_ADDR+i) >> 8) & 0xFF, (HOME3_ID_ADDR+i) & 0xFF, UserData.home3_id[i]);		  
		IAP_Program(((HOME4_ID_ADDR+i) >> 8) & 0xFF, (HOME4_ID_ADDR+i) & 0xFF, UserData.home4_id[i]);	
		IAP_Program(((HOME_ID_DEFAULT_ADDR+i) >> 8) & 0xFF, (HOME_ID_DEFAULT_ADDR+i) & 0xFF, UserData.home_id_default[i]);	  
	}
}


/***********************************************************************
@Function:    保存用户数据
@Input:       (USER_DATA_ADDR)  addr：将要保存的id地址    
			  (uint8_t) *home_id：要保存的数据指针
@Output:      (void)
@Description: 将要保寸的数据保存到IAP区,因为flash数据任何一位只能写0，
              如果需要写1，falsh需要擦除，但是ISP/IAP擦除只支持页擦除，
              一页有512个字节，所以擦除后要将其他数据重新写入IAP。
@Author:      kevin
@Date:        2021-09-06
***********************************************************************/
void save_user_data(USER_DATA_ADDR addr, uint8_t *home_id){
	uint8_t tmp;

	tmp = PageP_Read(IAPLB_P);
	//*1. 	Set IAP Low Boundary Address
	PageP_Write(IAPLB_P, (addr >> 8));				//set IAPLB address at 0x7200
	//*2. 	Set IAP Control Process
	IAP_Erase((addr >> 8), (addr & 0xFF));

	switch(addr){
		case HOME1_ID_ADDR:
			UserData.home1_id[0] = home_id[0];
			UserData.home1_id[1] = home_id[1];
			UserData.home1_id[2] = home_id[2];
			UserData.home1_id[3] = home_id[3];
			break;
		case HOME2_ID_ADDR:
			UserData.home2_id[0] = home_id[0];
			UserData.home2_id[1] = home_id[1];
			UserData.home2_id[2] = home_id[2];
			UserData.home2_id[3] = home_id[3];
			break;
		case HOME3_ID_ADDR:			
			UserData.home3_id[0] = home_id[0];
			UserData.home3_id[1] = home_id[1];
			UserData.home3_id[2] = home_id[2];
			UserData.home3_id[3] = home_id[3];
			break;
		case HOME4_ID_ADDR:
			UserData.home4_id[0] = home_id[0];
			UserData.home4_id[1] = home_id[1];
			UserData.home4_id[2] = home_id[2];
			UserData.home4_id[3] = home_id[3];
			break;
		case HOME_ID_DEFAULT_ADDR:
			UserData.home_id_default[0] = home_id[0];
			UserData.home_id_default[1] = home_id[1];
			UserData.home_id_default[2] = home_id[2];
			UserData.home_id_default[3] = home_id[3];
			break;
	}

	save_home_id();
		
	PageP_Write(IAPLB_P, tmp);
}


void user_data_init(void){
	
	get_user_data(HOME_ID_DEFAULT_ADDR, UserData.home_id_default);
	if(UserData.home_id_default[0] != 0 || UserData.home_id_default[1] != 0xFF ||
		UserData.home_id_default[2] != 0 || UserData.home_id_default[3] != 0xFF)
	{
    	UserData.home1_id[0] = 0;
    	UserData.home1_id[1] = 0xFF;
		UserData.home1_id[2] = 0;
		UserData.home1_id[3] = 0xFF;

		save_user_data(HOME1_ID_ADDR, UserData.home1_id);
		save_user_data(HOME2_ID_ADDR, UserData.home1_id);
		save_user_data(HOME3_ID_ADDR, UserData.home1_id);
		save_user_data(HOME4_ID_ADDR, UserData.home1_id);

		save_user_data(HOME_ID_DEFAULT_ADDR, UserData.home1_id);
	}

	get_user_data(HOME1_ID_ADDR, UserData.home1_id);
	get_user_data(HOME2_ID_ADDR, UserData.home2_id);
	get_user_data(HOME3_ID_ADDR, UserData.home3_id);
	get_user_data(HOME4_ID_ADDR, UserData.home4_id);

}




#define UART1_RX_BUFF_SIZE   64   		 //字节
#define UART1_TX_BUFF_SIZE   64   		 //字节
u8 RcvBuf[UART1_RX_BUFF_SIZE];
u8 TxBuf[UART1_TX_BUFF_SIZE];


static char get_uart_tx_buf(void){
	char tByte;

	tByte = TxBuf[UartClass.tx_out_index];
	UartClass.tx_out_index = (UartClass.tx_out_index + 1) % UART1_TX_BUFF_SIZE;
	return tByte;
}

static void post_uart_rx_buf(u8 tByte){
	RcvBuf[UartClass.rx_in_index] = tByte;
	UartClass.rx_in_index = (UartClass.rx_in_index + 1) % UART1_RX_BUFF_SIZE;
}




/***********************************************************************************
函数名称:   void INT_UART1(void)
功能描述:UART1 中断服务程序
		 
输入参数:   
返回参数:     
*************************************************************************************/
void INT_UART1(void) interrupt INT_VECTOR_UART1
{
	_push_(SFRPI);		   //保护SFRPI寄存器值
	SFR_SetPage(1);		   /*设置1页寄存器*/  //S1CON仅1页操作
	
	if(TI1)					//发送中断内容
	{
		TI1 = 0;	   
		if(UartClass.tx_in_index == UartClass.tx_out_index){
			UartClass.tx_falg = FALSE;
			UartClass.tx_in_index = 0;
			UartClass.tx_out_index = 0;
		}else{
			S1BUF = get_uart_tx_buf();
			UartClass.tx_falg = TRUE;
		}
	}
	if(RI1)					//接收中断内容
	{
		RI1 = 0;				//清除中断标志
		post_uart_rx_buf(S1BUF);
	}
	_pop_(SFRPI);		   //恢复SFRPI寄存器值
}



static void post_uart_tx_buf(u8 tByte){

	TxBuf[UartClass.tx_in_index] = tByte;
	UartClass.tx_in_index = (UartClass.tx_in_index + 1) % UART1_TX_BUFF_SIZE;
}


/***********************************************************************************
函数名称:void Uart1SendByte(u8 tByte)
功能描述:Uart1发送数据
输入参数:u8 tByte: 待发送的数据
返回参数:     
*************************************************************************************/
void Uart1SendByte(u8 tByte)
{	
	post_uart_tx_buf(tByte);
	if(UartClass.tx_falg == FALSE){		
		SFR_SetPage(1);		   /*设置1页寄存器*/  //S1CON仅1页操作
		TI1 = 1;
		SFR_SetPage(0);		   /*设置1页寄存器*/  //S1CON仅1页操作
	}
}

u8 get_uart_rx_buf(u8 length, u8 *buf){
	u8 i;
	for (i = 0; i < length; i++)
	{
		if(UartClass.rx_in_index != UartClass.rx_out_index){
			buf[i] = RcvBuf[UartClass.rx_out_index];
			UartClass.rx_out_index = (UartClass.rx_out_index + 1) % UART1_RX_BUFF_SIZE;
		}else{
			return 0;
		}
	}
	return 1;
}




/***********************************************************************
@Function:    CAN发送命令编码
@Input:       (uint8_t)  cmd：命令       data1-4 ：要发送的数据
@Output:      (void)
@Description: 将要发送的命令数据转化为标准协议
			  起始+长度+命令+数据+校验+结束
@Author:      kevin
@Date:        2021-08-24
***********************************************************************/
void send_can_cmd_encode(uint8_t cmd, uint8_t data1, uint8_t data2, uint8_t data3, uint8_t data4){

	uint8_t length = CMD_LENGTH_BASE;
	uint8_t check_code = 0;
	BOOL data1_null,data2_null,data3_null,data4_null;
	
	if(data1 != CMD_NULL){
		length++;
		data1_null = false;
	}else{
		data1 = 0x00;
		data1_null = true;
	}
	
	if(data2 != CMD_NULL || cmd == CMD_SEND_HOME_ID){
		length++;
		data2_null = false;
	}else{
		data2 = 0x00;
		data2_null = true;
	}
	
	if(data3 != CMD_NULL || cmd == CMD_SEND_HOME_ID){
		length++;
		data3_null = false;
	}else{
		data3 = 0x00;
		data3_null = true;
	}
	
	if(data4 != CMD_NULL){
		length++;
		data4_null = false;
	}else{
		data4 = 0x00;
		data4_null = true;
	}

	check_code = (length + cmd + data1 + data2 + data3 + data4) & 0xFF;
	
	Uart1SendByte(CMD_START);
	Uart1SendByte(length);
	Uart1SendByte(cmd);
	if(data1_null == false){
		Uart1SendByte(data1);
	}
	if(data2_null == false){
		Uart1SendByte(data2);
	}
	if(data3_null == false){
		Uart1SendByte(data3);
	}
	if(data4_null == false){
		Uart1SendByte(data4);
	}
	Uart1SendByte(check_code);
	Uart1SendByte(CMD_END);
	Uart1SendByte(length + 0x02);	
}



/***********************************************************************
@Function:    CAN接收命令解码
@Input:       (uint8_t)  cmd：命令       data1-4 ：接收到的数据
@Output:      (char) return -1:接收到的命令不符合协议
					 return  1：接收的命令符合协议
@Description: 将接收到的数据进行解码，判断符合协议后赋值给命令和数据
@Author:      kevin
@Date:        2021-09-04
***********************************************************************/
char receive_can_cmd_decode(uint8_t *cmd, uint8_t *data1, uint8_t *data2, uint8_t *data3, uint8_t *data4){

	uint8_t length = 0x00;
	uint8_t check = 0x00;
	uint8_t temp_buf[10];

	if(get_uart_rx_buf(1, &temp_buf)){
		//判断如果读到的数据不是“命令开始”,如果不是则返回不取
		if(temp_buf[0] != CMD_START){
			return -1;
		}
	}else{
		return -1;
	}
	//此处延时时间不能更改，否则会接收数据异常
	DelayXms(10);
	//读取数据长度
	if(get_uart_rx_buf(1, &temp_buf) == 0){
		return -1;
	}
	length = temp_buf[0];
	//此处延时时间不能更改，否则会接收数据异常
	DelayXms(10);
	if(get_uart_rx_buf(length, &temp_buf) == 0){
		return -1;
	}
	//判断如果读到的数据最后一位不是“命令结束”，如果不是则返回不取
	if(temp_buf[length - 1] != CMD_END){
		return -1;
	}
	switch(length){
		case 0x03:
			*cmd   = temp_buf[0];
			check = (length + *cmd) & 0xFF;
			break;
		case 0x04:
			*cmd   = temp_buf[0];
			*data1 = temp_buf[1];
			check = (length + *cmd + *data1) & 0xFF;
			break;
		case 0x05:
			*cmd   = temp_buf[0];
			*data1 = temp_buf[1];
			*data2 = temp_buf[2];
			check = (length + *cmd + *data1 + *data2) & 0xFF;
			break;
		case 0x06:
			*cmd   = temp_buf[0];
			*data1 = temp_buf[1];
			*data2 = temp_buf[2];
			*data3 = temp_buf[3];
			check = (length + *cmd + *data1 + *data2 + *data3) & 0xFF;
			break;
		case 0x07:
			*cmd   = temp_buf[0];
			*data1 = temp_buf[1];
			*data2 = temp_buf[2];
			*data3 = temp_buf[3];
			*data4 = temp_buf[4];
			check = (length + *cmd + *data1 + *data2 + *data3 + *data4) & 0xFF;
			break;
	}
	//通过校验位判断数据是否发送正确，如果正确返回1，否则返回-1
	if(check != temp_buf[length - 2]){
		return -1;
	}else{
		return 1;
	}
}



UART_CLASS UartClass =
{
	0,
	0,
	0,
	FALSE,
	0,
	0,
};
	


USER_DATA UserData = 
{
	{0, 0xFF, 0, 0xFF},
	{0, 0xFF, 0, 0xFF},
	{0, 0xFF, 0, 0xFF},
	{0, 0xFF, 0, 0xFF},
	{0, 0xFF, 0, 0xFF}
};


INTERCOM_CLASS Intercom = 
{
	 0,
	{0},
	{0},
	{0},
	{0},
	MONITOR_IDLE,
	UNLOCK_IDLE,
};



