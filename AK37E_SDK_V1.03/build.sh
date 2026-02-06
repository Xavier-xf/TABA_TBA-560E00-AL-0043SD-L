#!/bin/bash

build_uboot()
{
	cd os/uboot/
    ./build_uboot.sh
    cd -
}

build_kernel()
{
    cd os/kernel/
    ./build_kernel.sh
    cd -
}

build_env()
{
    cd tools/envtool/
    ./env_build.sh
    cd -
}

build_rootfs()
{
    cd upgrade
    ./make_image.sh
    cd -
}

copy_platform()
{
    rm -rf tools/burntool/platform
    cp -rf upgrade/platform tools/burntool/
}

# 更新使用说明，适配新的参数格式
usage()
{
	echo "========================================================"
	echo "Usage : "
	echo "  build.sh -a     全部编译打包"
	echo "  build.sh -u     编译uboot"
	echo "  build.sh -k     编译内核"
	echo "  build.sh -e     制作分区表"
	echo "  build.sh -r     构建文件系统并制作升级包"
	echo "  build.sh -cp     拷贝升级文件到burntool"
	echo "========================================================"
}

main() {
    case $option in
        # 全量编译（保留原-a逻辑）
        "-a")
            echo "全部编译全部打包"
            build_uboot
            build_kernel
            build_env
            build_rootfs
            copy_platform
            ;;
        # 编译uboot
        "-u")
            echo "编译uboot"
            build_uboot
            ;;
        # 编译内核
        "-k")
            echo "编译内核"
            build_kernel
            ;;
        # 制作分区表
        "-e")
            echo "制作分区表"
            build_env
            ;;
        # 构建文件系统并制作升级包
        "-r")
            echo "构建文件系统并制作升级包"
            build_rootfs
            ;;
        # 拷贝升级文件到burntool
        "-cp")
            echo "拷贝升级文件到burntool"
            copy_platform
            ;;
        # 无效参数/无参数时显示使用说明
        *)
            echo "无效参数: $option"
            usage
            ;;
    esac
}

# 获取脚本第一个入参
option=$1
# 执行主逻辑
main