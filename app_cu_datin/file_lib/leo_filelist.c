#include "file_api.h"
#include "ak_mem.h"
#include <dirent.h>
#include "ak_thread.h"
#include "ak_common.h"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stdio.h"





static int sd_mixed_total = 0;
static int sd_mixed_new_total = 0;
static media_info *p_sd_mixed;

static int sd_photo_total = 0;
static int sd_photo_new_total = 0;
static media_info *p_sd_photo;

static int sd_video_total = 0;
static int sd_video_new_total = 0;
static media_info *p_sd_video;

static int sd_message_total = 0;
static int sd_message_new_total = 0;
static media_info *p_sd_message;

static int 	sd_gallery_total = 0;
static int  sd_gallery_new_total = 0;
static media_info *p_sd_gallery;

static int flash_photo_total = 0;
static int flash_photo_new_total = 0;
static media_info *p_flash_photo;

static char sd_card_inserted = 0;
static ak_mutex_t file_list_mutex;

#define VIDEO_DOT ".AVI"
#define PHOTO_DOT ".JPG"
#define IMG_DOT   ".jpg"
#define AUDIO_DOT ".pcm"


static int _video_bad_path_check(const char *file, char *damaged_file)
{
    char bad_file_path[MEDIA_PATH_MAX];
    sprintf(bad_file_path,"%stemp", file);
    if (access(bad_file_path, F_OK) == 0) {
        if (damaged_file != NULL) {
            strcpy(damaged_file, bad_file_path);
        }
        return 1;
    }
    return 0;
}
static int _photo_bad_path_check(const char* file,char* badfile){

    sprintf(badfile,"%s",file);
    if(access(badfile,F_OK) == 0){
        return 0;
    }
    return 1;
}

