#!/bin/bash
upgrade_bin_version=$(date +"%Y%m%d%H%M%S")
export upgrade_bin_version

# 制作根文件系统，拷贝库、脚本、驱动等文件到根文件系统下
make_rootfs()
{
        cd ../rootfs/
        ./rootfs_build.sh
        cd -
}

# #创建一个用于保存升级文件的目录
create_platform()
{
	if [ -d "platform" ]; then
                echo "remove platform dir"
                rm -rf "platform"
                mkdir "platform"
        else
                echo "mkdir "rootfs""
                mkdir "platform"
        fi
}

# 制作config、tuya、data可读写分区的镜像文件
make_jffs2_images()
{
        ./mkfs.jffs2 -d ../rootfs/rootfs/etc/config -s 256 -e 4096 -m none -o platform/config.jffs2
        ./mkfs.jffs2 -d ../rootfs/rootfs/app/tuya -s 256 -e 4096 -m none -o platform/tuya.jffs2
        ./mkfs.jffs2 -d ../rootfs/rootfs/app/data -s 256 -e 4096 -m none -o platform/data.jffs2
}

# 制作usr、app、root只读分区的镜像文件
make_squashfs_images()
{
    mkdir -p tmpfs
    cp -af ../rootfs/rootfs/* tmpfs

    # 检查tmpfs中的库文件
    echo "=== 检查tmpfs中的库文件 ==="
    ls -la tmpfs/usr/lib/libpng16.so.16
    ls -la tmpfs/usr/lib/libakaudiofilter.so

    ./mksquashfs tmpfs/usr platform/usr.sqsh4 -noappend -comp xz
    ./mksquashfs tmpfs/app/app platform/app.sqsh4 -noappend -comp xz

    # 删除usr目录
    rm -rf tmpfs/usr/* tmpfs/etc/config/* tmpfs/app/tuya/* tmpfs/app/data/* tmpfs/app/app/*
    
    ./mksquashfs tmpfs/ platform/root.sqsh4 -noappend -comp xz

    rm -rf tmpfs
}

# 拷贝 uboot、设备树、uImage 到升级目录 platform/ 下
copy_uboot_kernel_images()
{
        cp  ../os/ubd/u-boot.bin                                                platform/
        cp  ../os/kbd/arch/arm/boot/dts/EVB_CBDR_AK3760E_V1.0.1.dtb             platform/
        cp  ../os/kbd/arch/arm/boot/uImage                                      platform/
}

# 拷贝 分区表、logo文件 到升级目录 platform/ 下
copy_env_logo_images()
{
        cd ../tools/envtool/
        rm env.img
        cp env_ak3760e_nor.cfg env_ak3760e_nor_tmp.cfg
        echo "uprade_image_version $upgrade_bin_version" >>env_ak3760e_nor_tmp.cfg
        tr '\000' '\377' </dev/zero | dd of=./env.img bs=1024 count=4
        ./fw_setenv -s env_ak3760e_nor_tmp.cfg
        ./fw_printenv
        cp env.img     ../../upgrade/platform/env_ak3760e_nor.img
        cd -
        cp logo/taba_hall_machine.rgb          platform/
}
# 把 升级脚本 和 进度条显示程序 也放进去打包压缩
images_compress()
{
    ./partition_image.sh
}

make_rootfs

create_platform

make_jffs2_images

make_squashfs_images

copy_uboot_kernel_images

copy_env_logo_images

images_compress


# image_install_dir=`pwd`/image/
# platform_dir=`pwd`/platform/

# get_config_info()
# {
# 	FLASH_TYPE=`awk -F= '/^FLASH_TYPE/{gsub(" ","",$2); printf $2}' config.mk`
# 	SCREEN_TYPE=`awk -F= '/^SCREEN_TYPE/{gsub(" ","",$2); printf $2}' config.mk`
# 	BOARD_TYPE=`awk -F= '/^BOARD_TYPE/{gsub(" ","",$2); printf $2}' config.mk`
# 	CHIP_TYPE=`awk -F= '/^CHIP_TYPE/{gsub(" ","",$2); printf $2}' config.mk`
# 	WIFI_TYPE=`awk -F= '/^WIFI_TYPE/{gsub(" ","",$2); printf $2}' config.mk`

# 	echo "FLASH_TYPE="$FLASH_TYPE", BOARD_TYPE="$BOARD_TYPE", CHIP_TYPE="$CHIP_TYPE
# }

# extract_fs()
# {
#     # 解压缩根文件系统
#     ./extract.sh

# }

# process_wifi_driver()
# {
# 	echo "WIFI_TYPE="$WIFI_TYPE
#     # 根据选择的WIFI类型，清理掉无用的驱动
#     if test $WIFI_TYPE = "NO_WIFI" ; then
# 		true
#     elif test $WIFI_TYPE = "rtl8188ftv" ; then
#         cp -rf ko/external/rtl8188ftv.ko rootfs/usr/modules/
#     elif test $WIFI_TYPE = "rtl8189ftv" ; then
#         cp -rf ko/external/rtl8189ftv.ko rootfs/usr/modules/
#     elif test $WIFI_TYPE = "atbm6031" ; then
# 		cp -rf ko/external/atbm6031.ko rootfs/usr/modules/
# 	elif test $WIFI_TYPE = "atbm6032" ; then
# 		cp -rf ko/external/atbm6032.ko rootfs/usr/modules/
#     elif test $WIFI_TYPE = "ssv6x5x" ; then
#     cp -rf ko/external/ssv6x5x.ko rootfs/usr/modules/
#     fi
# }

# process_screen_driver()
# {
#     if test -z $SCREEN_TYPE ; then
#         return
#     fi

#     if test $SCREEN_TYPE = "NO_SCREEN" ; then
#         true
#     elif test $SCREEN_TYPE = "RGB" ; then
#         cp -rf ko/external/ts_icn85xx.ko rootfs/usr/modules/
#         true
#     elif test $SCREEN_TYPE = "MIPI" ; then
#         cp -rf ko/external/ts_icn85xx_mipi.ko rootfs/usr/modules/
#         true
#     else
#         echo "Screen Type Error, please check!"
#         exit 1
#     fi
# }

# process_sensor_driver()
# {
#     return
#     #cp -rf ko/external/sensor_*.ko rootfs/usr/modules/
#     #cp -rf ko/external/sensor_gc0308.ko rootfs/usr/modules/
#     #cp -rf ko/external/sensor_tp9950.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_ar0230.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_pr2000.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_sc4236.ko rootfs/usr/modules/
# #    if test $SENSOR_TYPE = "NO_SENSOR" ; then
# #       true
# #   elif test $SENSOR_TYPE = "gc4653" ; then
# #       cp scripts/sensor/gc4653.sh rootfs/usr/sbin/sensor.sh
# #   elif test $SENSOR_TYPE = "pr2000" ; then
# #       cp scripts/sensor/pr2000.sh rootfs/usr/sbin/sensor.sh
# #   elif test $SENSOR_TYPE = "ar0230" ; then
# #       cp scripts/sensor/ar0230.sh rootfs/usr/sbin/sensor.sh
# #   elif test $SENSOR_TYPE = "sc4236" ; then
# #       cp scripts/sensor/sc4236.sh rootfs/usr/sbin/sensor.sh
# #   else
# #       echo "Sensor Type Error, please check!"
# #       exit 1
# #   fi
# }

# # 安装驱动到文件系统
# install_driver()
# {
# 	if test -d rootfs/usr/modules ; then
#         # 删除文件统中遗留的文件
#         rm -rf rootfs/usr/modules/*
#         rm -rf rootfs/lib/modules/*
#     else
# 		mkdir rootfs/usr/modules
# 	fi

#     # 拷贝新的驱动到文件系统
#     #cp -rf ko/external/ak_gpio_keys.ko rootfs/usr/modules/
# #    cp -rf ko/external/ak_saradc.ko rootfs/usr/modules/
#     #cp -rf ko/external/ak_adc_key.ko rootfs/usr/modules/
# #    cp -rf ko/external/ak_crypto.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_efuse.ko rootfs/usr/modules/
#     #cp -rf ko/external/ak_eth.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_hcd.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_i2c.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_ion.ko rootfs/usr/modules/
# #    cp -rf ko/external/ak_isp.ko rootfs/usr/modules/
# 	cp -rf ko/external/ak_image_capture.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_leds.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_mci.ko rootfs/usr/modules/
#     #cp -rf ko/external/ak_motor.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_pcm.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_pwm_char.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_ramdisk.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_rtc.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_saradc.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_udc.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_uio.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_fb.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_gui.ko rootfs/usr/modules/
#     cp -rf ko/external/ak_cs42l52.ko rootfs/usr/modules/
# #    cp -rf ko/external/cryptodev.ko rootfs/usr/modules/
#     cp -rf ko/external/exfat.ko rootfs/usr/modules/ 
#     cp -rf ko/external/sensor_tp9950.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_ar0230.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_f22_f23_f28_f35_f37.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_gc2063.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_gc4653.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_pr2000.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_sc2145.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_sc2335.ko rootfs/usr/modules/
# #    cp -rf ko/external/sensor_sc4236.ko rootfs/usr/modules/

#     cp -rf ko/internal/lib rootfs/
#     rm -rf rootfs/lib/modules/4.4.*/kernel/crypto/
#     rm -rf rootfs/lib/modules/4.4.*/kernel/drivers/staging/rtl8188eu
#     rm -rf rootfs/lib/modules/4.4.*/kernel/net/wireless/mac80211.ko

#     process_wifi_driver

#     process_screen_driver

#     process_sensor_driver
# }

# # 安装平台中间件的库文件到文件系统
# install_platform()
# {
#     # 设置库文件的安装目录
# 	rootfs_dir=`pwd`/rootfs/rootfs
#     lib_install_dir=$rootfs_dir/usr/lib
# 	sample_install_dir=$rootfs_dir/usr/bin
#     cp -rf $platform_dir/lib/*.so $lib_install_dir
#     cp -rf $platform_dir/lib/libssl.so.1.0.0 $lib_install_dir
#     cp -rf $platform_dir/lib/libts.so.0 $lib_install_dir
#     cp -rf $platform_dir/lib/libjpeg.so.62 $lib_install_dir
#     cp -rf $platform_dir/lib/libpng16.so.16 $lib_install_dir
#     cp -rf $platform_dir/lib/libfreetype.so.6 $lib_install_dir
# 	#cd platform/sample
# 	#make 
# 	#make install SAMPLE_INSTALL_DIR=$sample_install_dir
# 	#cd -
# }

# build_drv_sample()
# {
#     # 设置库文件的安装目录
# 	cd os/driver/external/sample
# 	make 
# 	make install INSTALL=$sample_install_dir
# 	cd -
# }

# process_flash_scripts()
# {
#     if test $FLASH_TYPE = "NOR" ; then
#         cp ./scripts/flash/rc.local.nor rootfs/etc/init.d/rc.local
#         cp ./scripts/flash/update.nor.sh rootfs/sbin/update.sh
#     elif test $FLASH_TYPE = "NAND" ; then
#         cp ./scripts/flash/rc.local.nand rootfs/etc/init.d/rc.local
#         cp ./scripts/flash/update.nand.sh rootfs/sbin/update.sh
# 		cp ./scripts/flash/nand_update rootfs/bin/nand_update
#     else
#         echo "Flash type error, please check!"
#         exit 1
#     fi
# }

# process_sensor_scripts()
# {
#     return
#     #cp scripts/sensor/*.sh rootfs/usr/sbin/
#     #cp scripts/sensor/gc0308.sh rootfs/usr/sbin/
#     #cp scripts/sensor/tp9950.sh rootfs/usr/sbin/
# #    cp scripts/sensor/pr2000.sh rootfs/usr/sbin/
# #    cp scripts/sensor/ar0230.sh rootfs/usr/sbin/
# #    cp scripts/sensor/sc4236.sh rootfs/usr/sbin/
# #   if test $SENSOR_TYPE = "NO_SENSOR" ; then
# #       true
# #   elif test $SENSOR_TYPE = "gc4653" ; then
# #       cp scripts/sensor/gc4653.sh rootfs/usr/sbin/sensor.sh
# #   elif test $SENSOR_TYPE = "pr2000" ; then
# #       cp scripts/sensor/pr2000.sh rootfs/usr/sbin/sensor.sh
# #   elif test $SENSOR_TYPE = "ar0230" ; then
# #       cp scripts/sensor/ar0230.sh rootfs/usr/sbin/sensor.sh
# #   elif test $SENSOR_TYPE = "sc4236" ; then
# #       cp scripts/sensor/sc4236.sh rootfs/usr/sbin/sensor.sh
# #   else
# #       echo "Sensor Type Error, please check!"
# #       exit 1
# #   fi
# }

# process_screen_scripts()
# {
#     if test -z $SCREEN_TYPE ; then
#         return
#     fi

#     if test $SCREEN_TYPE = "NO_SCREEN" ; then
#         true
#     elif test $SCREEN_TYPE = "RGB" ; then
#         cp scripts/screen/screen.sh rootfs/usr/sbin/
#     elif test $SCREEN_TYPE = "MIPI" ; then
#         cp scripts/screen/screen_mipi.sh rootfs/usr/sbin/
#     else
#         echo "Screen Type Error, please check!"
#         exit 1
#     fi
# }

# process_wifi_scripts()
# {
#     # 根据选择的WIFI类型，清理掉无用的驱动
#     if test $WIFI_TYPE = "NO_WIFI" ; then
#         true
#     elif test $WIFI_TYPE = "rtl8188ftv" ; then
#         cp scripts/wifi/rtl8188.sh rootfs/usr/sbin/wifi_driver.sh
#         cp -rf ./wifi/* rootfs/
#     elif test $WIFI_TYPE = "rtl8189ftv" ; then
#         cp scripts/wifi/rtl8189.sh rootfs/usr/sbin/wifi_driver.sh
#         cp -rf ./wifi/* rootfs/
#     elif test $WIFI_TYPE = "atbm6031" ; then
#         cp scripts/wifi/atbm6031.sh rootfs/usr/sbin/wifi_driver.sh
#         cp -rf ./wifi/* rootfs/
# 	elif test $WIFI_TYPE = "atbm6032" ; then
#         cp scripts/wifi/atbm6032.sh rootfs/usr/sbin/wifi_driver.sh
#         cp -rf ./wifi/* rootfs/
#     elif test $WIFI_TYPE = "ssv6x5x" ; then
#         cp scripts/wifi/ssv6x5x.sh rootfs/usr/sbin/wifi_driver.sh
#         cp -rf ./wifi/* rootfs/
#     else
#         echo "Wifi Type Error, please check!"
#         exit 1
#     fi
# }

# process_fs_scripts()
# {
#     cp -rf scripts/fs/* rootfs/usr/sbin/
# }

# process_other_scripts()
# {
#     cp -rf scripts/others/* rootfs/usr/sbin/
# }

# # 安装各类脚本
# install_scripts()
# {
#     process_flash_scripts

#     process_screen_scripts

#     process_sensor_scripts

#     process_wifi_scripts

#     process_fs_scripts

#     process_other_scripts
# 	UTILS_SUPPORT=1

#     if test $UTILS_SUPPORT -eq 1 ; then
#         cp -rf utils/* rootfs/
#     fi
# }


# # 拷贝升级模块
# copy_upgrade()
# {
#     rm -rf rootfs/usr/sbin/upgrade
#     cp -rf utils/usr/sbin/upgrade rootfs/usr/sbin/upgrade
# }


# # 制作烧录镜像
# make_image()
# {
#     # 根据选择的Flash类型制作烧录镜像
#     if test $FLASH_TYPE = "NOR" ; then
#         ./create_jffs2fs.sh
#         ./create_squashfs.sh

#         # 将制作好的烧录镜像放到burntool下
#         cp -rf usr.sqsh4 $image_install_dir
#         cp -rf usr.jffs2 $image_install_dir
#         cp -rf root.sqsh4 $image_install_dir
#     elif test $FLASH_TYPE = "NAND" ; then
#         ./create_yaffs2fs.sh
#         cp -rf *.yaffs2 $image_install_dir
#     fi

# }

# get_config_info

# cd rootfs

# extract_fs

# cd ..

# install_platform

# #build_drv_sample

# cd rootfs
# install_driver

# install_scripts

# copy_upgrade

# make_image

# cd ..
# ./partition_image.sh
