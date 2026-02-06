#if 0
#include "stdlib.h"
#include "fcntl.h"
#include <dirent.h>
#include "string.h"
#include <sys/stat.h>
#include "stdio.h"
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define FB_UPGRADE_DEV "/dev/fb0"

#define UPGRADE_NONE        	0X00
#define UPGRADE_FIRMWARE_FLG    0X01
#define UPGRADE_PHOTO_FLG       0X10
static unsigned char upgrade_flag = UPGRADE_NONE;
static unsigned char cur_upgrade_flag = UPGRADE_NONE;

static int upgrade_total = 0;
static int upgrade_count = 1;


#define MMCBLK0_PATH 	 "/dev/mmcblk0"
#define SDCARD_PATH		 "/mnt/tf/"
#define DST_PATH		"/app/app/"

#define FIRMWARE_SRC_PATH    SDCARD_PATH"ANYKA37EOS"
#define FIRMWARE_DST_PATH	 DST_PATH"ANYKA37EOS"


#define APP_NAME				"ANYKA37E.BIN"
#define FIRMWARE__TMP_APP		"/tmp/ANYKA37EOS"
#define APP_DST_PATH	 		DST_PATH"ANYKA37E.BIN"
#define APP_TMP_APP				DST_PATH"ANYKA37E.BIN"//"/tmp/ANYKA37E.BIN"

#define PHOTO_SRC_PATH   	 "/etc/config/photo/"
#define PHOTO_BACKUP_PATH 	SDCARD_PATH"backup/"

#define USER_DATA_PATH       "/app/app/data/"

static int backup_photo_total = 0;
static int copy_ring_total = 0;

static int check_sdcard_insert(void){

	return (access(MMCBLK0_PATH,F_OK) == 0)?1:0;
}


static void mount_sdcard(void){

	mkdir(SDCARD_PATH,0777);
    system("mount "MMCBLK0_PATH" "SDCARD_PATH);

	/*****************
	创建backup文件夹
	******************/
	if(access(PHOTO_BACKUP_PATH,F_OK) != 0){
		mkdir(PHOTO_BACKUP_PATH,0777);
	}	
    system("sync");
}


static int check_upgrade_firmware(void){

	if(access(FIRMWARE_SRC_PATH,F_OK) == 0){
	
		upgrade_flag |= UPGRADE_FIRMWARE_FLG;
		upgrade_total ++;
		printf("find firmware \n\r");
		return 1;
	}
	return 0;
}


static int check_file_is_photo(const char* file){

	char* dot = strrchr(file,'.')+1;
	if(strncasecmp(dot,"JPG",3) == 0){
		return 1;
	}
	return 0;
}

static int check_photo_src(void){

	char cmd_buffer[64] = {0};
	sprintf(cmd_buffer,"find %s -type f",PHOTO_SRC_PATH);
	
	FILE* pf = popen(cmd_buffer,"r");
	char buffer[128] = {0};
	while(fgets(buffer,128,pf)){
		if(check_file_is_photo(buffer) == 1){
			backup_photo_total++;
		}
		memset(buffer,0,sizeof(buffer));
	}
	pclose(pf);
	return backup_photo_total?1:0;	
}

static int check_backup_photo(void){

	if(access(PHOTO_SRC_PATH,F_OK) == 0){
		if(check_photo_src() == 0){
			return 0;
		}
		//upgrade_flag |= UPGRADE_PHOTO_FLG;
		//upgrade_total++;
		printf("find photo \n\r");

		system("cp -r "PHOTO_SRC_PATH" "PHOTO_BACKUP_PATH);
		printf("Backup flash photo finish... \n\r");
		return 1;
	}
	return 0;
}



static int upgrade_load(void){

	int reslut = 0;
	reslut = check_upgrade_firmware();
	//reslut |= check_backup_photo();
	return reslut;
}



static int fb_fd = -1;
static long fb_size = 0;
static unsigned char* fb_addres = NULL;
static int srceen_width = 0;
static int srceen_height = 0;