static bool scan_find_file(media_type type, const char *dir_path, media_info *p_info, int *p_total, int *p_total_new)
{
    char cmd_buffer[64] = {0};
    sprintf(cmd_buffer, "find %s -type f", dir_path);

    FILE *pf = popen(cmd_buffer, "r");
    char buffer[128] = {0};
    while (fgets(buffer, 128, pf)) {
        /**********************
        判断文件是否为规范
        **********************/
        buffer[strlen(buffer) - 1] = '\0';
        char *p_file = strrchr(buffer, '/') + 1;
        if (strlen(p_file) > 24 || strlen(p_file) < 4){
			printf("file name error: %s \n\r",p_file);
            goto fail_next;
        }
        /**************************
        判断是否为视频切换为坏文件
        ***************************/
        if (((type != FILE_TYPE_FLASH_PHOTO) && (type != FILE_TYPE_SD_GALLERY)) && (_video_bad_path_check(buffer, NULL) != 0)) {

			printf("unknown file type \n\r");
            goto fail_next;
        }


        //printf("--- %s %d ---\n\r",p_file,strlen(p_file));
        /******************
        获取该文件所属类型
        *******************/
        char *ptr = strrchr(buffer, '.');
        if ((type == FILE_TYPE_SD_MIXED_PHOTO) &&(strcmp(ptr, PHOTO_DOT) == 0)) {
            p_info->type = FILE_TYPE_SD_MIXED_PHOTO;
        }else if ((type == FILE_TYPE_SD_MIXED_VIDEO) && (strcmp(ptr, VIDEO_DOT) == 0) ) {
            p_info->type = FILE_TYPE_SD_MIXED_VIDEO;
        } else if (((type == FILE_TYPE_FLASH_PHOTO) || (type == FILE_TYPE_SD_PHOTO)) && (strcmp(ptr, PHOTO_DOT) == 0)) {
            p_info->type = FILE_TYPE_SD_PHOTO;
        }else if ((type == FILE_TYPE_SD_VIDEO) && (strcmp(ptr, VIDEO_DOT) == 0)) {
            p_info->type = FILE_TYPE_SD_VIDEO;
        } else if ((type == FILE_TYPE_SD_AUDIO) && (strcmp(ptr, AUDIO_DOT) == 0)) {
            p_info->type = FILE_TYPE_SD_AUDIO;
        }else {
            goto fail_next;
        }

        /***********
        获取该文件名
        ************/
        strncpy(p_info->file_name, p_file, 22);

        p_info->ch = p_file[16] - 48;
        p_info->mode = p_file[17] - 48;

        struct stat st;
        stat(buffer, &st);
        if (st.st_ctime == st.st_mtime) {
            p_info->is_new = 1;
            (*p_total_new)++;

        } else {
            p_info->is_new = 0;
        }
        p_info++;
        (*p_total)++;

        if ((type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && ((*p_total) > SD_MIXED_MAX)){
            break;
        }else if ((type == FILE_TYPE_SD_PHOTO) && ((*p_total) >= SD_PHOTO_MAX)){
            break;
        } if ((type == FILE_TYPE_SD_VIDEO) && ((*p_total) >= SD_VIDEO_MAX)){
            break;
        } else if ((type == FILE_TYPE_SD_AUDIO) && ((*p_total) > SD_AUDIO_MAX)){
            break;
        } else if ((type == FILE_TYPE_SD_GALLERY) && ((*p_total) > SD_GALLERY_MAX)){
            break;
        }else if ((type == FILE_TYPE_FLASH_PHOTO) && ((*p_total) > FLASH_PHOTO_MAX)){
            break;
        }
fail_next:
        memset(buffer, 0, sizeof(buffer));
    }
    pclose(pf);
    return true;
}

#define SD_MIXED_CACHE_PATH SD_MIXED_PATH".config"
#define SD_PHOTO_CACHE_PATH SD_PHOTO_PATH".config"
#define SD_VIDEO_CACHE_PATH SD_VIDEO_PATH".config"

static void sd_media_file_load(media_type type, media_info *p_info, int *p_total, int *p_total_new, int max)
{
    FILE *fp = NULL;
    if(type == FILE_TYPE_SD_MIXED||type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO){
        fp = fopen(SD_MIXED_CACHE_PATH, "rb");
        if (fp == NULL) {
            system("touch "SD_MIXED_CACHE_PATH);
            printf(SD_MIXED_CACHE_PATH" not exit \n\r");
            return ;
        }
    }else if (type == FILE_TYPE_SD_PHOTO) {
        fp = fopen(SD_PHOTO_CACHE_PATH, "rb");
        if (fp == NULL) {
			system("touch "SD_PHOTO_CACHE_PATH);
            printf(SD_PHOTO_CACHE_PATH" not exit \n\r");
            return ;
        }
    } else if(type == FILE_TYPE_SD_VIDEO){
        fp = fopen(SD_VIDEO_CACHE_PATH, "rb");
        if (fp == NULL) {
			system("touch "SD_VIDEO_CACHE_PATH);
            printf(SD_VIDEO_CACHE_PATH" not exit \n\r");
            return ;
        }
    }
	else
	{
		return ;
	}

    int read_len = 0;
    media_info info;
    int read_size = sizeof(media_info);
    while ((read_len = fread(&info, 1, read_size, fp)) == read_size) {

        *p_info = info;
        (*p_total)++;
        if (info.is_new) {
            (*p_total_new)++;
        }
        if ((*p_total) >= max) {
            break;
        }
        p_info++;
    }
    fclose(fp);
    printf("media total:%d \n\r", (*p_total));
    
}

static void sd_file_sync(media_type type)
{
    FILE *fp;
    if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) {
        fp = fopen(SD_MIXED_CACHE_PATH, "wb");
        if (fp == NULL) {

            printf(SD_MIXED_PATH"not exit \n\r");
            return ;
        }
        fwrite(p_sd_mixed, sd_mixed_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    }else if (type == FILE_TYPE_SD_PHOTO) {
        fp = fopen(SD_PHOTO_CACHE_PATH, "wb");
        if (fp == NULL) {

            printf(SD_PHOTO_CACHE_PATH"not exit \n\r");
            return ;
        }
        fwrite(p_sd_photo, sd_photo_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");
    } else {
        fp = fopen(SD_VIDEO_CACHE_PATH, "wb");
        if (fp == NULL) {

            printf(SD_VIDEO_CACHE_PATH"not exit \n\r");
            return ;
        }
        fwrite(p_sd_video, sd_video_total * sizeof(media_info), 1, fp);
        fclose(fp);
        system("sync");

    }
}

static int scan_media_file(media_type type)
{
    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) && (access(SD_MIXED_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_MIXED_PATH);
        return -1;
    } else if ((type == FILE_TYPE_SD_PHOTO) && (access(SD_PHOTO_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_PHOTO_PATH);
        return -1;
    } else if ((type == FILE_TYPE_SD_VIDEO) && (access(SD_VIDEO_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_VIDEO_PATH);
        return -1;
    }else if ((type == FILE_TYPE_SD_AUDIO) && (access(SD_AUDIO_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_AUDIO_PATH);
        return -1;
    } else if ((type == FILE_TYPE_SD_GALLERY) && (access(SD_GALLERY_PATH, F_OK) != 0)) {

        printf("%s non-existent \n\r", SD_GALLERY_PATH);
        return -1;
      } if ((type == FILE_TYPE_FLASH_PHOTO) && (access(FLASH_PHOTO_PATH, F_OK) != 0)) {

          printf("%s non-existent \n\r", FLASH_PHOTO_PATH);
          system("mkdir "FLASH_PHOTO_PATH);
          return -1;

      }else if (type >= FILE_TYPE_NONE) {

        printf("unknown file type \n\r");
        return -1;
    }


    char *dir_path = NULL;
    media_info *p_array = NULL;
    int *p_total = NULL;
    int *p_new_total = NULL;

	
	printf("scan_media_file type: %d \n\r",type);
	if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) {

	    dir_path = SD_MIXED_PATH;
	    sd_mixed_total = sd_mixed_new_total = 0;
	    p_array = p_sd_mixed;
	    p_total = &sd_mixed_total;
	    p_new_total = &sd_mixed_new_total;
	    sd_media_file_load(type, p_array, p_total, p_new_total, SD_MIXED_MAX);
	}else if (type == FILE_TYPE_SD_PHOTO) {

        dir_path = SD_PHOTO_PATH;
        sd_photo_total = sd_photo_new_total = 0;
        p_array = p_sd_photo;
        p_total = &sd_photo_total;
        p_new_total = &sd_photo_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total,SD_PHOTO_MAX);
    } else if (type == FILE_TYPE_SD_VIDEO) {

	    dir_path = SD_VIDEO_PATH;
        sd_video_total = sd_video_new_total = 0;
        p_array = p_sd_video;
        p_total = &sd_video_total;
        p_new_total = &sd_video_new_total;
        sd_media_file_load(type, p_array, p_total, p_new_total,SD_VIDEO_MAX);
    }else if (type == FILE_TYPE_SD_AUDIO) {

        dir_path = SD_AUDIO_PATH;
        sd_message_total = sd_message_new_total = 0;
        p_array = p_sd_message;
        p_total = &sd_message_total;
        p_new_total = &sd_message_new_total;
        scan_find_file(type, dir_path, p_array, p_total, p_new_total);
    } else if (type == FILE_TYPE_SD_GALLERY) {

        dir_path = SD_GALLERY_PATH;
        sd_gallery_total = sd_gallery_new_total = 0;
        p_array = p_sd_gallery;
        p_total = &sd_gallery_total;
        p_new_total = &sd_gallery_new_total;
        scan_find_file(type, dir_path, p_array, p_total, p_new_total);
     }else if (type == FILE_TYPE_FLASH_PHOTO) {
         dir_path = FLASH_PHOTO_PATH;
         flash_photo_total = flash_photo_new_total = 0;
         p_array = p_flash_photo;
         p_total = &flash_photo_total;
         p_new_total = &flash_photo_new_total;
         scan_find_file(type, dir_path, p_array, p_total, p_new_total);
     }

    return 0;
}

bool format_sd_flag = false;
static void sd_format_process(void)
{
    //system("mkfs.vfat /dev/mmcblk0");

    system("cd /");
    system("rm -rf /mnt/tf/*");

    system("umount "SD_BASE_PATH);

    system("mkdosfs -F 32  -n VDP /dev/mmcblk0");

    if (access(SD_BASE_PATH, F_OK) != 0) {

        system("mkdir "SD_BASE_PATH);
    } else {
        system("mount /dev/mmcblk0 "SD_BASE_PATH);
    }
    if(MIX_PHOTOS_AND_VIDEOS_FILE){
        if (access(SD_MIXED_PATH, F_OK) != 0) {

            system("mkdir "SD_MIXED_PATH);
        }
    }else{
        if (access(SD_PHOTO_PATH, F_OK) != 0) {

            system("mkdir "SD_PHOTO_PATH);
        }
        if (access(SD_VIDEO_PATH, F_OK) != 0) {

            system("mkdir "SD_VIDEO_PATH);
        }
    }

    if ((access(SD_AUDIO_PATH, F_OK) != 0) && AUDIO_MESSAGE_FILE_ENABLE) {
        system("mkdir "SD_AUDIO_PATH);
    }
    if ((access(SD_GALLERY_PATH, F_OK) != 0) && GALLERY_IMG_FILE_ENABLE) {
        system("mkdir "SD_GALLERY_PATH);
    }

    system("sync");
	
	//ak_sleep_ms(50);
	//system("umount "SD_BASE_PATH);
	//ak_sleep_ms(50);
	///system("mount /dev/mmcblk0 "SD_BASE_PATH);
}

bool copy_to_sd_flag = false;
static void copy_to_sd_process(void)
{
    if (access(SD_BACKUP_PATH, F_OK) != 0) {

        char buf[128] = {0};
        sprintf(buf, "mkdir %s", SD_BACKUP_PATH);
        system(buf);
        ak_sleep_ms(20);
        system("sync");
    }
    system("mv "FLASH_PHOTO_PATH"*.JPG "SD_BACKUP_PATH);
    ak_sleep_ms(20);
    system("sync");
    ak_sleep_ms(20);
}

char delete_file_flag = 0;
static void delete_file_process(void)
{
    if (delete_file_flag & DELETE_ALL_SD_PHOTO) { ///photo

        if (access(SD_PHOTO_PATH, F_OK) == 0) {
			
			system("rm "SD_PHOTO_CACHE_PATH);
            system("rm -rf "SD_PHOTO_PATH"*JPG");

            scan_media_file(FILE_TYPE_SD_PHOTO);
            printf("\033[31m delete sd photo \n\r");
        }
    }

    if (delete_file_flag & DELETE_ALL_SD_VIDEO) { ///video

        if (access(SD_VIDEO_PATH, F_OK) == 0) {
			
			system("rm "SD_VIDEO_CACHE_PATH);
            system("rm -rf "SD_VIDEO_PATH"*AVI*");

            scan_media_file(FILE_TYPE_SD_VIDEO);

            printf("\033[31m delete sd video \n\r");
        }

    }
    if(MIX_PHOTOS_AND_VIDEOS_FILE){
        if (delete_file_flag & DELETE_ALL_MIXED) { ///mixed

            if (access(SD_MIXED_PATH, F_OK) == 0) {

                system("rm "SD_MIXED_CACHE_PATH);
                system("rm -rf "SD_MIXED_PATH"*.*");
                scan_media_file(FILE_TYPE_SD_MIXED);
                printf("\033[31m delete sd mixed file \n\r");
            }
        }

        if(delete_file_flag & DELETE_ALL_MIXED_PHOTO){
            for(int i= 0;i < sd_mixed_total;i++){
                media_info * media = media_info_get(FILE_TYPE_SD_MIXED,i);
                if(media->type == FILE_TYPE_SD_MIXED_PHOTO)
                {
                    media_file_delete(FILE_TYPE_SD_MIXED, i);
                    i--;
                }
            }
        }

        if(delete_file_flag & DELETE_ALL_MIXED_VIDEO){
            for(int i= 0;i < sd_mixed_total;i++){
                media_info * media = media_info_get(FILE_TYPE_SD_MIXED,i);
                if(media->type == FILE_TYPE_SD_MIXED_VIDEO)
                {
                    media_file_delete(FILE_TYPE_SD_MIXED, i);
                    i--;
                }
            }
        }
        scan_media_file(FILE_TYPE_SD_MIXED);
    }

    if (delete_file_flag & DELETE_ALL_FLASH_PHOTO) { ///photo

        if (access(FLASH_PHOTO_PATH, F_OK) == 0) {
            system("rm -rf "FLASH_PHOTO_PATH"*JPG");

            scan_media_file(FILE_TYPE_FLASH_PHOTO);

            printf("\033[31m delete flash photo \n\r");
        }
    }

    if (delete_file_flag & DELETE_ALL_MESSAGE) { ///message
        if (access(SD_AUDIO_PATH, F_OK) == 0) {
            system("rm -rf "SD_AUDIO_PATH"*pcm");

            scan_media_file(FILE_TYPE_SD_AUDIO);

            printf("\033[31m delete sd message \n\r");
        }
    }
    if (delete_file_flag & DELETE_ALL_GALLERY) {
        if (access(SD_GALLERY_PATH, F_OK) == 0) {
            system("rm -rf "SD_GALLERY_PATH"*pcm");

            scan_media_file(FILE_TYPE_SD_GALLERY);

            printf("\033[31m delete sd message \n\r");
        }
    }

    if (delete_file_flag) {
        delete_file_flag = 0;
        ak_sleep_ms(20);
        system("sync");
    }
}


static void *file_list_task(void *arg)
{
    char cur_insert = 0;
    while (1) {

        cur_insert = (access("/dev/mmcblk0", F_OK) == 0) ? 1 : 0;
        if (cur_insert != sd_card_inserted) {
            ak_thread_mutex_lock(&file_list_mutex);
            sd_card_inserted = cur_insert;
            if (sd_card_inserted) {

                if (access(SD_BASE_PATH, F_OK) != 0) {

					system("mkdir "SD_BASE_PATH);
                }
				
                system("mount /dev/mmcblk0 "SD_BASE_PATH);
				
                if(MIX_PHOTOS_AND_VIDEOS_FILE){
                    if (access(SD_MIXED_PATH, F_OK) != 0) {

                        system("mkdir "SD_MIXED_PATH);
                    }
                    scan_media_file(FILE_TYPE_SD_MIXED);
                }else{
                    if (access(SD_PHOTO_PATH, F_OK) != 0) {

                        system("mkdir "SD_PHOTO_PATH);
                    }
                    if (access(SD_VIDEO_PATH, F_OK) != 0) {

                        system("mkdir "SD_VIDEO_PATH);
                    }
                    scan_media_file(FILE_TYPE_SD_PHOTO);
                    scan_media_file(FILE_TYPE_SD_VIDEO);
                }

                if ((access(SD_AUDIO_PATH, F_OK) != 0) && AUDIO_MESSAGE_FILE_ENABLE) {

                    system("mkdir "SD_AUDIO_PATH);
                    scan_media_file(FILE_TYPE_SD_AUDIO);
                }
                if ((access(SD_GALLERY_PATH, F_OK) != 0) && GALLERY_IMG_FILE_ENABLE) {

                    system("mkdir "SD_GALLERY_PATH);
                    scan_media_file(FILE_TYPE_SD_GALLERY);
                }
            }
            ak_thread_mutex_unlock(&file_list_mutex);
        }
        if (format_sd_flag) {
            sd_format_process();
            if(MIX_PHOTOS_AND_VIDEOS_FILE){
                scan_media_file(FILE_TYPE_SD_MIXED);
            }else{
                scan_media_file(FILE_TYPE_SD_PHOTO);
                scan_media_file(FILE_TYPE_SD_VIDEO);
            }
            if(AUDIO_MESSAGE_FILE_ENABLE){
                scan_media_file(FILE_TYPE_SD_AUDIO);
            }
            if(GALLERY_IMG_FILE_ENABLE){
                scan_media_file(FILE_TYPE_SD_GALLERY);
            }

            format_sd_flag = false;
        }
        if (copy_to_sd_flag) {
            printf("----------------- 123\n\r");
            copy_to_sd_process();
            scan_media_file(FILE_TYPE_FLASH_PHOTO);
            copy_to_sd_flag = false;
        }
        if (delete_file_flag) {
            delete_file_process();
        }
        ak_sleep_ms(50);
    }
    ak_thread_exit();
    return NULL;
}

void start_format_sd_card(void){
    format_sd_flag = true;
}
bool format_sd_card_status(void){
    return format_sd_flag;
}
void start_copy_flash_photo_to_sd(void){
    copy_to_sd_flag = true;
}
bool copy_flash_photo_to_sd_status(void){
    return copy_to_sd_flag;
}
void start_delete_media(enum delete_flag item){
    delete_file_flag = item;
}
char delete_media_status(void){
    return delete_file_flag;
}



void media_file_list_init(void)
{
    ak_thread_mutex_init(&file_list_mutex, NULL);
    p_flash_photo = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * FLASH_PHOTO_MAX);
    scan_media_file(FILE_TYPE_FLASH_PHOTO);
    if(MIX_PHOTOS_AND_VIDEOS_FILE){
        p_sd_mixed = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * SD_MIXED_MAX);
        scan_media_file(FILE_TYPE_SD_MIXED);
    }else{
        p_sd_video = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * SD_VIDEO_MAX);
        p_sd_photo = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * SD_PHOTO_MAX);
        scan_media_file(FILE_TYPE_SD_PHOTO);
        scan_media_file(FILE_TYPE_SD_VIDEO);
    }
    if(AUDIO_MESSAGE_FILE_ENABLE){
        p_sd_message = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * SD_AUDIO_MAX);
        scan_media_file(FILE_TYPE_SD_AUDIO);
    }
    if(GALLERY_IMG_FILE_ENABLE){
        p_sd_gallery = (media_info *)ak_mem_alloc(MODULE_ID_APP, sizeof(media_info) * SD_GALLERY_MAX);
        scan_media_file(FILE_TYPE_SD_GALLERY);
    }
    ak_pthread_t task_id;
    ak_thread_create(&task_id, file_list_task, NULL, ANYKA_THREAD_NORMAL_STACK_SIZE, -1);
}




