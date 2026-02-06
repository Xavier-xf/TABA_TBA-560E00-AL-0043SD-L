#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "ak_drv_common.h"

#include "../../include/ak_efuse.h"

#define AK_EFUSE_FILE_PATH "/dev/efuse"

static void display_efuse_global_id(struct ak_global_id *global_id) {
    int tmp = 0;
    unsigned char *id = &(global_id->chip_globle_id[0]);

    printf("Chip globalID: ");
    for (tmp = AK_GLOBE_ID_LEN; tmp > 0; tmp--) {
        printf("%02X ", id[tmp-1]);
    }
    printf("\n");
}

int ak_drv_read_globalID(void)
{
    int fd = -1;
    struct ak_global_id global_id;

    fd = open(AK_EFUSE_FILE_PATH, O_RDONLY);
    if (fd < 0) {
        ak_print_error("open efuse fail.\n");
        return -EACCES;
    }

    if (ioctl(fd, IOC_READ_GLOBAL_ID, &global_id)) {
        ak_print_error_ex("read fail.\n");
        return -EIO;
    }

    display_efuse_global_id(&global_id);

    close(fd);

    return 0;
}
