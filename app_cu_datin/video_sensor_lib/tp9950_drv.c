#include "sensor_video_api.h"
#include "tp9950_720p_25fps_ahd.h"
#include "tp9950_720p_30fps_ahd.h"
#include "tp9950_1080p_25fps_ahd.h"
#include "tp9950_1080p_30fps_ahd.h"
#include "tp9950_cvbs_ntsc.h"
#include "tp9950_cvbs_pal.h"
#include "i2c_devices_api.h"
#include <stdlib.h>
#include "tp9950_drv.h"
#include "ak_thread.h"
#include <stdio.h>
#include "ak_common.h"
#include "ak_vi.h"
#include "string.h"

#define TP9950_DEVICES 0X44


extern bool video_sensor_open(unsigned char);
extern bool video_sensor_close(void);

static void(*sensor_rx_reset_gpio)(void) = NULL;

static int(*sensor_detection_before_func)(void) = NULL;
static bool(*sensor_write_conf_after_func)(int*,int*,int*) = NULL;

static unsigned char (*tp9950_bright_map)[6] = NULL;
static unsigned char (*tp9950_cont_map)[6] = NULL;
static unsigned char (*tp9950_color_map)[6] = NULL;
static unsigned char tp9950_map_szie = 0;

/************************************
tp9950工作状态,防止其他线程抢占，则不
能将此变量暴露出去，如果外界需要控制
tp9950的工作状态，通过另外一个变量去
控制tp9950的状态
*************************************/
static char tp9950_state = TP9950_STATE_IDLE;//TP9950_STATE_IDLE;
static bool is_open_tp9950 = false;//false;

/***********************************
视频制式，当前制式和上一次制式
************************************/
static unsigned char tp9950_video_format = INVALID_FORMAT;
static int tp9950_time_delay_count = 0;

enum{
    VIDEO_UNPLUG,
    VIDEO_IN,
    VIDEO_LOCKED,
    VIDEO_UNLOCK
};
enum{
	STD_TVI,
	STD_AHD,
	STD_CVI
};
enum{
	MIPI_PAGE,
	VIDEO_PAGE
};
#define FLAG_LOSS         0x80
#define FLAG_H_LOCKED     0x20
#define FLAG_HV_LOCKED    0x60

typedef struct{

	int width;
	int height;
	int fps;
} tp9950_info;

static bool tp9950_write(const i2c_data* data,int len)
{
	return i2c_write(I2C_DEVICES_4,TP9950_DEVICES,data, len);
}
static bool tp9950_read(unsigned char addres,unsigned char* data)
{
	i2c_data reg;
	reg.addres = addres;
	reg.data = 0;
	i2c_read(I2C_DEVICES_4, TP9950_DEVICES,&reg);
	*data = reg.data;
	return true;
}

static void tp9950_detection_printf(unsigned char format){

	char* format_str[7] ={"FORMAT UNKNOW","CVBS-PAL","CVBS-NTSC","AHD-720P-25FPS","AHD-720P-30FPS","AHD-1080P-25FPS","AHD-1080P-30FPS"};
	unsigned char format_type[7] = {INVALID_FORMAT,TP2802_PAL,TP2802_NTSC,TP2802_720P25V2,TP2802_720P30V2,TP2802_1080P25,TP2802_1080P30};

	int i = 0;
	for( i = 0 ; i < 7 ; i++)
	{
		if(format_type[i] == format)
		{
			printf("\n\n\n=====================================:%s\n\n\n",format_str[i]);
			break;
		}
	}
}

static void tp9950_comm_init(void)
{
	i2c_data reg[] = {
		{0x40, 0x00},//default Vin1
		//{0x41, 0x03},
		{0x4c, 0x40},
		{0x4e, 0x00},
		{0x35, 0x25},
		{0xf5, 0x10},
		{0xfd, 0x80},
		{0x38, 0x40},
		{0x3d, 0x60}
	};
	tp9950_write(reg, sizeof(reg)/sizeof(i2c_data));
}



static void tp9950_hw_and_soft_reset(void)
{
	if(sensor_rx_reset_gpio != NULL)
	{
		sensor_rx_reset_gpio();
	}
	tp9950_comm_init();
	
}





