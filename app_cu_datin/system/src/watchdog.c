#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>

#include "ak_log.h"
#include "ak_drv_wdt.h"


#define DEFULAT_CFG_VALUE (-1)
#define DEFAULT_TIMEOUT 3

static int feed_time = DEFAULT_TIMEOUT-1;  /* 单位:(秒) 每过多久,喂一次狗 */
static int timeout = DEFAULT_TIMEOUT;      /* 单位:(秒) 超时时间,多长时间没有喂狗,就会重启 */

/*************************************************************************
 * @brief  看门狗喂狗线程
 * @date   2022-09-08 08:50
 * @author xiaoele
 **************************************************************************/
static void *watchdog_thread(void *arg) 
{
    while(1)
    {
        // printf("feed dog....\n");
        ak_drv_wdt_feed();
        sleep(feed_time);
    }
    pthread_exit(0);
}

/*************************************************************************
 * @brief  初始化看门狗
 * @date   2022-09-08 08:41
 * @author xiaoele
 **************************************************************************/
int watchdog_init(void)
{
    /* 设置看门狗的超时时间 */
    if (ak_drv_wdt_open(timeout))
    {
        return 0;
    }

    pthread_t watchdog_thread_id;
    pthread_create(&watchdog_thread_id, NULL, watchdog_thread, NULL);

    return 0;
}