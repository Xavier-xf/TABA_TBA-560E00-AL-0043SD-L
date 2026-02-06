#include "ak_common.h"
#include "ak_common_graphics.h"
#include "ak_tde.h"
#include "ak_mem.h"
#include "ui_api.h"
#include <stdbool.h>
#include "ak_vi.h"
#include "ak_thread.h"

extern const vector* sensor_vector_get(void);

#define VENC_FRAME_QUEUE_MAX 3
static int venc_frame_index = 0;
static int venc_frame_count = 0;
static unsigned char* venc_queue[VENC_FRAME_QUEUE_MAX] = {NULL};
static int venc_queue_size = 0;

static ak_mutex_t venc_queue_mutex ;

static bool is_venc_queue_init = false;

bool venc_queue_push(const struct        video_input_frame* frame,const vector* vetctor)
{
//	printf("push : index:%d\n\r",venc_frame_count);
//	unsigned long long start = os_get_ms();
	ak_thread_mutex_lock(&venc_queue_mutex);
	if((venc_frame_count > VENC_FRAME_QUEUE_MAX)||(is_venc_queue_init == false))
	{
		ak_thread_mutex_unlock(&venc_queue_mutex);
		return false;
	}	
	int index = (venc_frame_count + venc_frame_index)%VENC_FRAME_QUEUE_MAX;
	struct ak_tde_layer src,dst;
	src.format_param = GP_FORMAT_YUV420SP;
	src.width = src.pos_width = vetctor->width;
	src.height = src.pos_height = vetctor->height;
	src.pos_left = src.pos_top;
	src.phyaddr = frame->phyaddr;

	dst.format_param = GP_FORMAT_YUV420SP;
	dst.width = dst.pos_width = vetctor->width;
	dst.height = dst.pos_height =  vetctor->height;
	dst.pos_left = dst.pos_top = 0;
	ak_mem_dma_vaddr2paddr(venc_queue[index],(unsigned long*)&dst.phyaddr);
	ak_tde_opt_scale(&src, &dst);
	
	//ak_tde_opt_blit(&src, &dst);
	venc_frame_count+=1;
	ak_thread_mutex_unlock(&venc_queue_mutex);
//	unsigned long long end = os_get_ms();
//	printf("=================>>%llu \n\r",end - start);
	return true;
}

bool venc_queue_pop(unsigned char** pdata)
{
//	printf("pop : index:%d\n\r",venc_frame_count);

	ak_thread_mutex_lock(&venc_queue_mutex);
    int index = venc_frame_index;
	if((venc_frame_count == 0)||(is_venc_queue_init == false))
	{
		*pdata = NULL;
		ak_thread_mutex_unlock(&venc_queue_mutex);
		return false;
	}
//	*pdata = ak_mem_dma_alloc(MODULE_ID_VI, venc_queue_size);
	*pdata = venc_queue[index];

    venc_frame_index = (venc_frame_index+1)%VENC_FRAME_QUEUE_MAX;
	venc_frame_count -=1;
	ak_thread_mutex_unlock(&venc_queue_mutex);
	return true;
}

static void venc_mem_free(void)
{
	int  i = 0;
	for( i = 0 ; i < VENC_FRAME_QUEUE_MAX ; i++)
	{
		if( venc_queue[i] != NULL)
		{
			ak_mem_dma_free(venc_queue[i]);
			venc_queue[i] = NULL;
		}
	}
}

static void venc_mem_alloc(void)
{
	venc_mem_free();
	int i = 0;
	venc_queue_size =640*360*3/2;// ((const vector*)sensor_vector_get())->width*((const vector*)sensor_vector_get())->height*3/2;
	for( i = 0 ; i < VENC_FRAME_QUEUE_MAX ; i++)
	{
		venc_queue[i] = ak_mem_dma_alloc(MODULE_ID_VENC, venc_queue_size);
	}
}

static void venc_queue_mutex_init(void)
{
	static bool is_first = true;
	if(is_first == true)
	{
		is_first = false;
		ak_thread_mutex_init(&venc_queue_mutex, NULL);
	}
}


bool venc_queue_init(void)
{
	venc_queue_mutex_init();
	ak_thread_mutex_lock(&venc_queue_mutex);
	venc_mem_alloc();
	venc_frame_index = venc_frame_count = 0;

	is_venc_queue_init = true;
	ak_thread_mutex_unlock(&venc_queue_mutex);
	return true;
}




bool venc_queue_deinit(void)
{
	ak_thread_mutex_lock(&venc_queue_mutex);
	venc_mem_free();
	venc_frame_index = venc_frame_count = 0;
	is_venc_queue_init = false;
	ak_thread_mutex_unlock(&venc_queue_mutex);
	return true;
}