static void tp9950_set_reg_page(unsigned char ch)
{
	i2c_data reg[] =
	{
		{0x40, 0x08},
		{0x40, 0x00}
	};
    switch(ch)
    {
	    case MIPI_PAGE:
			tp9950_write(&reg[0], 1);
	        break;
	    default:
			tp9950_write(&reg[1], 1);
	        break;
    }
}



/**********************************
打开avin的通道，用于立马将视频关闭
***********************************/
static void tp9950_channel_set_reg(int ch)
{
	i2c_data reg;
	reg.addres = 0x41;
	switch(ch)
	{
		case SENSOR_VIN_1:
			reg.data = 0x00;
		break;
		case SENSOR_VIN_2:
			reg.data = 0x01;
		break;
		case SENSOR_VIN_3:
			reg.data = 0x02;
		break;
		case SENSOR_VIN_4:
			reg.data = 0x03;
		case SENSOR_VIN_NONE:
		default:
			reg.data = 0x04;
		break;
		
	}
	tp9950_write(&reg, 1);
}
static void tp9950_channel_set(int channel)
{
	i2c_data reg = {0x40, 0x00};
	tp9950_set_reg_page(VIDEO_PAGE);
	tp9950_write(&reg, 1);
	tp9950_channel_set_reg(channel);
}


#define TP9950_TIME_DELAY_MAX 100
static void tp9950_core_close(void){

	/**************************************************
	需要判断信号制式关闭后才能关闭sensor.
	**************************************************/
	//video_sensor_close();
//	tp9950_reset_config();
	tp9950_time_delay_count = 0;
	tp9950_video_format = INVALID_FORMAT;
	
	if(is_open_tp9950 == false)
	{	
		tp9950_channel_set(SENSOR_VIN_NONE);
		tp9950_state = TP9950_STATE_IDLE;
		printf("========================>>sensor idle \n\r");
	}
	else
	{
		tp9950_state = TP9950_STATE_DET;
		if(sensor_detection_before_func != NULL)
		{
			tp9950_channel_set(sensor_detection_before_func());
		}
		else
		{
			tp9950_channel_set(SENSOR_VIN_1);
		}
		video_sensor_close();
		printf("========================>>sensor det \n\r");
	}
}



static void tp9950_reset_default(void)
{
	i2c_data reg[] =
	{
		{0x40, 0x08},
		{0x23, 0x02},
		{0x40, 0x00},
		{0x07, 0xC0},
		{0x0B, 0xC0}
	};	
	tp9950_write(reg, sizeof(reg)/sizeof(i2c_data));

	unsigned char tmp;
	tp9950_read(0x26,&tmp);
	tmp &= 0xfe;
	reg[0].addres = 0x26;
	reg[0].data = tmp;
	tp9950_write(&reg[0],1);
	
	tp9950_read(0xa7,&tmp);
	tmp &= 0xfe;
	reg[0].addres = 0xa7;
	reg[0].data = tmp;
	tp9950_write(&reg[0],1);
	
	tp9950_read(0x06,&tmp);
	tmp &= 0xfb;
	reg[0].addres = 0x06;
	reg[0].data = tmp;
	tp9950_write(&reg[0],1);
	
}

static bool is_tp9950_signal_loss(unsigned char* status)
{
	tp9950_set_reg_page(VIDEO_PAGE);
	tp9950_read(0x01,status);
	return ((*status) & FLAG_LOSS)?true:false;
}

