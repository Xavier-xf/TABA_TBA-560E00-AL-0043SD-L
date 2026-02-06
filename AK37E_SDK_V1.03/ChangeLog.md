# AnyCloud37E SDK V1.03 正式版 更改说明

支持AK37E芯片SDK V1.03正式版本，发布时间2021年8月5日。
主要修改点如下：

## 内核
* 【新增功能】linux支持standby模式，支持唤醒源为GPIO。

* 【新增功能】新增支持两路SPI0_CS拖2两路Slave。

* SPI Nand Flash
  【新增器件】新增支持四款SPI Nand Flash:芯天下：XT26G01C、XT26G02E，旺宏MX：MX35LF2GE4AD、MX35LF1G4AB。

* dts

  【新增功能】SDK编译配置支持MPU。

  【新增功能】支持配置I2S。


## 驱动 
*  【新增功能】音频支持PDM采集。
*  【新增功能】音频支持I2S采集与播放。
*  【新增功能】支持gpio_key功能。
*  【代码优化】dac的设备节点名称改为pcmC0D0p，I2S0播放设备的设备节点名称改为pcmC1D0p。
*  【代码优化】dac loopback设备节点名称改为pcmC0D0l，I2S loopback设备节点名称改为pcmC1D0l。
*  【代码优化】adc的设备节点名称改为pcmC0D0c，PDM采集的设备节点名称改为pcmC2D0c，I2S0采集的设备节点名称改为pcmC1D0c，I2S1的设备名称节点改成pcmC1D1c。
*  【bug修复】解决ATBM6032i tcp协议rx、tx数据差异较大的问题。
*  【bug修复】解决MPU屏驱动异常的问题。


## Bootloader

【新增功能】Uboot支持MPU屏显示。

## 中间件

* 【新增功能】音频支持PDM采集。

* 【新增功能】音频支持I2S采集与播放。

* 【新增功能】增加ak_adec_get_buf_status，可以获取解码缓存区的状态。

* 【新增功能】ai模块增加声音侦测功能的接口。

* 【新增功能】ai模块可以通过ak_ai_set_max_frame_num接口配置最大缓存的帧数量。

* 【新增功能】ai模块可以通过ak_ai_get_max_frame_num接口获取到最大缓存的帧数量。

* 【bug修复】解决用ai_ao.sh脚本煲机一段时间后，出现“pcm frame is too many”打印的问题。

* 【bug修复】解决aenc 运行ak_aenc_sample，概率性出现pcm frame is too many，进程不退出的问题。

* 【bug修复】解决使用venc sample写多帧jpeg格式文件，出现Segmentation fault的问题。



# AnyCloud37E SDK V1.02 正式版 更改说明

支持AK37E芯片SDK V1.02正式版本，发布时间2021年5月14日。
包括以下修改：

* 1、新增支持320*240分辨率MPU屏（驱动IC ILI9342C）；
* 2、新增支持SZ18201；
* 3、新增支持AD芯片TP9950，新增支持BT.656、BT.1120；
* 4、EVB_CBDM_AK376xE_V1.0.2核心板新增支持UART1/UART2/UART3（mipi的dts里面默认打开了uart0/1/2， uart3需要手动参考uart2的配置修改dts）；
* 5、EVB_CBDM_AK376xE_V1.0.2核心板新增支持Camera采集功能；
* 6、EVB_CBDM_AK376xE_V1.0.2核心板新增支持nor+nand的配置；
* 7、解决atbm6031，运行wifi_driver.sh ap大概率性出现wifi加载失败，导致ap开启失败的问题；
* 8、解决网卡1概率出现动态获取不到ip的问题；
* 9、音频优化，支持播放采样率与采集采样率不相同的场景；
* 10、提供normal工作模式下，RTC计时的软件校准方案；
* 11、I2C速率由最高312kbps提升至400kbps。 



# AnyCloud37E SDK V1.01 正式版 更改说明


支持AK37E芯片SDK V1.01正式版本，发布时间2021年3月17日。
包括以下修改：
* 1、支持vi采集，支持MJPEG编码，最大编码分辨率到4096*4096;
* 2、支持 MIPI DSI V1.3接口LCD屏；
* 3、音频输入模块增加在打开ai的情况下，重新配置输入参数的功能接口：int ak_ai_reset_params(int ai_handle_id, struct ak_audio_in_param *param, int frame_length)；
* 4、音频输出模块增加在打开ao的情况下，重新配置输入参数的功能接口：int ak_ao_reset_params(int ao_handle_id, struct ak_audio_out_param *param)；
* 5、音频输出模块增加获取播放buffer状态的接口：int ak_ao_get_buf_status(int ao_handle_id, struct ak_dev_buf_status *buf_status)；
* 6、ao模块支持防啸叫功能接口:int ak_ao_enable_hs(int ao_handle_id, int enable)；
* 7、新增支持JL11X1 PHY；
* 8、新增支持GC0308 sensor。


# AnyCloud37E SDK V1.00 正式版 更改说明

支持AK37E芯片的第一个SDK正式版本，发布时间2020年12月23日。
包括以下功能：

* 1、视频解码支持MJPEG解码以及H264解码;
* 2、视频播放最高支持最高支持1920*1080@30fps解码性能；
* 3、音频支持8000、11025、12000、16000、22050、24000、32000、44100、48000采集以及音频播放功能；
* 4、音频编码及音频解码支持amr、g711a/u、MP3、AAC编解码，如果需要其他格式的编解码可以后续修改编解码库进行增加；
* 5、触摸屏功能支持型号ADT07016BR50-22T；
* 6、双以太网功能；
* 7、wifi支持atbm6031, atbm6032, rtl8188ftv, rtl8189ftv；
* 8、TF卡功能；
* 9、支持USB接口接U盘，读写U盘数据。