bool is_sdcard_insert(void)
{

    ak_thread_mutex_lock(&file_list_mutex);
    char insert = sd_card_inserted;
    ak_thread_mutex_unlock(&file_list_mutex);
    return insert ? true : false;
}



bool create_one_media_file(media_type type, char ch, char mode, char *path)
{
    if ((type != FILE_TYPE_FLASH_PHOTO) && ((is_sdcard_insert() == 0) || format_sd_card_status())) 
	{
        printf("create file fail \n\r");
        return false;
    }
    media_info *p_array = NULL;
    char *file_path = NULL;
    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
                && MIX_PHOTOS_AND_VIDEOS_FILE) {

        file_path = SD_MIXED_PATH;
        p_array = &p_sd_mixed[sd_mixed_total];
        sd_mixed_total++;
        sd_mixed_new_total++;
        if (sd_mixed_total > SD_MIXED_MAX) {
            media_file_delete(type, 0);
        }
    }else if ((type == FILE_TYPE_SD_VIDEO) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        file_path = SD_VIDEO_PATH;
        p_array = &p_sd_video[sd_video_total];
        sd_video_total++;
        sd_video_new_total++;
        if (sd_video_total > SD_VIDEO_MAX) {
            media_file_delete(type, 0);
        }
    } else if ((type == FILE_TYPE_SD_PHOTO)&& (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        file_path = SD_PHOTO_PATH;
        p_array = &p_sd_photo[sd_photo_total];
        sd_photo_total++;
        sd_photo_new_total++;
        if (sd_photo_total > SD_VIDEO_MAX) {
            media_file_delete(type, 0);
        }
    } else if ((type == FILE_TYPE_SD_AUDIO) && AUDIO_MESSAGE_FILE_ENABLE) {

        file_path = SD_AUDIO_PATH;
        p_array = &p_sd_message[sd_message_total];
        sd_message_total++;
        sd_message_new_total++;
        if (sd_message_total > SD_AUDIO_MAX) {
            media_file_delete(type, 0);
        }
    } else if ((type == FILE_TYPE_SD_GALLERY) && GALLERY_IMG_FILE_ENABLE) {

        file_path = SD_GALLERY_PATH;
        p_array = &p_sd_gallery[sd_gallery_total];
        sd_gallery_total++;
        sd_gallery_new_total++;
        if (sd_gallery_total > SD_GALLERY_MAX) {
            media_file_delete(type, 0);
        }
    } else if (type == FILE_TYPE_FLASH_PHOTO){
        if (flash_photo_total >= FLASH_PHOTO_MAX) {
            media_file_delete(type, 0);
        }
        file_path = FLASH_PHOTO_PATH;
        p_array = &p_flash_photo[flash_photo_total];
        flash_photo_total++;
        flash_photo_new_total++;
        printf("--------------->>media total :%d ,cur:%d \n\r", SD_VIDEO_MAX, flash_photo_total);
    }else {
        printf("No this type file.\n\r");
        return false;
    }

    p_array->ch = ch;
    p_array->is_new = 1;
    p_array->mode = mode;
    p_array->type = type;
    struct ak_date date;
    ak_get_localdate(&date);
    do {

        if (type == FILE_TYPE_SD_AUDIO) {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, AUDIO_DOT);
        } else if (type == FILE_TYPE_SD_VIDEO || type == FILE_TYPE_SD_MIXED_VIDEO) {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, VIDEO_DOT);
        } else if (type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_FLASH_PHOTO) {
            snprintf(p_array->file_name, MEDIA_PATH_MAX, "%04d%02d%02d-%02d%02d%02d-%d%d%s", date.year, date.month + 1, date.day + 1,
                     date.hour, date.minute, date.second, ch, mode, PHOTO_DOT);
        }

        strcpy(path, file_path);
        strcat(path, p_array->file_name);

        date.second++;
        date.second %= 60;
        printf("%s exist\n\r",path);
    } while (access(path, F_OK) == 0);

    if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_VIDEO || type == FILE_TYPE_SD_MIXED_PHOTO ||type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO) {
        printf("----------------FILE_TYPE_SD_MIXED:%d\n\r ",type);
        sd_file_sync(type);
    }
    return true;
}


