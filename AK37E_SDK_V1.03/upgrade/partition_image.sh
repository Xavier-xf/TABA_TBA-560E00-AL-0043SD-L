#!/bin/bash
# pwd
# echo "partition_image"
TARGET_DIR="$PWD"
BOOT_TOOLS=$TARGET_DIR/../tools/burntool/boot_tool.bin
UPGRADE_IMAGE_DIR=$TARGET_DIR/platform/
upgrade_bin_name=$TARGET_DIR/HALL_MACHINEOS
# upgrade_bin_version=$(date +"%Y%m%d%H%M%S")
UBOOT_PARTTION="u-boot.bin"
ENV_PARTTION="env_ak3760e_nor.img"
ENVBK_PARTTION="env_ak3760e_nor.img"
DTB_PARTTION="EVB_CBDR_AK3760E_V1.0.1.dtb"
KERNEL_PARTTION="uImage"
LOGO_PARTTION="taba_hall_machine.rgb"
ROOTFS_PARTTION="root.sqsh4"
USR_PARTTION="usr.sqsh4"
CONFIG_PARTTION="config.jffs2"
APP_PARTTION="app.sqsh4"
# RESOURCE_PARTTION="resource.sqsh4"
DATA_PARTTION="data.jffs2"
TUYA_PARTTION="tuya.jffs2"
# ASTERISK_PARTTION="asterisk.sqsh4"

first_arg=$1
# upgrade_bin_version
rm -rf $upgrade_bin_name
touch $upgrade_bin_name

if [ "$first_arg" = "all" ]; then
    uboot_upgrade=y
    env_img_uprade=y
    dtb_upgrade=y
    kernel_upgrade=y
    logo_upgrade=y
    rootfs_upgrade=y
    usr_upgrade=y
    config_upgrade=y
    app_upgrade=y
    # resouce_upgrade=y
    data_upgrade=y
    tuya_upgrade=y
    # asterisk_upgrade=y
elif [ "$first_arg" = "app_resource" ]; then
    app_upgrade=y
    # resouce_upgrade=y
else
    echo -n "upgrade $UBOOT_PARTTION? [y/n]"
    read -n 2 uboot_upgrade

    echo -n "upgrade $ENV_PARTTION? [y/n]"
    read -n 2 env_img_uprade

    echo -n "upgrade $DTB_PARTTION? [y/n]"
    read -n 2 dtb_upgrade

    echo -n "upgrade  $KERNEL_PARTTION? [y/n]"
    read -n 2 kernel_upgrade

    echo -n "upgrade $LOGO_PARTTION? [y/n]"
    read -n 2 logo_upgrade

    echo -n "upgrade $ROOTFS_PARTTION? [y/n]"
    read -n 2 rootfs_upgrade

    echo -n "upgrade $USR_PARTTION?[y/n]"
    read -n 2 usr_upgrade

    echo -n "upgrade  $CONFIG_PARTTION? [y/n]"
    read -n 2 config_upgrade

    echo -n "upgrade $APP_PARTTION? [y/n]"
    read -n 2 app_upgrade

    # echo -n "upgrade $RESOURCE_PARTTION? [y/n]"
    # read -n 2 resouce_upgrade

    echo -n "upgrade $DATA_PARTTION? [y/n]"
    read -n 2 data_upgrade

    echo -n "upgrade $TUYA_PARTTION? [y/n]"
    read -n 2 tuya_upgrade

    # echo -n "upgrade $ASTERISK_PARTTION? [y/n]"
    # read -n 2 asterisk_upgrade
fi

if [ -e $upgrade_bin_name ]; then
    rm -f $upgrade_bin_name
fi

echo "#<upgrade_bin_version=$upgrade_bin_version>" >$upgrade_bin_name
echo "" >>$upgrade_bin_name

