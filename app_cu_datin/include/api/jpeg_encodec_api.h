#ifndef JPEG_ENCODEC_API_H
#define JPEG_ENCODEC_API_H
#include <stdbool.h>

bool jpeg_encodec_start(const char* file_path,void(*finish_callback)(void));
bool check_jpeg_encodec_run(void);

#endif