media_info *media_info_get(media_type type, int index)
{
    if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)
                && MIX_PHOTOS_AND_VIDEOS_FILE) {
        if (index >= sd_mixed_total) {
            printf("get info error \n\r");
            return NULL;
        }
        return &p_sd_mixed[index];
    }else if (type == FILE_TYPE_FLASH_PHOTO) {
        if (index >= flash_photo_total) {
            printf("get info error \n\r");
            return NULL;
        }
        return &p_flash_photo[index];
    } else if ((type == FILE_TYPE_SD_PHOTO) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (index >= sd_photo_total) {
            printf("get info error \n\r");
            return NULL;
        }
        return &p_sd_photo[index];
    } else if ((type == FILE_TYPE_SD_VIDEO) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (index >= sd_video_total) {
            printf("get info error \n\r");
            return NULL;
        }
        return &p_sd_video[index];
    } else if ((type == FILE_TYPE_SD_AUDIO)&& AUDIO_MESSAGE_FILE_ENABLE) {

        if (index >= sd_message_total) {
            printf("get info error \n\r");
            return NULL;
        }
        return &p_sd_message[index];
    } else if ((type == FILE_TYPE_SD_GALLERY) && GALLERY_IMG_FILE_ENABLE) {

        if (index >= sd_gallery_total) {
            printf("get info error \n\r");
            return NULL;
        }
        return &p_sd_gallery[index];
    }
    printf("type error ! \n\r");
    return NULL;
}


