#include "i2c_devices_api.h"
#include "ak_thread.h"
#include "fcntl.h"
#include "linux/i2c-dev.h"
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static ak_mutex_t i2c_devices_mutex[I2C_DEVICES_TOTAL];
static const char* i2c_devies[I2C_DEVICES_TOTAL] =
{
	"/dev/i2c-0",
	"/dev/i2c-1",
	"/dev/i2c-2",
	"/dev/i2c-3",
	"/dev/i2c-4"
};

bool i2c_init(I2C_DEVICES devices)
{
	ak_thread_mutex_init(&i2c_devices_mutex[devices], NULL);
	return true;
}

bool i2c_write(I2C_DEVICES devices,unsigned short addres,const i2c_data* data,int len)
{
	bool reslut = true;
	int i = 0;
	ak_thread_mutex_lock(&i2c_devices_mutex[devices]);
	int fd = open(i2c_devies[devices],O_RDWR);
	if(fd < 0)
	{
		printf("open i2c(%s) devices fail \n\r",i2c_devies[devices]);
		reslut = false;
		goto finish;
	}

	if(ioctl(fd, I2C_SLAVE_FORCE, addres) < 0) 
	{ 
		printf("oictl:setslave address(%02x) failed\n",addres);
		reslut = false;
		goto finish;
	}

	for(i = 0 ; i < len ; i++)
	{
		unsigned char i2c_data[2];
		i2c_data[0] = data[i].addres;
		i2c_data[1] = data[i].data;
		if(write(fd,i2c_data,2) <= 0)
		{
			printf("write addr:%02x:%02x fail \n\r",i2c_data[0],i2c_data[1]);
			reslut = false;
			goto finish;
		}
	}
finish:
	if(fd >= 0)
	{
		close(fd);
	}	
	ak_thread_mutex_unlock(&i2c_devices_mutex[devices]);
	return reslut;
}


bool i2c_read(I2C_DEVICES devices,unsigned short addres,i2c_data* data)
{
	bool reslut = true;
	ak_thread_mutex_lock(&i2c_devices_mutex[devices]);
	int fd = open(i2c_devies[devices],O_RDWR);
	if(fd < 0)
	{
		printf("open i2c(%s) devices fail \n\r",i2c_devies[devices]);
		reslut = false;
		goto finish;
	}

	if (ioctl(fd, I2C_SLAVE_FORCE, addres) < 0)
	{ 
		printf("oictl:setslave address(%02x) failed\n",addres);
		reslut = false;
		goto finish;
	}

	if(write(fd,&data->addres,1) <= 0)
	{
        printf("write addr:%02x fail \n\r",data->addres);
		reslut = false;
		goto finish;
    }
	
    if(read(fd,&data->data,1) <= 0)
	{
        printf("read addr:%02x fail \n\r",data->addres);
		reslut = false;
		goto finish;
    }

finish:
	if(fd >= 0)
	{
		close(fd);
	}	
	ak_thread_mutex_unlock(&i2c_devices_mutex[devices]);
	return reslut;
}


