#include "ak_common_graphics.h"
#include "ui_api.h"
#include <sys/stat.h> 
#include <unistd.h> 
#include "ak_mem.h"
#include "jpeglib.h"
#include "jerror.h"
#include "ui.h"


extern unsigned char*rom_base_addres;
extern const vector* screen_vector_get(void);
bool position_adj(const position* src_pos,int dst_width,int dst_height,align align,position* dst_pos);


bool jpeg_decodec(jpeg_decodec_info* jpeg_info)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
 	int row_stride;    
	bool reslut = true;
	
	//第1步：申请和初始化JPEG解压目标
	cinfo.err = jpeg_std_error(&jerr); 
	
	//给解码器作必要的内存分配和数据结构的初始化
	jpeg_create_decompress(&cinfo);

	unsigned char* jpeg_buffer =  (unsigned char*)rom_base_addres + jpeg_info->resource->id;
    jpeg_stdio_buffer_src(&cinfo, jpeg_buffer, jpeg_info->resource->size);

	  //第3步：读取图片头信息
    (void) jpeg_read_header(&cinfo, true);

	 //第4步：开始解码
    (void) jpeg_start_decompress(&cinfo);

	  //一行的字节数
    row_stride = cinfo.output_width * cinfo.output_components;
    //int byte  = png_get_bit_depth(png_ptr, info_ptr);

   	int image_width = cinfo.image_width;
    int image_height= cinfo.image_height;
	if(jpeg_info->dst_vector != NULL)
	{
		jpeg_info->dst_vector->width = image_width;
		jpeg_info->dst_vector->height = image_height;
	}

	
	position display_pos = {{0,0},{0,0}};
	if(position_adj(jpeg_info->src_pos,image_width,image_height,jpeg_info->align,&display_pos) == false)
	{
		reslut = false;
		goto JPEG_DECODEC_FINISH;
	}
	if(jpeg_info->offset!= NULL)
	{
    	display_pos.point.x += jpeg_info->offset->x;
		display_pos.point.y += jpeg_info->offset->y;
	}

	row_stride = cinfo.output_width * cinfo.output_components;
	if(jpeg_info->extern_create_buffer == false)
	{
		*(jpeg_info->dst) = (unsigned char *)ak_mem_dma_alloc(MODULE_ID_APP, image_height*row_stride);  
	}
	unsigned char* src = *(jpeg_info->dst);
    if (src == NULL)
	{
		reslut = false;
		goto JPEG_DECODEC_FINISH;
    }

	
	while (cinfo.output_scanline < cinfo.output_height)
	{	
        jpeg_read_scanlines(&cinfo, (JSAMPARRAY)&src, 1);
		src += row_stride;
    }

	//position dst_pos;
	//dst_pos.point.x = dst_pos.point.y = 0;
	//dst_pos.vector = *(screen_vector_get());
	//data_biblt_display(&display_pos.vector,&display_pos.vector,&dst_pos,layer,buffer,GP_FORMAT_BGR888);
	
	//ak_mem_dma_free(buffer);
JPEG_DECODEC_FINISH:

	(void) jpeg_finish_decompress(&cinfo);

    //第7步：释放解码目标
    jpeg_destroy_decompress(&cinfo);

	return reslut;
}