int media_file_total_get(media_type type, char is_new)
{

    if (type == FILE_TYPE_FLASH_PHOTO) {

        return is_new ? flash_photo_new_total : flash_photo_total;

    } else {

        if (is_sdcard_insert() == 0) {

            return -1;
        }
        if (type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) {

            return is_new ? sd_mixed_new_total : sd_mixed_total;
        }else if (type == FILE_TYPE_SD_PHOTO) {

            return is_new ? sd_photo_new_total : sd_photo_total;
        } else if (type == FILE_TYPE_SD_VIDEO) {

            return is_new ? sd_video_new_total : sd_video_total;
        } else if (type == FILE_TYPE_SD_AUDIO) {

            return is_new ? sd_message_new_total : sd_message_total;
        } else if (type == FILE_TYPE_SD_GALLERY) {

            return is_new ? sd_gallery_new_total : sd_gallery_total;
        }
    }

    printf("type error ! \n\r");
    return 0;
}

int media_file_new_clear(media_type type, int index)
{

    media_info *info  = NULL;
    char *path = NULL;

    if (type == FILE_TYPE_FLASH_PHOTO) {

        if (index >= flash_photo_total) {
            printf("get info error \n\r");
            return -1;
        }
        info =  &p_flash_photo[index];
        path = FLASH_PHOTO_PATH;
        if ((info->is_new) && (flash_photo_new_total > 0)) {

            flash_photo_new_total--;
        }

    } else if ((type == FILE_TYPE_SD_MIXED || type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO)&& MIX_PHOTOS_AND_VIDEOS_FILE) {

        if (index >= sd_mixed_total) {

            printf("get info error \n\r");
            return -1;
        }
        info =  &p_sd_mixed[index];
        path = SD_MIXED_PATH;
        if ((info->is_new) && (sd_mixed_new_total > 0)) {
            sd_mixed_new_total--;
        }
    }else if ((type == FILE_TYPE_SD_PHOTO) &&(!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (index >= sd_photo_total) {

            printf("get info error \n\r");
            return -1;
        }
        info =  &p_sd_photo[index];
        path = SD_PHOTO_PATH;
        if ((info->is_new) && (sd_photo_new_total > 0)) {
            sd_photo_new_total--;
        }
    } else if ((type == FILE_TYPE_SD_VIDEO) &&(!MIX_PHOTOS_AND_VIDEOS_FILE)) {

        if (index >= sd_video_total) {

            printf("get info error \n\r");
            return -1;
        }
        info =  &p_sd_video[index];
        path = SD_VIDEO_PATH;
        if ((info->is_new) && (sd_video_new_total > 0)) {
            sd_video_new_total--;
        }
    } else if ((type == FILE_TYPE_SD_AUDIO) && AUDIO_MESSAGE_FILE_ENABLE) {

        if (index >= sd_message_total) {

            printf("get info error \n\r");
            return -1;
        }
        info =  &p_sd_message[index];
        path = SD_AUDIO_PATH;
        if ((info->is_new) && (sd_message_new_total > 0)) {
            sd_message_new_total--;
        }
    } else if ((type == FILE_TYPE_SD_GALLERY) && AUDIO_MESSAGE_FILE_ENABLE) {

        if (index >= sd_gallery_total) {

            printf("get info error \n\r");
            return -1;
        }
        info =  &p_sd_gallery[index];
        path = SD_GALLERY_PATH;
        if ((info->is_new) && (sd_gallery_new_total > 0)) {
            sd_gallery_new_total--;
        }
    }else{
        printf("get info error type \n\r");
        return -1;
    }

    if (info->is_new == 0) {

        return 0;
    }

    info->is_new = 0;
    char file_path[MEDIA_PATH_MAX] = {0};
    strcat(file_path, path);
    strcat(file_path, info->file_name);

    struct stat st;
    chmod(file_path, S_IRUSR | S_IWUSR);
    stat(file_path, &st);
    if (type == FILE_TYPE_SD_PHOTO || type == FILE_TYPE_SD_VIDEO|| type == FILE_TYPE_SD_MIXED_PHOTO || type == FILE_TYPE_SD_MIXED_VIDEO) {
        sd_file_sync(type);
    }
    return 0;
}



int media_file_delete(media_type type, int index)
{

    media_info *info  = NULL, *p_array = NULL;
    char file_path[FLASH_PHOTO_MAX];
    int *total_file  = NULL, *new_total_file = NULL;;

    if (type == FILE_TYPE_FLASH_PHOTO) {
        if (index >= flash_photo_total)
            return -1;

        p_array = p_flash_photo;
        info = &p_flash_photo[index];
        strcpy(file_path, FLASH_PHOTO_PATH);
        strcat(file_path, info->file_name);

        total_file = &flash_photo_total;
        new_total_file = &flash_photo_new_total;

    } else {

        if (is_sdcard_insert() == 0) {

            printf("no insert sd ,delete fail \n\r");
            return -1;
        }
        if ((type == FILE_TYPE_SD_MIXED
            || type == FILE_TYPE_SD_MIXED_PHOTO
            ||type == FILE_TYPE_SD_MIXED_VIDEO)
            && (index < sd_mixed_total)
            && MIX_PHOTOS_AND_VIDEOS_FILE) {

            p_array = p_sd_mixed;
            info = &p_sd_mixed[index];
            strcpy(file_path, SD_MIXED_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_mixed_total;
            new_total_file = &sd_mixed_new_total;
        }else if ((type == FILE_TYPE_SD_PHOTO && index < sd_photo_total) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {
            p_array = p_sd_photo;
            info = &p_sd_photo[index];
            strcpy(file_path, SD_PHOTO_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_photo_total;
            new_total_file = &sd_photo_new_total;
        } else if ((type == FILE_TYPE_SD_VIDEO && index < sd_video_total) && (!MIX_PHOTOS_AND_VIDEOS_FILE)) {

            p_array = p_sd_video;
            info = &p_sd_video[index];
            strcpy(file_path, SD_VIDEO_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_video_total;
            new_total_file = &sd_video_new_total;
        } else if ((type == FILE_TYPE_SD_AUDIO  && index < sd_message_total) && AUDIO_MESSAGE_FILE_ENABLE) {

            p_array = p_sd_message;
            info = &p_sd_message[index];
            strcpy(file_path, SD_AUDIO_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_message_total;
            new_total_file = &sd_message_new_total;
        } else if ((type == FILE_TYPE_SD_GALLERY  && index < sd_gallery_total) && GALLERY_IMG_FILE_ENABLE) {

            p_array = p_sd_gallery;
            info = &p_sd_gallery[index];
            strcpy(file_path, SD_GALLERY_PATH);
            strcat(file_path, info->file_name);

            total_file = &sd_gallery_total;
            new_total_file = &sd_gallery_new_total;
        } else {
            printf("delete media file error.\n\r");
            return -1;
        }
    }

    if ((*total_file) <= 0) {

        printf("delete fail file total %d \n\r", *total_file);
        return -1;
    }
    if ((info->is_new) && ((*new_total_file) > 0)) {

        (*new_total_file)--;
    }

    remove(file_path);
    printf("del %d.%s \n\r", index, file_path);

    if (index < ((*total_file) - 1)) {

        memmove(&p_array[index], &p_array[index + 1], (((*total_file) - 1) - index)*sizeof(media_info));
    }
    (*total_file)--;

    if (type == FILE_TYPE_SD_PHOTO
        || type == FILE_TYPE_SD_VIDEO
        || type == FILE_TYPE_SD_MIXED
        || type == FILE_TYPE_SD_MIXED_PHOTO
        || type == FILE_TYPE_SD_MIXED_VIDEO) {
        sd_file_sync(type);
    }
    return 0;
}


int record_null_error_file_remove(const char *file , bool is_null)
{
	if (is_sdcard_insert() == 0) {
        printf("no insert sd\n\r");
        return -1;
    }

	char bad_file[MEDIA_PATH_MAX] = {0};
    if ((_video_bad_path_check(file, bad_file))) {

        remove(bad_file);
        int index = sd_video_total - 1;
        media_file_delete(p_sd_video[index].type, index);
		return 1;

    }else if(is_null){
	 	int index = sd_video_total - 1;
     	media_file_delete(p_sd_video[index].type, index);
		printf("+++++%s \n\r",file);
		return 1;
    }
	return 0;
}

int snap_null_error_file_remove(const char *file,  bool is_null)
{
	char bad_file[MEDIA_PATH_MAX] = {0};
	if (_photo_bad_path_check(file, bad_file)){
		
        if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0) {
            int index = sd_photo_total - 1;
            media_file_delete(p_sd_photo[index].type, index);
		
			remove(bad_file);
			return 1;
        } else {
			
            int index = flash_photo_total - 1;
            media_file_delete(p_flash_photo[index].type, index);
			
			remove(bad_file);
			return 1;
        }
    }
	else if(is_null)
	{
		printf("+++++%s \n\r",file);
		if (is_sdcard_insert() == 0)
		{
			int index = flash_photo_total - 1;
            media_file_delete(p_flash_photo[index].type, index);
			return 1;
		}
		else  if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0)
		{
            int index = sd_photo_total - 1;
            media_file_delete(p_sd_photo[index].type, index);
			return 1;
        }
	}
	return 0;
}


int playback_bad_file_check(const char *file,int index)
{
	char bad_file[MEDIA_PATH_MAX] = {0};
    if ((_video_bad_path_check(file, bad_file))) {

        remove(bad_file);
        media_file_delete(p_sd_video[index].type, index);

		return 1;
    } else if (_photo_bad_path_check(file, bad_file)) {
		
        if (strncmp(file, SD_BASE_PATH, strlen(SD_BASE_PATH)) == 0) {

			remove(bad_file);
            media_file_delete(p_sd_photo[index].type, index);
			return 1;
        } else {

			remove(bad_file);
            media_file_delete(p_flash_photo[index].type, index);
			return 1;
        }
    }
	return 0;
}

int media_bad_path_check(const char* file){
    if (is_sdcard_insert() == 0){
        printf("no insert sd\n\r");
        return -1;
    }

    char bad_file[MEDIA_PATH_MAX] = {0};
    if(MIX_PHOTOS_AND_VIDEOS_FILE){
        if((_video_bad_path_check(file,bad_file))){
            remove(bad_file);
            int index = sd_mixed_total - 1;
            media_file_delete(p_sd_mixed[index].type,index);
        }else if(_photo_bad_path_check(file,bad_file)){
            if(strncmp(file,SD_BASE_PATH,strlen(SD_BASE_PATH)) == 0){
                int index = sd_mixed_total - 1;
                media_file_delete(p_sd_mixed[index].type,index);
            }else{
                int index = flash_photo_total - 1;
                media_file_delete(p_flash_photo[index].type,index);
            }
        }
    }else{

        if((_video_bad_path_check(file,bad_file))){
            remove(bad_file);
            int index = sd_video_total - 1;
            media_file_delete(p_sd_video[index].type,index);

        }else if(_photo_bad_path_check(file,bad_file)){
            if(strncmp(file,SD_BASE_PATH,strlen(SD_BASE_PATH)) == 0){
                int index = sd_photo_total - 1;
                media_file_delete(p_sd_photo[index].type,index);
            }else{
                int index = flash_photo_total - 1;
                media_file_delete(p_flash_photo[index].type,index);
            }
        }
    }

    return 0;
}