static void draw_rect(int x,int y,int w,int h,int color){

	unsigned char* dst_addr = fb_addres + y*srceen_width*3 + x*3;
	unsigned char r = (color>>16)&0xFF;
	unsigned char g = (color>>8)&0xFF;
	unsigned char b = (color)&0xFF;
	int j,i;
	int line_size = w*3;
	for( j=0; j < h ; j++){
		
		for(i = 0 ; i < line_size ; i+=3){
			
			dst_addr[j*srceen_width*3 + i] = r;
			dst_addr[j*srceen_width*3 + i+1] = g;
			dst_addr[j*srceen_width*3 + i+2] = b;
		}
	}
}


static void draw_progress_bg(void){

	draw_rect(90,132,300,1,0x2693FF);
    draw_rect(389,132,1,10,0x2693FF);
    draw_rect(90,132,1,10,0x2693FF);
    draw_rect(90,141,300,1,0x2693FF);
}

static int fb_open(void){
	
	/**************
	设置fb的分辨率
	***************/
	system("fbset -fb /dev/fb0 -g 480 272 480 272 24");

	fb_fd = open(FB_UPGRADE_DEV,O_RDWR);
	if(fb_fd < 0){
		printf("----------->>>open %s fail \n\r",FB_UPGRADE_DEV);
		return 0;
	}

	struct fb_fix_screeninfo finfo;
	ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
	
	struct fb_var_screeninfo vinfo;
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);

	long linesize = vinfo.xres*vinfo.bits_per_pixel/8;
	fb_size = linesize*vinfo.yres;

	srceen_width = vinfo.xres;
	srceen_height = vinfo.yres;
	printf("srceen width:%d,height:%d \n\r",srceen_width,srceen_height);
	fb_addres = (unsigned char*)mmap(0,finfo.smem_len,PROT_READ|PROT_WRITE,MAP_SHARED,fb_fd,0);	
	memset(fb_addres,0x00,fb_size);
	draw_progress_bg();
	printf("srceen addres :%p size:%d\n\r",fb_addres,finfo.smem_len);
	return 1;
}
static int fb_close(void){
	if((fb_fd == -1)||(fb_addres == NULL)){
		goto EXIT;
	}
	munmap(fb_addres,fb_size);
	close(fb_fd);
EXIT:
	//ak_sdk_exit();
	return 1;
}

static void printf_progress(int per){

	unsigned char str_buffer[128] = {0};
	int str_len = 0;
	if(cur_upgrade_flag ==  UPGRADE_FIRMWARE_FLG){
		sprintf(str_buffer,"upgrade firmware:%d%%",per);
	}else if(cur_upgrade_flag == UPGRADE_PHOTO_FLG){
		sprintf(str_buffer,"backup Photo:%d%%",per);
	}
	printf("%s",str_buffer);	
	str_len = strlen(str_buffer);
	while(str_len--){
		putc('\b',stdout);
	}
	fflush(stdout);
}


static int draw_progress(int per){

	int upgrade_per= upgrade_count*294/upgrade_total;
	int width = upgrade_per*per/100;
	if(width>294){
		printf("error :w = %d per=%d count=%d total=%d\n",width,per,upgrade_count,upgrade_total);
		return 0;
	}
	draw_rect(93, 134, width, 6, 0x2693FF);
	printf_progress(per);
	return 0;
}

static int file_size_get(FILE*fp){

	fseek(fp,0,SEEK_END);
    int file_size = ftell(fp);
    fseek(fp,0,SEEK_SET);
	return file_size;
}

