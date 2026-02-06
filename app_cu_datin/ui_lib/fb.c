#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <linux/fb.h>
#include "ak_mem.h"
#include <stdbool.h>
#include "ui_api.h"
#include "ak_tde.h"
#include "ak_thread.h"
#include "ui.h"

#define FB_DEV "/dev/fb0"														  // 帧缓冲设备节点路径
#define FB_BUFFER_A 0															  // 帧缓冲双缓冲标识-A缓冲区
#define FB_BUFFER_B 1															  // 帧缓冲双缓冲标识-B缓冲区
#define FB_BUFFER_GET ioctl(fb_device_fd, FBIOGET_VSCREENINFO, &fb_device_vinfo)  // 获取fb可变参数
#define FB_BUFFER_SET ioctl(fb_device_fd, FBIOPUT_VSCREENINFO, &fb_device_vinfo); // 设置fb可变参数
#define FB_BUFFER fb_device_vinfo.reserved[0]									  // 当前激活的fb缓冲区标识

static char fb_rotation = 0;					 // 屏幕旋转角度（0/90/180度）
static ak_mutex_t fb_mutex;						 // 帧缓冲操作互斥锁，保护fb临界资源
static int fb_device_fd = -1;					 // 帧缓冲设备文件描述符
static struct fb_fix_screeninfo fb_device_finfo; // fb固定屏幕信息结构体（显存物理地址/长度等）
static struct fb_var_screeninfo fb_device_vinfo; // fb可变屏幕信息结构体（分辨率/色深等）
static unsigned char *fb_addres_base = NULL;	 // fb虚拟地址基址
static unsigned long fb_phyaddres_base = 0;		 // fb物理地址基址
static int screen_size = 0;						 // 屏幕显存总大小（字节）
static unsigned char *gui_buffer_addres = NULL;	 // GUI层DMA缓存地址（ARGB8888格式）
static unsigned char *bg_buffer_addres = NULL;	 // 背景层DMA缓存地址（BGR888格式）
static unsigned char *bg_temp_buffer = NULL;	 // 背景临时缓存地址（用于背景备份/恢复）
static bool fb_refresh_disable = false;			 // fb刷新禁用标志（true=禁用刷新，false=允许）
static bool is_bg_visiable = false;				 // 背景层可见标志

extern void *osal_fb_mmap_viraddr(int fb_len, int fb_fd);
bool jpeg_decodec(jpeg_decodec_info *info);

static vector screen_vector; // 屏幕分辨率向量（宽/高，适配旋转后尺寸）

/*********************************************************************************************************
 * 函 数 名 : screen_vector_get
 * 功能说明 : 获取屏幕分辨率向量（适配旋转后的实际宽高）
 * 形    参 : 无
 * 返 回 值 : const vector* - 屏幕分辨率向量指针
 * 备    注 : 只读返回，避免外部修改屏幕尺寸参数
 *********************************************************************************************************/
const vector *screen_vector_get(void)
{
	return &screen_vector;
}

/*********************************************************************************************************
 * 函 数 名 : fb_devices_init
 * 功能说明 : 初始化帧缓冲设备及相关缓存
 * 形    参 : vector：原始屏幕分辨率向量；rota：屏幕旋转角度（0/90/180）
 * 返 回 值 : bool - 初始化成功返回true，失败返回false
 * 备    注 : 打开fb设备、配置分辨率/色深、映射显存、分配DMA缓存、初始化TDE/互斥锁
 *********************************************************************************************************/
