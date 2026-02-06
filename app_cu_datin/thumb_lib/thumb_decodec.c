#include "ui_api.h"
#include "ak_vdec.h"
#include "thumb_display_api.h"
#include "string.h"
#include "ui_api.h"
#include "ak_tde.h"
#include "ak_mem.h"
#include "avilib.h"

bool fb_video_data_pos_adj(struct ak_tde_layer* src,const position* pos);

int thumb_jpeg_decodec_handle_id = -1;
static bool check_thumb_device_state(void)
{
	if(thumb_jpeg_decodec_handle_id != -1)
	{
		return false;
	}
	return true;
}

int thumb_device_handle_get(void)
{
	return thumb_jpeg_decodec_handle_id;
}

static bool thumb_mjpeg_device_open(void)
{
	struct ak_vdec_param param;
    memset(&param, 0, sizeof(struct ak_vdec_param));
    param.vdec_type = MJPEG_ENC_TYPE;
    param.output_type = AK_YUV420SP;
    param.sc_width = 640;
    param.sc_height = 360;;

    if(ak_vdec_open(&param, &thumb_jpeg_decodec_handle_id))
    {
        printf("open ao device fail \n\r");
        return false;
    }
	ak_vdec_clear_buff(thumb_jpeg_decodec_handle_id);
	return true;
}


bool thumb_device_open(void)
{
	if(check_thumb_device_state() == false)
	{
		return false;
	}
	thumb_mjpeg_device_open();
	return true;
}




static int file_size_get(FILE* fp)
{
	fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
	return file_size;
}



void mjpeg_stream_send(unsigned char* pdata,int len)
{
	int dec_len = 0;
	int read_len = len;
	int send_len = 0;
	while(read_len > 0)
	{
		ak_vdec_send_stream(thumb_jpeg_decodec_handle_id, &pdata[send_len], read_len, BLOCK, &dec_len);
		read_len -= dec_len;
		send_len += dec_len;
	}
}


void thumb_frame_display(const position* pos,struct ak_vdec_frame* frame)
{
	struct ak_tde_layer src;
	src.format_param = GP_FORMAT_YUV420SP;
	src.width = frame->frame_obj.data.pitch_width;
	src.height = frame->frame_obj.data.pitch_height;
	src.pos_top = src.pos_left = 0;
	src.pos_width = frame->width;
	src.pos_height = frame->height;
	ak_mem_dma_vaddr2paddr(frame->frame_obj.data.data, (unsigned long *)&src.phyaddr);
	fb_video_data_pos_adj(&src,pos);
}


static bool thumb_jpeg_stream_send(const char* file)
{
	FILE* fp = fopen(file,"rb");
	if(fp == NULL)
	{
		return false;
	}

	int file_size = file_size_get(fp);
	
	unsigned char* pdata = (unsigned char*)ak_mem_alloc(MODULE_ID_APP, file_size);
	if(fread(pdata,1,file_size,fp) < 0)
	{
		fclose(fp);
		ak_mem_free(pdata);
		printf("============%s:%d read fail \n\r",__func__,__LINE__);
		return false;
	}
	fclose(fp);
	
	mjpeg_stream_send(pdata, file_size);
	
	ak_mem_free(pdata);
	return true;
}

bool mjpeg_frame_display(const position* pos)
{
	struct ak_vdec_frame frame = {0};
	if(ak_vdec_get_frame(thumb_jpeg_decodec_handle_id, &frame) == 0)
	{
		thumb_frame_display(pos,&frame);
		ak_vdec_release_frame(thumb_jpeg_decodec_handle_id, &frame);
		return true;
	}
	return false;
}


static bool thumb_jpeg_decodec(const position* pos,const char* file)
{
	if(thumb_jpeg_stream_send(file) == false)
	{
		return false;
	}
	return mjpeg_frame_display(pos);
}


static bool thumb_video_mjpeg_decodec(const position* pos,const char* file)
{	
	avi_t* avi_file = AVI_open_input_file(file, 1);
	if(avi_file == NULL)
	{
		return false;
	}
	
	if(AVI_set_video_position(avi_file, 0) != 0)
	{
		AVI_close(avi_file);
		return false;
	}

	int frame_size = AVI_frame_size(avi_file, 0);
	if(frame_size <= 0)
	{
		AVI_close(avi_file);
		return false;
	}

	char* frame_buffer = (char*)ak_mem_alloc(MODULE_ID_VDEC, frame_size);
	if(frame_buffer == NULL)
	{
		AVI_close(avi_file);
		return false;
	}

	int iskeyframe;
	if(AVI_read_frame(avi_file, frame_buffer, &iskeyframe)<0)
	{
		ak_mem_free(frame_buffer);
		AVI_close(avi_file);
		return false;
	}
	
	mjpeg_stream_send((unsigned char*)frame_buffer, frame_size);
	
	mjpeg_frame_display(pos);

	ak_mem_free(frame_buffer);
	AVI_close(avi_file);
	return true;	
}




bool thumb_data_load(const position* pos,const char* file,bool is_video)
{
	if(is_video == false)
	{
		return thumb_jpeg_decodec(pos,file);
	}
	else
	{
		return thumb_video_mjpeg_decodec(pos,file);
	}
	
	return false;
}



static void thumb_device_finish_close(int handle_id)
{
	ak_vdec_clear_buff(handle_id);
	ak_vdec_end_stream(handle_id);
	while(1)
	{
		int status = 0;
		ak_vdec_get_decode_finish(handle_id,&status);
		if(status) 
		{
			break;
		}
		ak_sleep_ms(1);
	}
	ak_vdec_close(handle_id);
}


bool thumb_device_close(void)
{
	if(thumb_jpeg_decodec_handle_id != -1)
	{
		thumb_device_finish_close(thumb_jpeg_decodec_handle_id);
		thumb_jpeg_decodec_handle_id = -1;
	}
	return true;
}

