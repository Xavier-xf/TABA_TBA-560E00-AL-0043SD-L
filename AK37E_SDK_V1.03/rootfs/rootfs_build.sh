#!/bin/bash
# 解压缩根文件系统
tar_rootfs()
{
    if [ -d "rootfs" ]; then
            echo "remove rootfs dir"
            rm -rf "rootfs"
            mkdir "rootfs"
    else
            echo "mkdir "rootfs""
            mkdir "rootfs"
    fi
    
    echo "tar zxvf rootfs.tar.gz -C rootfs/"
    tar zxvf rootfs.tar.gz -C rootfs/
    cp -rf resource/* rootfs
    find rootfs -name .gitkeep | xargs rm -f
}


process_wifi_driver()
{
    cp -rf ./wifi/* rootfs/
    cp -rf ko/external/rtl8188ftv.ko rootfs/usr/modules/
	
}

process_screen_driver()
{
    if test -z $SCREEN_TYPE ; then
        return
    fi

    if test $SCREEN_TYPE = "NO_SCREEN" ; then
        true
    elif test $SCREEN_TYPE = "RGB" ; then
        cp -rf ko/external/ts_icn85xx.ko rootfs/usr/modules/
        true
    elif test $SCREEN_TYPE = "MIPI" ; then
        cp -rf ko/external/ts_icn85xx_mipi.ko rootfs/usr/modules/
        true
    else
        echo "Screen Type Error, please check!"
        exit 1
    fi
}

process_sensor_driver()
{
    #cp -rf ko/external/sensor_*.ko rootfs/usr/modules/
 #   cp -rf ko/external/sensor_gc0308.ko rootfs/usr/modules/
    cp -rf ko/external/sensor_tp9950.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_ar0230.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_pr2000.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_sc4236.ko rootfs/usr/modules/
#    if test $SENSOR_TYPE = "NO_SENSOR" ; then
#       true
#   elif test $SENSOR_TYPE = "gc4653" ; then
#       cp scripts/sensor/gc4653.sh rootfs/usr/sbin/sensor.sh
#   elif test $SENSOR_TYPE = "pr2000" ; then
#       cp scripts/sensor/pr2000.sh rootfs/usr/sbin/sensor.sh
#   elif test $SENSOR_TYPE = "ar0230" ; then
#       cp scripts/sensor/ar0230.sh rootfs/usr/sbin/sensor.sh
#   elif test $SENSOR_TYPE = "sc4236" ; then
#       cp scripts/sensor/sc4236.sh rootfs/usr/sbin/sensor.sh
#   else
#       echo "Sensor Type Error, please check!"
#       exit 1
#   fi
}

# 安装驱动到文件系统
install_driver()
{
	if test -d rootfs/usr/modules ; then
        # 删除文件统中遗留的文件
        rm -rf rootfs/usr/modules/*
        rm -rf rootfs/lib/modules/*
    else
		mkdir rootfs/usr/modules
	fi

    # 拷贝新的驱动到文件系统
#    cp -rf ko/external/ak_gpio_keys.ko rootfs/usr/modules/
#    cp -rf ko/external/ak_saradc.ko rootfs/usr/modules/
    # cp -rf ko/external/ak_adc_key.ko rootfs/usr/modules/
#    cp -rf ko/external/ak_crypto.ko rootfs/usr/modules/
    cp -rf ko/external/ak_efuse.ko rootfs/usr/modules/
#    cp -rf ko/external/ak_eth.ko rootfs/usr/modules/
    cp -rf ko/external/ak_hcd.ko rootfs/usr/modules/
    cp -rf ko/external/ak_i2c.ko rootfs/usr/modules/
    cp -rf ko/external/ak_ion.ko rootfs/usr/modules/
#    cp -rf ko/external/ak_isp.ko rootfs/usr/modules/
    cp -rf ko/external/ak_image_capture.ko rootfs/usr/modules/
    cp -rf ko/external/ak_leds.ko rootfs/usr/modules/
    cp -rf ko/external/ak_mci.ko rootfs/usr/modules/
#    cp -rf ko/external/ak_motor.ko rootfs/usr/modules/
    cp -rf ko/external/ak_pcm.ko rootfs/usr/modules/
    cp -rf ko/external/ak_pwm_char.ko rootfs/usr/modules/
    cp -rf ko/external/ak_ramdisk.ko rootfs/usr/modules/
    cp -rf ko/external/ak_rtc.ko rootfs/usr/modules/
    cp -rf ko/external/ak_saradc.ko rootfs/usr/modules/
    cp -rf ko/external/ak_udc.ko rootfs/usr/modules/
    cp -rf ko/external/ak_uio.ko rootfs/usr/modules/
    cp -rf ko/external/ak_fb.ko rootfs/usr/modules/
    cp -rf ko/external/ak_gui.ko rootfs/usr/modules/
    cp -rf ko/external/ak_cs42l52.ko rootfs/usr/modules/
#    cp -rf ko/external/cryptodev.ko rootfs/usr/modules/
    cp -rf ko/external/exfat.ko rootfs/usr/modules/
    cp -rf ko/external/sensor_tp9950.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_ar0230.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_f22_f23_f28_f35_f37.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_gc2063.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_gc4653.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_pr2000.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_sc2145.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_sc2335.ko rootfs/usr/modules/
#    cp -rf ko/external/sensor_sc4236.ko rootfs/usr/modules/

    cp -rf ko/internal/lib rootfs/
    rm -rf rootfs/lib/modules/4.4.*/kernel/crypto/
    rm -rf rootfs/lib/modules/4.4.*/kernel/drivers/staging/rtl8188eu
    rm -rf rootfs/lib/modules/4.4.*/kernel/net/wireless/mac80211.ko

    process_wifi_driver

    process_screen_driver

    process_sensor_driver
}

