/**
  *
  *参考https://www.cnblogs.com/silencehuan/p/11103074.html
  *
**/
/* 包含的头文件 */
#include <stdio.h>        //标准输入输出,如printf、scanf以及文件操作
#include <stdlib.h>       //标准库头文件，定义了五种类型、一些宏和通用工具函数
#include <unistd.h>       //定义 read write close lseek 等Unix标准函数
#include <fcntl.h>        //文件控制定义
#include <termios.h>      //终端I/O
#include <string.h>       //字符串操作
#include <time.h>         //时间
#include <os_sys_api.h>
#include "ui_api.h"
#include "ak_thread.h"
#include "intercom.h"
#include "ak_common.h"
#include "swiping_card.h"
/// 串口设备
#define DEV_INTERCOM   "/dev/ttySAK2"
#define DEV_CARD   	   "/dev/ttySAK3"

int fd_serial = -1;

/**@brief   设置串口参数：波特率，数据位，停止位和效验位
 * @param[in]  fd           类型  int     打开的串口文件句柄
 * @param[in]  speed        类型  int     波特率
 * @param[in]  number_bits  类型  int     数据位   取值为 7 或者8
 * @param[in]  parity       类型  int     停止位   取值为 1 或者2
 * @param[in]  stop         类型  int     效验类型 取值为N,E,O,,S
 * @return     返回设置结果
 * - 0         设置成功
 * - -1        设置失败
 */
int set_options(int fd, int speed, int number_bits, int parity, int stop)
{
    struct termios new_termios, old_termios;
    // 保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息
    if (tcgetattr(fd, &old_termios) != 0){
        perror("SetupSerial 1");
        return -1;
    }
    //新termios参数清零
    bzero(&new_termios, sizeof(new_termios));
    //CLOCAL--忽略 modem 控制线,本地连线, 不具数据机控制功能, CREAD--使能接收标志
    new_termios.c_cflag |= CLOCAL | CREAD;
    //清数据位标志,设置数据位数
    new_termios.c_cflag &= ~CSIZE;
    switch (number_bits){
        case 7:
            new_termios.c_cflag |= CS7;
            break;
        case 8:
            new_termios.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr, "Unsupported data size\n");
            return -1;
    }
    // 设置校验位
    switch (parity){
        case 'o':
        case 'O':                     //奇校验
            new_termios.c_cflag |= PARENB;
            new_termios.c_cflag |= PARODD;
            new_termios.c_iflag |= (INPCK | ISTRIP);
            break;
        case 'e':
        case 'E':                     //偶校验
            new_termios.c_iflag |= (INPCK | ISTRIP);
            new_termios.c_cflag |= PARENB;
            new_termios.c_cflag &= ~PARODD;
            break;
        case 'n':
        case 'N':                    //无校验
            new_termios.c_cflag &= ~PARENB;
            break;
        default:
            fprintf(stderr, "Unsupported parity\n");
            return -1;
    }
    // 设置停止位
    switch (stop)
    {
        case 1:
            new_termios.c_cflag &= ~CSTOPB;
            break;
        case 2:
            new_termios.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported stop bits\n");
            return -1;
    }
    // 设置波特率
    switch (speed)
    {
        case 2400:
            cfsetispeed(&new_termios, B2400);
            cfsetospeed(&new_termios, B2400);
            break;
        case 4800:
            cfsetispeed(&new_termios, B4800);
            cfsetospeed(&new_termios, B4800);
            break;
        case 9600:
            cfsetispeed(&new_termios, B9600);
            cfsetospeed(&new_termios, B9600);
            break;
        case 19200:
            cfsetispeed(&new_termios, B19200);
            cfsetospeed(&new_termios, B19200);
            break;
        case 38400:
            cfsetispeed(&new_termios, B38400);
            cfsetospeed(&new_termios, B38400);
            break;
        case 57600:
            cfsetispeed(&new_termios, B57600);
            cfsetospeed(&new_termios, B57600);
            break;
        case 115200:
            cfsetispeed(&new_termios, B115200);
            cfsetospeed(&new_termios, B115200);
            break;
        case 230400:
            cfsetispeed(&new_termios, B230400);
            cfsetospeed(&new_termios, B230400);
            break;
        default:
            printf("\tSorry, Unsupported baud rate, set default 9600!\n\n");
            cfsetispeed(&new_termios, B9600);
            cfsetospeed(&new_termios, B9600);
            break;
    }
    // 设置read读取最小字节数和超时时间
    new_termios.c_cc[VTIME] = 0;     	// 读取一个字符等待1*(1/10)s
    new_termios.c_cc[VMIN] = 0;        	// 读取字符的最少个数为1
    tcflush(fd, TCIFLUSH);              //清空缓冲区
    //使能新设置
    if (tcsetattr(fd, TCSANOW, &new_termios) != 0){
        perror("SetupSerial 3");
        return -1;
    }
    printf("Serial set done!\n");
    return 0;
}