parttion_start_postion=0
if [ "$uboot_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$UBOOT_PARTTION")
    echo "# File Parttion: $UBOOT_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$env_img_uprade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$ENV_PARTTION")
    echo "# File Parttion: $ENV_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))

    echo "# File Parttion: $ENVBK_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$dtb_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$DTB_PARTTION")
    echo "# File Parttion: $DTB_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$kernel_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$KERNEL_PARTTION")
    echo "# File Parttion: $KERNEL_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$logo_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$LOGO_PARTTION")
    echo "# File Parttion: $LOGO_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$rootfs_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$ROOTFS_PARTTION")
    echo "# File Parttion: $ROOTFS_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$usr_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$USR_PARTTION")
    echo "# File Parttion: $USR_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$config_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$CONFIG_PARTTION")
    echo "# File Parttion: $CONFIG_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$app_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$APP_PARTTION")
    echo "# File Parttion: $APP_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

# if [ "$resouce_upgrade" = "y" ]; then

#     value=$(wc -c <"$UPGRADE_IMAGE_DIR$RESOURCE_PARTTION")
#     echo "# File Parttion: $RESOURCE_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
#     echo "" >>$upgrade_bin_name
#     parttion_start_postion=$((parttion_start_postion + value))
# fi

if [ "$data_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$DATA_PARTTION")
    echo $value
    echo "# File Parttion: $DATA_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

if [ "$tuya_upgrade" = "y" ]; then

    value=$(wc -c <"$UPGRADE_IMAGE_DIR$TUYA_PARTTION")
    echo "# File Parttion: $TUYA_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
    echo "" >>$upgrade_bin_name
    parttion_start_postion=$((parttion_start_postion + value))
fi

# if [ "$asterisk_upgrade" = "y" ]; then

#     value=$(wc -c <"$UPGRADE_IMAGE_DIR$ASTERISK_PARTTION")
#     echo "# File Parttion: $ASTERISK_PARTTION $parttion_start_postion $value" >>$upgrade_bin_name
#     echo "" >>$upgrade_bin_name
#     parttion_start_postion=$((parttion_start_postion + value))
# fi

echo "# <- this is end of image parttion" >>$upgrade_bin_name
if [ "$uboot_upgrade" = "y" ]; then
    dd if=$BOOT_TOOLS bs=512 count=1 >>$upgrade_bin_name
    dd if=$UPGRADE_IMAGE_DIR$UBOOT_PARTTION bs=512 skip=1 >>$upgrade_bin_name
fi
if [ "$env_img_uprade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$ENV_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
    dd if=$UPGRADE_IMAGE_DIR$ENV_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$dtb_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$DTB_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$kernel_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$KERNEL_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$logo_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$LOGO_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$rootfs_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$ROOTFS_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$usr_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$USR_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$config_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$CONFIG_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$app_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$APP_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
# if [ "$resouce_upgrade" = "y" ]; then
#     dd if=$UPGRADE_IMAGE_DIR$RESOURCE_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
# fi
if [ "$data_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$DATA_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
if [ "$tuya_upgrade" = "y" ]; then
    dd if=$UPGRADE_IMAGE_DIR$TUYA_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
fi
# if [ "$asterisk_upgrade" = "y" ]; then
#     dd if=$UPGRADE_IMAGE_DIR$ASTERISK_PARTTION bs=512 conv=notrunc >>$upgrade_bin_name
# fi

# 获取文件当前大小  
current_size=$(stat -c%s "$upgrade_bin_name")  
  
# 目标大小（16KB）  
target_size=16384  
  
# 计算需要填充的字节数  
additional_size=$((target_size - current_size))  
  
# 判断是否需要填充  
if [ $additional_size -gt 0 ]; then  
    # 如果需要填充，则执行填充操作  
    printf '\xFF%.0s' $(seq 1 $additional_size) >> "$upgrade_bin_name"  
else  
    # 如果不需要填充（即当前大小已经足够或超过目标大小），则可以选择打印一条消息（可选）  
    echo "No padding needed. Current size is already >= target size."  
fi


chmod 777 $upgrade_bin_name
sync

# echo -e "\n=================================================="
# echo "=== AK3760E 升级镜像生成完成！ ==="
# echo "镜像路径：$upgrade_bin_name"
# echo "升级版本：$upgrade_bin_version"
# echo "使用方法："
# echo "1. 将 $upgrade_bin_name 拷贝到SD卡根目录"
# echo "2. 设备启动进入UBoot命令行，执行：sd_upgrade"
# echo "3. 等待升级完成（LCD进度条显示），设备将自动重启"
# echo "=================================================="