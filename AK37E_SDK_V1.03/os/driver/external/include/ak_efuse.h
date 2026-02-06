/*
 * @file  ak_efuse.h
 * @brief efuse operation interface
 * Copyright (C) 2021 Anyka (Guangzhou) Software Technology Co., LTD
 * @author yangruibin
 * @date  2021-02-25
 * @version 1.0
 */

#ifndef __AK_EFUSE_H__
#define __AK_EFUSE_H__

#define AK_GLOBE_ID_LEN 8 //全球唯一ID，共64位

typedef struct ak_global_id {
	unsigned char chip_globle_id[AK_GLOBE_ID_LEN];
} AK_GLOBAL_ID;

/********************** IOCTL *********************************************/
#define AK_EFUSE_IOC_MAGIC			'F'
#define AK_EFUSE_IO(nr)				_IOC(_IOC_NONE, AK_EFUSE_IOC_MAGIC, nr, 0)
#define AK_EFUSE_IOR(nr)			_IOR(AK_EFUSE_IOC_MAGIC, nr, int)
#define AK_EFUSE_IORn(nr, size)		_IOR(AK_EFUSE_IOC_MAGIC, nr, size)
#define AK_EFUSE_IOW(nr)			_IOW(AK_EFUSE_IOC_MAGIC, nr, int)
#define AK_EFUSE_IOWn(nr, size)		_IOW(AK_EFUSE_IOC_MAGIC, nr, size)

#define IOC_NR_READ_GLOBAL_ID		(0xA1)
#define IOC_READ_GLOBAL_ID			AK_EFUSE_IORn(IOC_NR_READ_GLOBAL_ID, struct ak_global_id)

#endif //__AK_EFUSE_H__