/**
 * @brief 串口读取函数
 * @param[in]  fd               打开的串口文件句柄
 * @param[in]  *rcv_buf 接收缓存指针
 * @param[in]  data_len    要读取数据长度
 * @param[in]  timeout     接收等待超时时间，单位ms
 * @return     返回设置结果
 * - >0      设置成功
 * - 其他      读取超时或错误
 */
int uart_receive(int fd, char *rcv_buf, int data_len, int timeout){
    int len, fs_sel;
    fd_set fs_read;
    struct timeval time;
    //set the rcv wait time
    time.tv_sec = timeout / 1000;
    //100000us = 0.1s
    time.tv_usec = timeout % 1000 * 1000;

    //每次循环都要清空集合，否则不能检测描述符变化
    FD_ZERO(&fs_read);
    //添加描述符
    FD_SET(fd, &fs_read);
    // 超时等待读变化，>0：就绪描述字的正数目， -1：出错， 0 ：超时
    fs_sel = select(fd + 1, &fs_read, NULL, NULL, &time);
    //printf("fs_sel = %d\n", fs_sel);
    if (fs_sel){
        len = read(fd, rcv_buf, data_len);
        return len;
    }else{
        // printf("\033[31merr:uart_receive err,%d\033[0m\n", fs_sel);
        return -1;
    }
}

int uart_get(char *rcv_buf, int data_len){
	if(fd_serial < 0){
		return -1;
	}
	
	int timeout = 100;	
	int len, fs_sel;
    fd_set fs_read;
    struct timeval time;
    //set the rcv wait time
    time.tv_sec = timeout / 1000;
    //100000us = 0.1s
    time.tv_usec = timeout % 1000 * 1000;
    //每次循环都要清空集合，否则不能检测描述符变化
    FD_ZERO(&fs_read);
    //添加描述符
    FD_SET(fd_serial, &fs_read);
    // 超时等待读变化，>0：就绪描述字的正数目， -1：出错， 0 ：超时
    fs_sel = select(fd_serial + 1, &fs_read, NULL, NULL, &time);
    //printf("fs_sel = %d\n", fs_sel);
    if (fs_sel){
        len = read(fd_serial, rcv_buf, data_len);
        return len;
    }else{
        // printf("\033[31;1m uart get error! \n\033[0m");
        return -1;
    }
}
/**@brief 串口发送函数
 * @param[in]  fd            打开的串口文件句柄
 * @param[in]  *send_buf     发送数据指针
 * @param[in]  data_len      发送数据长度
 * @return     返回结果
 * - data_len  成功
 * - -1        失败
 */