static int tp9950_signale_detection(void){
#define MAX_COUNT 0x20
	
	unsigned int det_count = 0;
	unsigned char det_state = VIDEO_UNPLUG;
	unsigned char det_format = INVALID_FORMAT;

	unsigned char status = 0x00;
	while(det_count < MAX_COUNT){
		
		if(is_tp9950_signal_loss(&status) == true){//no video
			if(0 == det_count){
				tp9950_reset_default();
                det_count++;
			}else{
				if(det_count < MAX_COUNT){
					det_count++;
                }
                continue;
			}
		}else{
			unsigned char flag_locked = FLAG_HV_LOCKED;
			if(flag_locked == (status & flag_locked)){
				
				if(VIDEO_LOCKED == det_state){
					if(det_count < MAX_COUNT){
						det_count++;
					}
				}else if(VIDEO_UNPLUG == det_state){
					det_state = VIDEO_IN;
					det_count = 0;
				}else if(det_format != INVALID_FORMAT){
					det_state = VIDEO_LOCKED;
                    det_count = 0;
				}else{
					if(det_count < MAX_COUNT){
						det_count++;
					}
				}
			}else { //video in but unlocked
				if(VIDEO_UNPLUG == det_state){
					det_state = VIDEO_IN;
                    det_count = 0;
				}else if(VIDEO_LOCKED == det_state){
					det_state = VIDEO_UNLOCK;
                    det_count = 0;
				}else if(VIDEO_LOCKED == det_state){
					det_state = VIDEO_UNLOCK;
					det_count = 0;
				}else{
					if(det_count < MAX_COUNT){
						det_count++;
					}
					if((VIDEO_UNLOCK ==det_state) && (det_count > 2)){
						det_state = VIDEO_IN;
						det_count = 0;
						tp9950_reset_default();
					}
					
				}

			}
		}
		if(VIDEO_IN == det_state){
			unsigned char cvstd = 0x00;
			unsigned char data = 0x00;
			i2c_data reg = {0x2f,0x09};
			tp9950_read(0x03, &cvstd);
            cvstd &= 0x0f;
			
			switch (cvstd&0x07)
			{
				case TP2802_SD:
					tp9950_write(&reg,1);
					ak_sleep_ms(1);
					tp9950_read(0x04,&data);
					if(data==0x94){
						det_format = TP2802_PAL;
					}else if(data==0x93){
						det_format = TP2802_NTSC;
					}else{
						tp9950_write(&reg,1);
						ak_sleep_ms(1);
						tp9950_read(0x04, &data);
						if(data==0x94){
							det_format = TP2802_PAL;
						}else if(data==0x93){
							det_format = TP2802_NTSC;
						}else{ 
							det_format=INVALID_FORMAT;
						}
					}
					goto RETURN_MODE;
					break;
				case TP2802_720P25:
					det_format = TP2802_720P25V2;
					goto RETURN_MODE;
					break;
				case TP2802_720P30:
					det_format = TP2802_720P30V2;
					goto RETURN_MODE;
					break;
				case TP2802_1080P25:
					det_format = TP2802_1080P25;					
					goto RETURN_MODE;
					break;
				case TP2802_1080P30:
					det_format = TP2802_1080P30;					
					goto RETURN_MODE;
					break;
				default:
				//	LeoDebug("not support:%d \r\n",cvstd&0x07);
					break;
			}
        }
    }
RETURN_MODE:
	return det_format;
}

static void tp9950_detection(void)
{
	unsigned char format = tp9950_signale_detection();
	if(format != tp9950_video_format)
	{
		printf("foramt:%d,cur format:%d \n\r",format,tp9950_video_format);
		if((format == INVALID_FORMAT)||(tp9950_video_format != INVALID_FORMAT))
		{
			tp9950_video_format = INVALID_FORMAT;
			tp9950_state = TP9950_STATE_CLOSE;
			printf("------------------------>>start close detection \n\r");
		}
		else
		{
			tp9950_video_format = format;
			tp9950_state = TP9950_STATE_WRITE_CFG;
			printf("------------------------>>start write config \n\r");
		}
	}
}

static bool tp9950_module_write_config(int width,int height,int fps){

	tp9950_info info;
	info.width = width;
	info.height = height;
	info.fps = fps;

	FILE* fp = fopen("/proc/TP9950","wb");
	if(fp == NULL)
	{
		printf("open /proc/tp9950 fail \n\r");
		return false;
	}

	int size = sizeof(tp9950_info);
	if(fwrite(&info,size,1,fp) < 0)
	{
		printf("write /proc/tp9950 fail \n\r");
		fclose(fp);
		return false;
	}
	fclose(fp);
	return true;
}