static void upgrade_firmware_start(void){

	int file_size =0;
	int read_size = 0;
	int read_len = 0;
	unsigned char read_buffer[10*1024] = {0};

	if((upgrade_flag&UPGRADE_FIRMWARE_FLG) == 0x00){
		return ;
	}
	FILE* src_fp = NULL;
	FILE* dst_fp = NULL;
	/*********************************
	打开升级包，判断是否存在有升级包
	*********************************/
	src_fp = fopen(FIRMWARE_SRC_PATH,"r");
    if(src_fp == NULL){
        printf("open %s fail \n\r",FIRMWARE_SRC_PATH);
        goto EXIT;
    }
	/*******************
	判断是否有旧的升级包
	*******************/
    if(access(FIRMWARE_DST_PATH,F_OK) == 0){
        remove(FIRMWARE_DST_PATH);
    }
    
    dst_fp = fopen(FIRMWARE_DST_PATH,"w");
    if(dst_fp == NULL){
        printf(" open %s fail \n\r",FIRMWARE_DST_PATH);
        goto EXIT;
    }

	file_size = file_size_get(src_fp);

	/************************
	清屏和重置升级标志
	************************/	
	cur_upgrade_flag = UPGRADE_FIRMWARE_FLG;
	while((read_len = fread(read_buffer,1,10*1024,src_fp)) > 0){
		
		fwrite(read_buffer,read_len,1,dst_fp);
		read_size += read_len;
		/*******************
		当前文件占用的百分比
		********************/
		draw_progress(read_size*100/file_size);
	}
EXIT:
	if(src_fp != NULL){
		fclose(src_fp);
	}
	if(dst_fp != NULL){
		fclose(dst_fp);
	}
	char chmod_buffer[128] = {0};
	sprintf(chmod_buffer,"chmod -R 777 %s",FIRMWARE_DST_PATH);
	system(chmod_buffer);
	system("sync");
	upgrade_count++;
}

static void upgrade_start(void){

	upgrade_firmware_start();
	putc('\n',stdout);fflush(stdout);
}



static void run_system_bin(void){

	char buffer[128] = {0};
	sprintf(buffer,"cp %s %s",FIRMWARE_DST_PATH,FIRMWARE__TMP_APP);
	printf("%s\n\r",buffer);
	system(buffer);

	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"tar -xvf %s -C /tmp",FIRMWARE__TMP_APP);
	printf("%s\n\r",buffer);
	system(buffer);
	system(APP_TMP_APP);
}

static void delete_user_data(void){
	if(access(USER_DATA_PATH,F_OK) == 0){
		system("rm -rf" USER_DATA_PATH);
	}
}

static void upgrade_pramer_init(void){

	system("insmod /usr/modules/ak_fb.ko");	
	system("insmod /usr/modules/ak_gui.ko");	
	
	if(access(PHOTO_SRC_PATH,F_OK) != 0){
		mkdir(PHOTO_SRC_PATH,0777);
	}	
    system("sync");
}

static void upgrade_remove_tmp_cache(void)
{
	system("rm -rf /tmp/*");
	system("echo 1 > /proc/sys/vm/drop_caches");
	system("echo 2 > /proc/sys/vm/drop_caches");
	system("echo 3 > /proc/sys/vm/drop_caches");
}
int main(int argc,char** argv){

	upgrade_pramer_init();


	/********************
	检测SDcard 是否插入
	*********************/
	if(check_sdcard_insert() == 0){
		goto EXIT;
	}

	/********************
	挂载SDcard
	*********************/
	mount_sdcard();

	upgrade_remove_tmp_cache();
	/***************
	加载更新的数据包
	*****************/
	if(upgrade_load() == 0){
		goto EXIT;
	}
	
	check_backup_photo();
	
	if(fb_open() == 0){
		goto EXIT;
	}
	/***************
	开始升级
	*****************/
	printf("start upgrade firmware\n\r");
	upgrade_start();
	delete_user_data();

EXIT:
	fb_close();

	run_system_bin();
	exit(0);
}
#endif

#if 1

#include <stdbool.h>
#include <linux/fb.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>


#define FB_PATH "/dev/fb0"
#define FIRMWARE_PATH "/mnt/.upgrade/tmp/"

static int upgrade_total = 0;
static int upgrade_count = 0;

static int h_size = 0;//水平宽度
static int v_size = 0;//垂直宽度

static unsigned char *fb_adder = NULL;
static struct fb_var_screeninfo var_info;
static struct fb_fix_screeninfo fix_info;
static u_int32_t process_color = 0x00FFFF;


