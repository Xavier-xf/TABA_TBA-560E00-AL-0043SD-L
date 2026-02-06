#ifndef THUMB_DECODEC_H
#define THUMB_DECODEC_H
#include "ui_api.h"
#include "ak_vdec.h"

bool mjpeg_frame_display(const position* pos);
void mjpeg_stream_send(unsigned char* pdata,int len);

void thumb_frame_display(const position* pos,struct ak_vdec_frame* frame);

int thumb_device_handle_get(void);

#endif