bool fb_devices_init(const vector *vector, char rota)
{
	fb_device_fd = open(FB_DEV, O_RDWR | O_EXCL);
	if (fb_device_fd < 0)
	{
		return false;
	}
	fb_rotation = rota;

	// 获取fb固定/可变参数
	ioctl(fb_device_fd, FBIOGET_FSCREENINFO, &fb_device_finfo);
	ioctl(fb_device_fd, FBIOGET_VSCREENINFO, &fb_device_vinfo);
	// 配置fb分辨率和色深（24位RGB888）
	fb_device_vinfo.xres = vector->width;
	fb_device_vinfo.yres = vector->height;
	fb_device_vinfo.bits_per_pixel = 24;
	fb_device_vinfo.red.offset = 16;
	fb_device_vinfo.red.length = 8;
	fb_device_vinfo.green.offset = 8;
	fb_device_vinfo.green.length = 8;
	fb_device_vinfo.blue.offset = 0;
	fb_device_vinfo.blue.length = 8;
	ioctl(fb_device_fd, FBIOPUT_VSCREENINFO, &fb_device_vinfo);

	// 根据旋转角度调整屏幕分辨率向量
	if ((fb_rotation == 0) || (fb_rotation == 180))
	{
		screen_vector.width = fb_device_vinfo.xres;
		screen_vector.height = fb_device_vinfo.yres;
	}
	else if (fb_rotation == 90)
	{
		screen_vector.width = fb_device_vinfo.yres;
		screen_vector.height = fb_device_vinfo.xres;
	}

	// 计算屏幕显存大小，映射fb虚拟地址
	int bpp = fb_device_vinfo.bits_per_pixel;
	screen_size = screen_vector.width * screen_vector.height * bpp / 8;
	fb_addres_base = (unsigned char *)osal_fb_mmap_viraddr(screen_size, fb_device_fd);
	bzero(fb_addres_base, screen_size);

	// 获取fb物理地址，初始化双缓冲为A缓冲区
	fb_phyaddres_base = fb_device_finfo.smem_start;
	fb_device_vinfo.reserved[0] = 0;
	ioctl(fb_device_fd, FBIOPUT_VSCREENINFO, &fb_device_vinfo);

	// 分配GUI/背景层DMA缓存（ARGB8888/BGR888格式）
	gui_buffer_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, screen_vector.width * screen_vector.height * 4);
	bg_buffer_addres = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, screen_vector.width * screen_vector.height * 3);

	// 打开TDE硬件加速，初始化fb互斥锁
	ak_tde_open();
	ak_thread_mutex_init(&fb_mutex, NULL);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : fb_dst_tde_layer_get
 * 功能说明 : 获取目标TDE层（fb缓冲区）参数
 * 形    参 : dst - 输出TDE层参数结构体指针
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 根据当前激活的fb缓冲区设置物理地址，格式为RGB888，尺寸为fb原始分辨率
 *********************************************************************************************************/