static bool fb_init(void)
{
	system("insmod /usr/modules/ak_fb.ko");
	system("insmod /usr/modules/ak_gui.ko");
	int fd = open(FB_PATH, O_RDWR);
	if (fd < 0)
	{
		printf("open %s failed \n", FB_PATH);
		exit(0);
	}
	/***** 获取fb的相关的信息 *****/

	ioctl(fd, FBIOGET_VSCREENINFO, &var_info);

	var_info.activate |= FB_ACTIVATE_FORCE;
	var_info.activate |= FB_ACTIVATE_NOW;
	var_info.xres = var_info.xres_virtual;
	var_info.yres = var_info.yres_virtual;
	/***** 设置RGB565格式 *****/
	var_info.bits_per_pixel = 24; // 16;
	var_info.red.offset = 16;     // 11;
	var_info.red.length = 8;      // 5;
	var_info.green.offset = 8;    // 5;
	var_info.green.length = 8;    // 6;
	var_info.blue.offset = 0;     // 0;
	var_info.blue.length = 8;     // 5;
	ioctl(fd, FBIOPUT_VSCREENINFO, &var_info);
	printf("X:%d  Y:%d  bbp:%d  \n", var_info.xres, var_info.yres, var_info.bits_per_pixel);
	h_size = var_info.xres;
	v_size = var_info.yres;
	/***** 获取不可变参数 *****/
	ioctl(fd, FBIOGET_FSCREENINFO, &fix_info);

	fb_adder = (unsigned char *)mmap(0, fix_info.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if(fb_adder == NULL)
	{
		printf("can't mmap Framebuffer\n");
		exit(0);
	}

	memset((void *)fb_adder, 0x00, fix_info.smem_len);

	var_info.reserved[0] = 0;

	ioctl(fd, FBIOPUT_VSCREENINFO, &var_info);
	return true;
}


static void fb_rect_draw(int x, int y, int w, int h, char r, char g, char b)
{
	unsigned char *addr = fb_adder + y * h_size * 3 + x * 3;
	for (int j = 0; j < h; j++)
	{
		for (int i = 0; i < w; i++)
		{
			addr[i * 3 + 2] = r;
			addr[i * 3 + 1] = g;
			addr[i * 3] = b;
		}
		addr += h_size * 3;
	}
}

static void fb_rect_init(void)
{
	/***** x:394 y:288 w:13 h:704 *****/

	fb_rect_draw(h_size / 6, v_size / 2 - 7, h_size / 6 * 4, 2, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
	fb_rect_draw(h_size / 6, v_size / 2 + 7, h_size / 6 * 4, 2, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
	fb_rect_draw(h_size / 6, v_size / 2 - 7, 2, 14, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
	fb_rect_draw(h_size / 6 * 5 - 2, v_size / 2 - 7, 2, 14, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
}


static void upgrade_fb_progress_display(void)
{
	fb_rect_draw(h_size / 6 + 3, v_size / 2 - 4, upgrade_total ? (upgrade_count * (h_size / 6 * 4 - 6) / upgrade_total) : h_size / 6 * 4 - 6, 10, (char)(process_color >> 16), (char)(process_color >> 8), (char)(process_color));
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        printf("请输入参数，如：[./upgrade_progress 10 1]\n");
        return 0;
    }

    fb_init();

    process_color = 0x00FFFF;

    // process_color = 0xFF8000;

	fb_rect_init();

    upgrade_total = atoi(argv[1]);

    upgrade_count = atoi(argv[2]);

	// printf("=========>>> total:[%d] count:[%d]\n", upgrade_total, upgrade_count);

	upgrade_fb_progress_display();

	int fd = open("/tmp/proc_value", O_RDONLY);

	int proc_value = 0;

	char buffer[4] = {0};

	while(1)
	{
		usleep(500 * 1000);
		memset(buffer, 0, sizeof(buffer));
		lseek(fd, 0, SEEK_SET);
		int res = read(fd, buffer, sizeof(buffer));
		if (res < 0)
		{
			return 1;
		}
		else if(res == 0)
		{
			continue;
		}
		else
		{
			proc_value = atoi(buffer);
			if(proc_value > upgrade_total || proc_value < upgrade_count)
			{
				continue;
			}
			upgrade_count = proc_value;
		}
		upgrade_fb_progress_display();
	}

    return 0;
}
#endif