# 安装平台中间件的库文件到文件系统
install_platform()
{
    echo "=== 当前目录: $(pwd) ==="
    
    # 设置库文件的安装目录
    rootfs_dir=`pwd`/rootfs
    lib_install_dir=$rootfs_dir/usr/lib
    
    echo "源目录: ../platform/lib/"
    echo "目标目录: $lib_install_dir"
    
    # 检查目标目录结构
    echo "=== 检查目标目录结构 ==="
    ls -la $rootfs_dir/usr/ 2>/dev/null || echo "usr目录不存在，需要创建"
    
    # 创建目标目录（确保存在）
    echo "创建目录: $lib_install_dir"
    mkdir -p "$lib_install_dir"
    
    # 检查创建结果
    echo "目录创建后:"
    ls -la $rootfs_dir/usr/ 2>/dev/null
    
    # 复制库文件（使用详细模式）
    echo "=== 复制库文件 ==="
    # cp -v "../platform/lib/libpng16.so.16" "$lib_install_dir/"
    # cp -v "../platform/lib/libakaudiofilter.so" "$lib_install_dir/"
    # cp -v "../platform/lib/libjpeg.so.62" "$lib_install_dir/"
    # cp -v "../platform/lib/"*.so "$lib_install_dir/" 2>/dev/null
    # cp -v "../platform/lib/"*.so. "$lib_install_dir/" 2>/dev/null
    find "../platform/lib" -name "*.so" -o -name "*.so.*" | while read file; do
    cp -v "$file" "$lib_install_dir/"
done
    
    # 检查目标文件
    echo "=== 目标库文件列表 ==="
    ls -la "$lib_install_dir/"*.so 2>/dev/null || echo "没有找到.so文件"
    
    # 特别检查关键文件
    echo "=== 检查关键文件 ==="
    if [ -f "$lib_install_dir/libpng16.so.16" ]; then
        echo "✅ libpng16.so.16 - 存在"
    else
        echo "❌ libpng16.so.16 - 缺失"
    fi
    
    if [ -f "$lib_install_dir/libakaudiofilter.so" ]; then
        echo "✅ libakaudiofilter.so - 存在"
    else
        echo "❌ libakaudiofilter.so - 缺失"
    fi
    if [ -f "$lib_install_dir/libjpeg.so.62" ]; then
        echo "✅ libjpeg.so.62 - 存在"
    else
        echo "❌ libjpeg.so.62 - 缺失"
    fi
}


process_flash_scripts()
{
	cp ./scripts/flash/rc.local.nor rootfs/etc/init.d/rc.local
    # cp ./scripts/flash/update.nor.sh rootfs/sbin/update.sh
}




# 安装各类脚本
install_scripts()
{
    process_flash_scripts

    UTILS_SUPPORT=1

    if test $UTILS_SUPPORT -eq 1 ; then
        cp -rf utils/* rootfs/
    fi
}



tar_rootfs

install_platform

install_driver

install_scripts