static bool fb_dst_tde_layer_get(struct ak_tde_layer *dst)
{
	dst->format_param = GP_FORMAT_RGB888;
	dst->width = fb_device_vinfo.xres;
	dst->height = fb_device_vinfo.yres;
	dst->pos_left = dst->pos_top = 0;
	dst->pos_width = fb_device_vinfo.xres;
	dst->pos_height = fb_device_vinfo.yres;
	// 根据双缓冲标识选择物理地址（B缓冲区偏移screen_size）
	dst->phyaddr = FB_BUFFER == FB_BUFFER_B ? (fb_phyaddres_base + screen_size) : fb_phyaddres_base;
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : fb_gui_tde_layer_get
 * 功能说明 : 获取GUI层TDE参数
 * 形    参 : dst - 输出TDE层参数结构体指针
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 格式为ARGB8888，尺寸为旋转后屏幕分辨率，物理地址由DMA虚拟地址转换
 *********************************************************************************************************/
static bool fb_gui_tde_layer_get(struct ak_tde_layer *dst)
{
	dst->format_param = GP_FORMAT_ARGB8888;
	dst->width = screen_vector.width;
	dst->height = screen_vector.height;
	dst->pos_left = dst->pos_top = 0;
	dst->pos_width = screen_vector.width;
	dst->pos_height = screen_vector.height;
	ak_mem_dma_vaddr2paddr(gui_buffer_addres, (unsigned long *)&dst->phyaddr);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : fb_bg_tde_layer_get
 * 功能说明 : 获取背景层TDE参数
 * 形    参 : dst - 输出TDE层参数结构体指针
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 格式为BGR888，尺寸为旋转后屏幕分辨率，物理地址由DMA虚拟地址转换
 *********************************************************************************************************/
static bool fb_bg_tde_layer_get(struct ak_tde_layer *dst)
{
	dst->format_param = GP_FORMAT_BGR888;
	dst->width = screen_vector.width;
	dst->height = screen_vector.height;
	dst->pos_left = dst->pos_top = 0;
	dst->pos_width = screen_vector.width;
	dst->pos_height = screen_vector.height;
	ak_mem_dma_vaddr2paddr(bg_buffer_addres, (unsigned long *)&dst->phyaddr);
	return true;
}

#if 0
/*********************************************************************************************************
* 函 数 名 : fb_video_layer_adj
* 功能说明 : [未启用] 视频层缩放调整
* 形    参 : src - 源TDE层参数
* 返 回 值 : bool - 成功返回true
* 备    注 : 预留视频层缩放接口，暂未使用
*********************************************************************************************************/
static bool fb_video_layer_adj(struct ak_tde_layer* src)
{
	struct ak_tde_layer dst;
	fb_dst_tde_layer_get(&dst);
	ak_tde_opt_scale(src, &dst);
	return true;
}
#endif

/*********************************************************************************************************
 * 函 数 名 : fb_gui_layer_adj
 * 功能说明 : GUI层渲染到fb缓冲区（含旋转处理）
 * 形    参 : 无
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 根据旋转角度选择TDE操作（BLIT/旋转90/180度），将GUI层绘制到目标fb缓冲区
 *********************************************************************************************************/
static bool fb_gui_layer_adj(void)
{
	struct ak_tde_cmd opt;
	// 根据旋转角度设置TDE操作类型
	if (fb_rotation == 0)
	{
		opt.opt = GP_OPT_BLIT;
	}
	else if (fb_rotation == 90)
	{
		opt.opt = GP_OPT_ROTATE;
		opt.rotate_param = AK_GP_ROTATE_90;
	}
	else if (fb_rotation == 180)
	{
		opt.opt = GP_OPT_ROTATE;
		opt.rotate_param = AK_GP_ROTATE_180;
	}

	// 设置源（GUI层）和目标（fb缓冲区）参数
	fb_gui_tde_layer_get(&opt.tde_layer_src);
	fb_dst_tde_layer_get(&opt.tde_layer_dst);

	// 执行TDE硬件加速操作
	ak_tde_opt(&opt);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : fb_bg_layer_adj
 * 功能说明 : 背景层渲染到fb缓冲区（含旋转处理）
 * 形    参 : 无
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 逻辑同GUI层，处理背景层的旋转和绘制，格式为BGR888
 *********************************************************************************************************/
static bool fb_bg_layer_adj(void)
{
	struct ak_tde_cmd opt;
	if (fb_rotation == 0)
	{
		opt.opt = GP_OPT_BLIT;
	}
	else if (fb_rotation == 90)
	{
		opt.opt = GP_OPT_ROTATE;
		opt.rotate_param = AK_GP_ROTATE_90;
	}
	else if (fb_rotation == 180)
	{
		opt.opt = GP_OPT_ROTATE;
		opt.rotate_param = AK_GP_ROTATE_180;
	}
	fb_bg_tde_layer_get(&opt.tde_layer_src);
	fb_dst_tde_layer_get(&opt.tde_layer_dst);

	ak_tde_opt(&opt);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : fb_video_data_pos_adj
 * 功能说明 : 视频数据渲染到背景层指定位置（缩放）
 * 形    参 : src - 视频源TDE层参数；pos - 目标位置/尺寸
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 加锁保护，使用TDE缩放操作将视频数据绘制到背景层指定位置
 *********************************************************************************************************/
bool fb_video_data_pos_adj(struct ak_tde_layer *src, const position *pos)
{
	ak_thread_mutex_lock(&fb_mutex);

	struct ak_tde_layer dst_layer;
	fb_bg_tde_layer_get(&dst_layer);
	// 设置目标位置和尺寸
	dst_layer.pos_left = pos->point.x;
	dst_layer.pos_top = pos->point.y;
	dst_layer.pos_width = pos->vector.width;
	dst_layer.pos_height = pos->vector.height;
	// TDE缩放渲染
	ak_tde_opt_scale(src, &dst_layer);

	fb_refresh_disable = false;
	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : fb_gui_addres_get
 * 功能说明 : 获取GUI层指定位置的缓存地址
 * 形    参 : pos - 目标位置；row_byte - 输出每行字节数
 * 返 回 值 : unsigned char* - GUI层指定位置的缓存地址
 * 备    注 : 加锁保护，返回地址为ARGB8888格式，每行字节数=屏幕宽*4
 *********************************************************************************************************/
unsigned char *fb_gui_addres_get(const position *pos, int *row_byte)
{
	ak_thread_mutex_lock(&fb_mutex);
	*row_byte = screen_vector.width * 4;
	unsigned char *addres = gui_buffer_addres + pos->point.y * screen_vector.width * 4 + pos->point.x * 4;
	fb_refresh_disable = false;
	ak_thread_mutex_unlock(&fb_mutex);
	return addres;
}

/*********************************************************************************************************
 * 函 数 名 : draw_rect
 * 功能说明 : 在GUI层绘制带透明度的矩形
 * 形    参 : pos - 矩形位置/尺寸；color - ARGB8888格式颜色（含透明度）
 * 返 回 值 : bool - 透明度为0时返回false，否则返回true
 * 备    注 : 加锁保护，根据目标像素透明度混合颜色，支持半透明叠加
 *********************************************************************************************************/
bool draw_rect(const position *pos, unsigned int color)
{
	// 透明度为0时不绘制
	if ((color & 0xFF000000) == 0x00)
	{
		return false;
	}

	ak_thread_mutex_lock(&fb_mutex);

	// 计算矩形起始地址和每行字节数
	unsigned char *dst_start = gui_buffer_addres + pos->point.y * screen_vector.width * 4 + pos->point.x * 4;
	int dst_row_byte = screen_vector.width * 4;

	// 解析ARGB分量，计算反向透明度
	unsigned char A = (color >> 24) & 0xFF;
	unsigned char R = (color >> 16) & 0xFF;
	unsigned char G = (color >> 8) & 0xFF;
	unsigned char B = (color) & 0xFF;
	unsigned char rev_s_a = 255 - A;

	int i, j;
	unsigned char *dst = NULL;
	for (j = 0; j < pos->vector.height; j++)
	{
		dst = dst_start;
		for (i = 0; i < pos->vector.width; i++)
		{
			// 目标像素透明度低于阈值时直接覆盖，否则混合颜色
			if (dst[3] < ALPHA_LEVEL)
			{
				*(unsigned int *)dst = color;
			}
			else
			{
				dst[3] = (dst[3] * rev_s_a + A * A) >> 8;
				dst[2] = (dst[2] * rev_s_a + R * A) >> 8;
				dst[1] = (dst[1] * rev_s_a + G * A) >> 8;
				dst[0] = (dst[0] * rev_s_a + B * A) >> 8;
			}
			dst += 4;
		}
		dst_start += dst_row_byte;
	}
	fb_refresh_disable = false;
	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : gui_erase
 * 功能说明 : 擦除GUI层指定区域（填充指定颜色）
 * 形    参 : pos - 擦除区域位置/尺寸；color - ARGB8888格式颜色
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 加锁保护，直接填充颜色覆盖目标区域，无透明度混合
 *********************************************************************************************************/
bool gui_erase(const position *pos, unsigned int color)
{
	ak_thread_mutex_lock(&fb_mutex);

	// 计算擦除区域起始地址和每行字节数
	unsigned char *dst_start = gui_buffer_addres + pos->point.y * screen_vector.width * 4 + pos->point.x * 4;
	int dst_row_byte = screen_vector.width * 4;

	// 逐行填充颜色
	int i, j;
	for (i = 0; i < pos->vector.height; i++)
	{
		unsigned int *dst = (unsigned int *)dst_start;
		for (j = 0; j < pos->vector.width; j++)
		{
			dst[j] = color;
		}
		dst_start += dst_row_byte;
	}

	fb_refresh_disable = false;
	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : gui_layer_channge
 * 功能说明 : [预留] GUI层格式转换/填充
 * 形    参 : 无
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 加锁保护，将GUI层转为RGB888格式并填充黑色，暂未实际使用
 *********************************************************************************************************/
bool gui_layer_channge(void)
{
	ak_thread_mutex_lock(&fb_mutex);
	struct ak_tde_layer src;
	fb_gui_tde_layer_get(&src);
	src.format_param = GP_FORMAT_RGB888;
	src.pos_height = src.height = screen_vector.height * 4 / 3;
	ak_tde_opt_fillrect(&src, 0x00);

	fb_refresh_disable = false;
	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : bg_resource_load
 * 功能说明 : 加载背景资源（JPEG）到背景层
 * 形    参 : res - 资源指针；src_pos - 源位置；offset - 偏移量；align - 对齐方式
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 加锁保护，调用JPEG解码接口，将解码后数据存入背景层缓存，标记背景可见
 *********************************************************************************************************/
bool bg_resource_load(const resource *res, const position *src_pos, const point *offset, align align)
{
	ak_thread_mutex_lock(&fb_mutex);

	// 配置JPEG解码参数
	jpeg_decodec_info jpeg_info;
	jpeg_info.resource = res;
	jpeg_info.align = align;
	jpeg_info.dst = &bg_buffer_addres;
	jpeg_info.dst_vector = NULL;
	jpeg_info.offset = offset;
	jpeg_info.src_pos = src_pos;
	jpeg_info.extern_create_buffer = true;
	jpeg_decodec(&jpeg_info);
	is_bg_visiable = true;

	fb_refresh_disable = false;
	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : rgb_src_scale
 * 功能说明 : RGB数据缩放（TDE硬件加速）
 * 形    参 : dst_vector - 目标尺寸；src_vector - 源尺寸；dst_buffer - 目标缓存；src_buffer - 源缓存
 * 返 回 值 : 无
 * 备    注 : 使用TDE缩放操作，将源BGR888数据缩放到目标尺寸
 *********************************************************************************************************/
static void rgb_src_scale(const vector *dst_vector, const vector *src_vector, unsigned char *dst_buffer, unsigned char *src_buffer)
{
	struct ak_tde_layer src, dst;
	// 设置源层参数
	src.format_param = GP_FORMAT_BGR888;
	src.width = src_vector->width;
	src.height = src_vector->height;
	src.pos_left = src.pos_top = 0;
	src.pos_width = src_vector->width;
	src.pos_height = src_vector->height;
	ak_mem_dma_vaddr2paddr(src_buffer, (unsigned long *)&src.phyaddr);

	// 设置目标层参数
	dst.format_param = GP_FORMAT_BGR888;
	dst.width = dst_vector->width;
	dst.height = dst_vector->height;
	dst.pos_left = dst.pos_top = 0;
	dst.pos_width = dst_vector->width;
	dst.pos_height = dst_vector->height;
	ak_mem_dma_vaddr2paddr(dst_buffer, (unsigned long *)&dst.phyaddr);
	// TDE缩放
	ak_tde_opt_scale(&src, &dst);
}

/*********************************************************************************************************
 * 函 数 名 : jpeg_resource_load
 * 功能说明 : 加载JPEG资源并渲染到背景层指定位置
 * 形    参 : res - JPEG资源指针；pos - 目标位置/尺寸
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 加锁保护，解码JPEG→缩放→TDE绘制到背景层，释放临时缓存
 *********************************************************************************************************/
bool jpeg_resource_load(const resource *res, const position *pos)
{
	ak_thread_mutex_lock(&fb_mutex);

	unsigned char *src = NULL;
	vector src_vector = {0, 0};

	// 解码JPEG到临时缓存
	jpeg_decodec_info jpeg_info;
	jpeg_info.align = CENTER_BOTTOM;
	jpeg_info.dst = &src;
	jpeg_info.dst_vector = &src_vector;
	jpeg_info.offset = NULL;
	jpeg_info.resource = res;
	jpeg_info.src_pos = pos;
	jpeg_info.extern_create_buffer = false;
	jpeg_decodec(&jpeg_info);

	// 分配缩放后缓存，执行RGB缩放
	unsigned char *dst = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_APP, src_vector.width * src_vector.height * 3);
	rgb_src_scale(&pos->vector, &src_vector, dst, src);

	// TDE绘制缩放后数据到背景层指定位置
	struct ak_tde_layer src_layer, dst_layer;
	src_layer.format_param = GP_FORMAT_BGR888;
	src_layer.width = pos->vector.width;
	src_layer.height = pos->vector.height;
	src_layer.pos_left = 0;
	src_layer.pos_top = 0;
	src_layer.pos_width = pos->vector.width;
	src_layer.pos_height = pos->vector.height;
	ak_mem_dma_vaddr2paddr(dst, (unsigned long *)&src_layer.phyaddr);

	fb_bg_tde_layer_get(&dst_layer);
	dst_layer.pos_left = pos->point.x;
	dst_layer.pos_top = pos->point.y;
	dst_layer.pos_width = pos->vector.width;
	dst_layer.pos_height = pos->vector.height;
	ak_tde_opt_blit(&src_layer, &dst_layer);

	// 释放临时缓存
	ak_mem_dma_free(dst);
	if (src != NULL)
	{
		ak_mem_dma_free(src);
	}
	is_bg_visiable = true;

	fb_refresh_disable = false;
	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : gui_background_copy_to_cache
 * 功能说明 : 将当前背景层备份到临时缓存
 * 形    参 : 无
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 加锁保护，首次调用分配临时缓存，使用TDE将背景层数据复制到缓存
 *********************************************************************************************************/
bool gui_background_copy_to_cache(void)
{
	ak_thread_mutex_lock(&fb_mutex);
	// 首次调用分配临时缓存（BGR888格式）
	if (bg_temp_buffer == NULL)
	{
		bg_temp_buffer = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_VO, screen_vector.width * screen_vector.height * 3);
	}

	// TDE复制背景层到临时缓存
	struct ak_tde_layer src, dst;
	fb_bg_tde_layer_get(&src);
	dst = src;
	ak_mem_dma_vaddr2paddr(bg_temp_buffer, (unsigned long *)&dst.phyaddr);
	ak_tde_opt_blit(&src, &dst);

	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : gui_background_recovery_from_cache
 * 功能说明 : 从临时缓存恢复背景层
 * 形    参 : 无
 * 返 回 值 : bool - 缓存不存在返回false，否则返回true
 * 备    注 : 加锁保护，使用TDE将临时缓存数据复制回背景层
 *********************************************************************************************************/
bool gui_background_recovery_from_cache(void)
{
	ak_thread_mutex_lock(&fb_mutex);
	if (bg_temp_buffer == NULL)
	{
		ak_thread_mutex_unlock(&fb_mutex);
		return false;
	}

	// TDE复制临时缓存到背景层
	struct ak_tde_layer src, dst;
	fb_bg_tde_layer_get(&dst);
	src = dst;
	ak_mem_dma_vaddr2paddr(bg_temp_buffer, (unsigned long *)&src.phyaddr);
	ak_tde_opt_blit(&src, &dst);

	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : gui_background_clear
 * 功能说明 : 清空背景层（填充黑色）
 * 形    参 : 无
 * 返 回 值 : bool - 成功返回true
 * 备    注 : 加锁保护，使用TDE填充背景层为黑色（0x00）
 *********************************************************************************************************/
bool gui_background_clear(void)
{
	ak_thread_mutex_lock(&fb_mutex);
	struct ak_tde_layer src_layer;
	fb_bg_tde_layer_get(&src_layer);
	ak_tde_opt_fillrect(&src_layer, 0x00);
	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

/*********************************************************************************************************
 * 函 数 名 : gui_background_cache_destroy
 * 功能说明 : 销毁背景临时缓存
 * 形    参 : 无
 * 返 回 值 : bool - 缓存不存在返回false，否则返回true
 * 备    注 : 加锁保护，释放DMA缓存并置空指针
 *********************************************************************************************************/
bool gui_background_cache_destroy(void)
{
	ak_thread_mutex_lock(&fb_mutex);
	if (bg_temp_buffer == NULL)
	{
		ak_thread_mutex_unlock(&fb_mutex);
		return false;
	}
	ak_mem_dma_free(bg_temp_buffer);
	bg_temp_buffer = NULL;
	ak_thread_mutex_unlock(&fb_mutex);
	return true;
}

#if 0
/*********************************************************************************************************
* 函 数 名 : screen_update
* 功能说明 : [未启用] 屏幕更新函数
* 形    参 : 无
* 返 回 值 : 无
* 备    注 : 预留屏幕更新接口，暂未实现
*********************************************************************************************************/
void screen_update(void)
{
	return ;
#if 0	
	ak_thread_mutex_lock(&fb_mutex);
	if(fb_refresh_disable == false)
	{
		FB_BUFFER_GET;
		FB_BUFFER = FB_BUFFER == FB_BUFFER_A?FB_BUFFER_B:FB_BUFFER_A;
		fb_bg_layer_adj();
		fb_gui_layer_adj();
		FB_BUFFER_SET;	
		fb_refresh_disable = true;
	}
	ak_thread_mutex_unlock(&fb_mutex);
#endif
}
#endif

/*********************************************************************************************************
 * 函 数 名 : screen_display
 * 功能说明 : 屏幕显示刷新（双缓冲切换+图层渲染）
 * 形    参 : 无
 * 返 回 值 : 无
 * 备    注 : 加锁保护，切换双缓冲，渲染背景层和GUI层到新缓冲区，禁用刷新直到下次更新
 *********************************************************************************************************/
void screen_display(void)
{
	ak_thread_mutex_lock(&fb_mutex);
	if (fb_refresh_disable == false)
	{
		// 获取当前缓冲区，切换到另一个缓冲区
		FB_BUFFER_GET;
		FB_BUFFER = FB_BUFFER == FB_BUFFER_A ? FB_BUFFER_B : FB_BUFFER_A;
		// 渲染背景层和GUI层到新缓冲区
		fb_bg_layer_adj();
		fb_gui_layer_adj();
		// 应用缓冲区设置
		FB_BUFFER_SET;
		fb_refresh_disable = true;
	}
	ak_thread_mutex_unlock(&fb_mutex);
}

#if 0
/*********************************************************************************************************
* 函 数 名 : screen_update_disable/screen_update_enable
* 功能说明 : [未启用] 禁用/启用屏幕更新
* 形    参 : 无
* 返 回 值 : 无
* 备    注 : 预留屏幕更新控制接口，暂未实现
*********************************************************************************************************/
void screen_update_disable(void)
{
	ak_thread_mutex_lock(&fb_mutex);
	fb_refresh_disable = true;
	ak_thread_mutex_unlock(&fb_mutex);
}

void screen_update_enable(void)
{
	ak_thread_mutex_lock(&fb_mutex);
	fb_refresh_disable = false;
	ak_thread_mutex_unlock(&fb_mutex);
}
#endif