static void tp9950_config_write(unsigned char mode)
{
	const i2c_data * pdata = NULL;
	int data_len = 0;
	switch(mode){
		case TP2802_PAL:
			pdata  = TP9950_CVBS_PAL_CFG;
			data_len = sizeof(TP9950_CVBS_PAL_CFG)/sizeof(i2c_data);
			tp9950_module_write_config(960,288,50);
		break;
		case TP2802_NTSC:
			pdata  = TP9950_CVBS_NTSC_CFG;
			data_len = sizeof(TP9950_CVBS_NTSC_CFG)/sizeof(i2c_data);		
			tp9950_module_write_config(960,240,60);
		break;
		case TP2802_720P25V2:
			pdata  = TP9950_720P_AHD_25FPS_CFG;
			data_len = sizeof(TP9950_720P_AHD_25FPS_CFG)/sizeof(i2c_data);
			tp9950_module_write_config(1280,720,25);
		break;
		case TP2802_720P30V2:
			pdata  = TP9950_720P_AHD_30FPS_CFG;
			data_len = sizeof(TP9950_720P_AHD_30FPS_CFG)/sizeof(i2c_data);		
			tp9950_module_write_config(1280,720,30);
		break;
		case TP2802_1080P25:
			pdata  = TP9950_1080P_AHD_25FPS_CFG;
			data_len = sizeof(TP9950_1080P_AHD_25FPS_CFG)/sizeof(i2c_data);
			tp9950_module_write_config(960,1080,25);
		break;
		case TP2802_1080P30:
			pdata  = TP9950_1080P_AHD_30FPS_CFG;
			data_len = sizeof(TP9950_1080P_AHD_30FPS_CFG)/sizeof(i2c_data);
			tp9950_module_write_config(960,1080,30);
		break;
		
	}
	if((pdata == NULL)||(data_len == 0)){
		return ;
	}
	tp9950_write(pdata,data_len);
	tp9950_detection_printf(mode);
#if 0
	if(mode == TP2802_1080P30){
		for(int i = 0 ; i < 0xFF ; i++){
			unsigned char data = 0;
			tp9950_read(i,&data);
			printf("%02x:%02x\n\r",i,data);
		}
	}
#endif
}

/****************************************
画面调整:
 调整机制:
	根据当前通道，获取其亮度，对比度，色
度的相对值（-10 10)
	根据当前制式，获取其绝对的默认值，根
据相对值后再算出其绝对值写入到寄存器。
备注：根据其相对值值，最终有一个步宽的设置
用于设置每次减1后，实际减的数值可以使其的
系数倍。
****************************************/
static unsigned char bright_volume_base_get(void)
{
	switch(tp9950_video_format)
	{
		case TP2802_PAL:		return 0x7F;
		case TP2802_NTSC: 		return 0xf8;
		case TP2802_720P25V2: 	return 0x00;
		case TP2802_720P30V2: 	return 0x14;
		case TP2802_1080P25: 	return 0x00;
		case TP2802_1080P30: 	return 0x00;
	}
	return 0x00;
}
static unsigned char const_voluem_base_get(void)
{
	switch(tp9950_video_format)
	{
		case TP2802_PAL:		return 0x40;
		case TP2802_NTSC: 		return 0x50;
		case TP2802_720P25V2: 	return 0x40;
		case TP2802_720P30V2: 	return 0x54;
		case TP2802_1080P25: 	return 0x40;
		case TP2802_1080P30: 	return 0x40;
	}
	return 0x00;
}
static unsigned char color_voluem_base_get(void)
{
	switch(tp9950_video_format)
	{
		case TP2802_PAL:		return 0x60;
		case TP2802_NTSC: 		return 0x60;
		case TP2802_720P25V2: 	return 0x40;
		case TP2802_720P30V2: 	return 0x40;
		case TP2802_1080P25: 	return 0x60;
		case TP2802_1080P30: 	return 0x60;
	}
	return 0x00;
}