int uart_send(int fd,  char *send_buf, int data_len){
    ssize_t ret = 0;
    ret = write(fd, send_buf, data_len);
    if (ret == data_len){
        //printf("send data is %s\n", send_buf);
        return ret;
    }else{
        printf("\033[31m write device error\n");
        tcflush(fd, TCOFLUSH);
        return -1;
    }
}
int uart_put( char *send_buf, int data_len){
	if(fd_serial < 0){
		return -1;
	}
	ssize_t ret = 0;

    ret = write(fd_serial, send_buf, data_len);
    if (ret == data_len){
        //printf("send data is %d\n", *send_buf);
        return ret;
    }else{
        printf("\033[31m write device error\n");
        tcflush(fd_serial, TCOFLUSH);
        return -1;
    }
}
static void *intercom_uart_detect_task(void *arg){

    // 打开串口设备
    fd_serial = open(DEV_INTERCOM, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd_serial < 0){
        perror(DEV_INTERCOM);
        return NULL;
    }
    //设置串口阻塞， 0：阻塞， FNDELAY：非阻塞
    //阻塞，即使前面在open串口设备时设置的是非阻塞的
    if (fcntl(fd_serial, F_SETFL, 0) < 0){
        printf("fcntl failed!\n");
    }else{
        printf("fcntl=%d\n", fcntl(fd_serial, F_SETFL, 0));
    }
    if (isatty(fd_serial) == 0){
        printf("standard input is not a terminal device\n");
        close(fd_serial);
        return NULL;
    }else{
        printf("is a tty success!\n");
    }
    printf("fd-open=%d\n", fd_serial);

    // 设置串口参数:设置8位数据位、1位停止位、无校验
    if (set_options(fd_serial, 9600, 8, 'N', 1) == -1) {
        fprintf(stderr, "Set opt Error\n");
        close(fd_serial);
        exit(1);
    }
    //清掉串口缓存
    tcflush(fd_serial, TCIOFLUSH);
    //串口阻塞
    fcntl(fd_serial, F_SETFL, 0);
    //循环读取数据
    while (1){
		intercom_event_detect();
        ak_sleep_ms(10); 
    }
    close(fd_serial);
    ak_thread_exit();
	return NULL;
}
void clean_uart_buff(void){
	if(fd_serial >= 0){
		tcflush(fd_serial, TCIOFLUSH);
	}
}
void intercom_uart_init(void){
    ak_pthread_t pthread_id;
    ak_thread_create(&pthread_id, intercom_uart_detect_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
}



static void *swiping_card_uart_detect_task(void *arg){

    int card_fd_serial = -1;
    // 打开串口设备
    card_fd_serial = open(DEV_CARD, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if (card_fd_serial < 0){
        perror(DEV_CARD);
        return NULL;
    }
    //设置串口阻塞， 0：阻塞， FNDELAY：非阻塞
    //阻塞，即使前面在open串口设备时设置的是非阻塞的
    if (fcntl(card_fd_serial, F_SETFL, FNDELAY) < 0){
        printf("fcntl failed!\n");
    }else{
        printf("fcntl=%d\n", fcntl(card_fd_serial, F_SETFL, 0));
    }
    if (isatty(card_fd_serial) == 0){
        printf("standard input is not a terminal device\n");
        close(card_fd_serial);
        return NULL;
    }else{
        printf("is a tty success!\n");
    }
    printf("fd-open=%d\n", card_fd_serial);

    // 设置串口参数:设置8位数据位、1位停止位、无校验
    if (set_options(card_fd_serial, 9600, 8, 'N', 1) == -1) {
        fprintf(stderr, "Set opt Error\n");
        close(card_fd_serial);
        exit(1);
    }
    //清掉串口缓存
    tcflush(card_fd_serial, TCIOFLUSH);
    //串口阻塞
    fcntl(card_fd_serial, F_SETFL, O_NONBLOCK);
    //循环读取数据

    while (1){
		swiping_card_event_detect(card_fd_serial);
        ak_sleep_ms(10); 
    }
    close(card_fd_serial);
    ak_thread_exit();
	return NULL;
}


void swiping_card_uart_init(void){
    ak_pthread_t pthread_id;
    ak_thread_create(&pthread_id, swiping_card_uart_detect_task, NULL, ANYKA_THREAD_CARD_STACK_SIZE, -1);
}