static unsigned char bright_map_volume_get(unsigned char vol)
{
	switch(tp9950_video_format)
	{
		case TP2802_PAL: 		return tp9950_bright_map[vol][0];
		case TP2802_NTSC: 		return tp9950_bright_map[vol][1];
		case TP2802_720P25V2: 	return tp9950_bright_map[vol][2];
		case TP2802_720P30V2: 	return tp9950_bright_map[vol][3];
		case TP2802_1080P25: 	return tp9950_bright_map[vol][4];
		case TP2802_1080P30: 	return tp9950_bright_map[vol][5];
	}
	return 0x00;
}
static unsigned char cont_map_volume_get(unsigned char vol)
{
	switch(tp9950_video_format)
	{
		case TP2802_PAL: 		return tp9950_cont_map[vol][0];
		case TP2802_NTSC: 		return tp9950_cont_map[vol][1];
		case TP2802_720P25V2: 	return tp9950_cont_map[vol][2];
		case TP2802_720P30V2: 	return tp9950_cont_map[vol][3];
		case TP2802_1080P25: 	return tp9950_cont_map[vol][4];
		case TP2802_1080P30: 	return tp9950_cont_map[vol][5];
	}
	return 0x00;
}
static unsigned char color_map_volume_get(unsigned char vol)
{
	switch(tp9950_video_format)
	{
		case TP2802_PAL: 		return tp9950_color_map[vol][0];
		case TP2802_NTSC: 		return tp9950_color_map[vol][1];
		case TP2802_720P25V2: 	return tp9950_color_map[vol][2];
		case TP2802_720P30V2: 	return tp9950_color_map[vol][3];
		case TP2802_1080P25: 	return tp9950_color_map[vol][4];
		case TP2802_1080P30: 	return tp9950_color_map[vol][5];
	}
	return 0x00;
}

void display_bright_adj(int bright)
{
	int vol = 0;
	if((bright < tp9950_map_szie)&&(tp9950_bright_map != NULL))
	{
		vol = bright_map_volume_get(bright);
	}
	else
	{
		unsigned char base = bright_volume_base_get();
	 	vol = base + (bright - 10)*2;
	}
	
	i2c_data reg;
	reg.addres = 0x10;
	reg.data = vol;
	tp9950_write(&reg, 1);
}
void display_const_adj(int cont)
{
	int vol = 0;
	if((cont < tp9950_map_szie)&&(tp9950_cont_map != NULL))
	{
		vol = cont_map_volume_get(cont);
	}
	else
	{
		unsigned char base = const_voluem_base_get();
		vol = base + (cont - 10)*2;
	}
	i2c_data reg;
	reg.addres = 0x11;
	reg.data = vol;
	tp9950_write(&reg, 1);
}
void display_color_adj(int color)
{
	int vol = 0;
	if((color < tp9950_map_szie)&&(tp9950_color_map != NULL))
	{
		vol = color_map_volume_get(color);
	}
	else
	{
		unsigned char base = color_voluem_base_get();
		vol = base + (color - 10)*2;
	}
	i2c_data reg;
	reg.addres = 0x12;
	reg.data = vol;
	tp9950_write(&reg, 1);
}






static void tp9950_write_cfg(void)
{
#if 0
	/*************************
	先判断信号是否符合要求
	**************************/
	unsigned char format = tp9950_signale_detection();
	if((format == INVALID_FORMAT)||(format != tp9950_video_format)){
		if(tp9950_time_delay_count++ > TP9950_TIME_DELAY_MAX){
			
			tp9950_state = TP9950_STATE_CLOSE;
			LeoDebug("------------------------>>start close detection \n\r");
		}
		return ;
	}
#endif	
	tp9950_config_write(tp9950_video_format);
	if(sensor_write_conf_after_func != NULL)
	{
		int brightness,cont,color;
		if(sensor_write_conf_after_func(&brightness,&cont,&color) == true)
		{
			display_bright_adj(brightness);
			display_const_adj(cont);
			display_color_adj(color);
		}
	}
	
	//display_adjust_write_cfg(monitor_channel_get());
	tp9950_state = TP9950_STATE_WAIT_HV;
	tp9950_time_delay_count = 0;
	printf("------------------------>>start wait HV \n\r");
}

/***********************
判断当前信号是否稳定
************************/
static bool tp9950_hvsync_check(void)
{

	tp9950_set_reg_page(VIDEO_PAGE);
	unsigned char status = 0x00;
	tp9950_read(0x01,&status);
	if((status&0x60) == 0x60)
	{
		return true;
	}
	return false;
}

static void tp9950_check_hv(void){

#if 0
	/***********************************************
	先判断信号是否符合要求,写入屏参后可能数据不稳定
	************************************************/
	unsigned char format = tp9950_signale_detection();
	if((format == INVALID_FORMAT)||(format != tp9950_video_format)){
		if(tp9950_time_delay_count++ > TP9950_TIME_DELAY_MAX){
					
			tp9950_state = TP9950_STATE_CLOSE;
			LeoDebug("------------------------>>start close detection \n\r");
			return ;
		}
	}
#endif
	if((tp9950_hvsync_check() == false)&&(tp9950_time_delay_count++ > TP9950_TIME_DELAY_MAX))
	{
		tp9950_state = TP9950_STATE_CLOSE;
		return;
	}
	tp9950_time_delay_count = 0;
	tp9950_state = TP9950_STATE_OPEN_CORE;
	printf("------------------------>>start open core \n\r");
}

static void check_tp9950_state(void){
	
	/*****************************************************
	主要根据外部环境接口(is_open_tp9950)去改变tp9950的
	工作状态(tp9950_state).
		例如：外部将tp9950关闭，则必须将tp9950_state置
	位到空闲状态，但是空闲之前，需要先将core,关闭.tp9950
	工作复位。然后才能进入空闲状态
		如果外部环境将tp9950打开，则需要将tp9950_state 
	置位到检测状态，用于检测，稳定HV,打开isp，等一系列
	动作
	*******************************************************/
	if(is_open_tp9950 == false)
	{
		if(tp9950_state != TP9950_STATE_IDLE)
		{
			tp9950_state = TP9950_STATE_CLOSE;
		}
	}
	else if(tp9950_state == TP9950_STATE_IDLE)
	{
		tp9950_state = TP9950_STATE_CLOSE;
	}
}



static void* tp9950_task(void*arg)
{
	printf("sensor ad drv pthread success \n\r");
	while(1)
	{
		/***********************************
		tp9950状态必须在这个线程里面去修改
		***********************************/
		switch(tp9950_state)
		{
			/***********************
			不需要侦测视频信号。
			************************/
			case TP9950_STATE_CLOSE:
			{
				tp9950_core_close();
			}
			break;
			/***********************
			侦测视频信号。
			************************/
			case TP9950_STATE_DET:
			{
				tp9950_detection();
				if(tp9950_state == TP9950_STATE_DET){
					ak_sleep_ms(200);
				}
			}
			break;
			case TP9950_STATE_WRITE_CFG:
			{
				tp9950_write_cfg();
			}
			break;
			/***********************
			等待视频信号的行场信号稳定
			************************/
			case TP9950_STATE_WAIT_HV:
			{
				tp9950_check_hv();
			}
			break;
			/***********************
			打开37d的ISP设备
			************************/
			case TP9950_STATE_OPEN_CORE:
			{
				video_sensor_open(tp9950_video_format);
				tp9950_state = TP9950_STATE_DET;
			}
			break;
			default:
			break;
		}
		/*********************
		根据外部环境接口判断是
		否需要关闭tp9950
		**********************/
		check_tp9950_state();
		ak_sleep_ms(10);
	
	}
	ak_thread_exit();
	return NULL;
}


bool tp9950_init(void(*reset_callback_func)(void),
						int(*detection_before_callback)(void),
						bool(*write_conf_after_callback)(int*,int*,int*),
						unsigned char (* bright_map)[],
						unsigned char (* cont_map)[],
						unsigned char (* color_map)[],
						int map_size)
{
	ak_pthread_t thread_id;
	
	i2c_init(I2C_DEVICES_4);
	
	sensor_rx_reset_gpio = reset_callback_func;
	sensor_detection_before_func = detection_before_callback;
	sensor_write_conf_after_func = write_conf_after_callback;
	if(map_size > 0)
	{
		tp9950_bright_map = bright_map;
		tp9950_cont_map = cont_map;
		tp9950_color_map = color_map;
		tp9950_map_szie = map_size;
	}
	tp9950_hw_and_soft_reset();
	
	ak_thread_create(&thread_id, tp9950_task, NULL ,ANYKA_THREAD_NORMAL_STACK_SIZE,-1);
	return true;
}



unsigned char tp9950_state_get(void)
{
	return tp9950_state;
}

unsigned char tp9950_format_get(void)
{
	return tp9950_video_format;
}



static void tp9950_wait_idle(void)
{
	int time_count = 1000;
	while(time_count--)
	{
		if(tp9950_state == TP9950_STATE_IDLE)
		{
			break;
		}
		ak_sleep_ms(1);
	}
}

void tp9950_close(void)
{
	is_open_tp9950 = false;
	tp9950_wait_idle();
}

void tp9950_open(void)
{
	is_open_tp9950 = true;
}


