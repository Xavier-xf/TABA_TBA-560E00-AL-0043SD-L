#!/bin/bash
set -euo pipefail  # 开启严格模式：未定义变量报错、管道失败则整体失败

##################################################
# 脚本具备功能:
# 1> 编译自动拷贝至对应目录(NFS/U盘)
# 2> 根据版本号快速打包备份
# 3> 实现不修改 Makefile文件而改变编译时的附加功能
# 4> 拷贝BIN和ROM文件到SDK目录并运行auto_build.sh
# 优化点：
# - -all-sdk 模式下编译失败则终止后续拷贝/构建操作
# - 错误码规范（错误场景exit 1，正常exit 0）
# - 编译线程数自动适配CPU核心数
# - 增强错误提示和日志可读性
# - 严格模式避免隐藏错误
##################################################

# 编译线程数（可通过环境变量 BUILD_JOBS 覆盖，例如：BUILD_JOBS=8 ./autoBuild.sh -all-sdk）
BUILD_JOBS=${BUILD_JOBS:-$(nproc)}

# <SYSTEM_VERSION 所在文件>
info_file="system/layout/layout_base.h"

# <定义相关路径>
project_path=$(cd "$(dirname "$0")" && pwd -P)
project_name="${project_path##*/}"

# <定义其他路径>
nfs_path="/home/xiaole/nfs"
usb_file_path="/mnt/hgfs/usb"

# <SDK相关路径>
sdk_path="/home/xiaoxiao/workspace/taba_datin/AK37E_SDK_V1.03"
sdk_app_path="${sdk_path}/rootfs/resource/app/app"
sdk_auto_build_script="${sdk_path}/build.sh"

# <编译生成的目标文件>
target="${project_path}/ANYKA37EOS"
bin="${project_path}/ANYKA37E.BIN"

# <UI ROM文件路径>
ui_rom_path="${project_path}/system/ui/rom.bin"

# <打包文件名前缀>
tarPrefix="${project_name}"

# 全局变量初始化
ver_info_line=""
version=""
tarName=""

####################################################
# 通用错误处理函数
####################################################
function error_exit() {
    echo -e "\033[1;31mError: $1\033[0m" >&2  # 错误信息输出到stderr
    exit 1
}

####################################################
# 检查make命令是否存在
####################################################
function check_make() {
    if ! command -v make &> /dev/null; then
        error_exit "make command not found! Please install make first."
    fi
}

####################################################
# 将.BIN 拷贝至 NFS 文件夹  将 OS  拷贝至 USB 文件夹 #
####################################################
function copy() {
    [ ! -e "$target" ] && error_exit "$target not found"
    [ ! -e "$bin" ] && error_exit "$bin not found"

    # sudo cp -v "$bin" "$nfs_path"

    if [ -e "$usb_file_path" ]; then
        cp -v "$target" "${usb_file_path}/" || error_exit "Copy $target to $usb_file_path failed"
    else
        echo -e "\033[37;1mtips: ${usb_file_path} not exists, copy to usb fail..\033[0m"
    fi
}

#######################################################################################
# 根据define.h文件里的 #define SYSTEM_VERSION "v0.1.01_220907_dev" 版本号,进行打包备份  #
#######################################################################################
function backup() {
    # 清理编译产物
    echo -e "\033[33;1mCleaning build artifacts...\033[0m"
    make clean || error_exit "make clean failed"

    # 打包在上层路径
    cd .. || error_exit "Failed to enter parent directory"

    # 打包,生成 v0.1.01_220907_dev.tar.gz
    echo -e "\033[33;1mCreating backup archive: ${tarName}.tar.gz\033[0m"
    tar -zcvf "${tarName}.tar.gz" "$project_name" || error_exit "Backup failed (tar command error)"
    
    echo -e "\033[32;1m============== <BACKUP SUCCESS> ==============\033[0m"
    echo -e "\033[32;1m${tarName}.tar.gz\033[0m"
}

####################################################
# 拷贝BIN和ROM文件到SDK目录并运行auto_build.sh脚本  #
####################################################
function copy_to_sdk_and_build() {
    # 检查必要的文件是否存在
    local missing_files=()
    [ ! -e "$bin" ] && missing_files+=("ANYKA37E.BIN")
    [ ! -e "$ui_rom_path" ] && missing_files+=("system/ui/rom.bin")
    
    if [ ${#missing_files[@]} -gt 0 ]; then
        echo -e "\033[1;31mThe following files are missing:\033[0m"
        for file in "${missing_files[@]}"; do
            echo -e "\033[1;31m  - $file\033[0m"
        done
        error_exit "Missing required files for SDK copy"
    fi

    # 检查SDK应用目录是否存在
    [ ! -d "$sdk_app_path" ] && error_exit "SDK app directory not found: $sdk_app_path"

    # 检查auto_build.sh脚本是否存在
    [ ! -f "$sdk_auto_build_script" ] && error_exit "auto_build.sh script not found: $sdk_auto_build_script"

    # 拷贝BIN文件到SDK应用目录
    echo -e "\033[32;1mCopying files to SDK directory...\033[0m"
    echo -e "\033[36;1m==========================================\033[0m"
    
    # 拷贝 ANYKA37E.BIN
    echo -e "\033[33;1mCopying $bin to $sdk_app_path ...\033[0m"
    cp -v "$bin" "$sdk_app_path/" || error_exit "ANYKA37E.BIN copy failed"
    echo -e "\033[32;1m  ANYKA37E.BIN copy successful!\033[0m"
    
    echo ""
    
    # 拷贝 system/ui/rom.bin
    echo -e "\033[33;1mCopying $ui_rom_path to $sdk_app_path ...\033[0m"
    cp -v "$ui_rom_path" "$sdk_app_path/" || error_exit "rom.bin copy failed"
    echo -e "\033[32;1m  rom.bin copy successful!\033[0m"
    
    echo -e "\033[36;1m==========================================\033[0m"
    echo -e "\033[32;1mAll files copied successfully!\033[0m"
    echo ""

    # 显示拷贝后的文件信息
    echo -e "\033[36;1mFiles in $sdk_app_path:\033[0m"
    ls -lh "$sdk_app_path/ANYKA37E.BIN" "$sdk_app_path/rom.bin" 2>/dev/null || echo -e "\033[37;1m  No target files found (possible copy error)\033[0m"
    echo ""

    # 运行SDK的auto_build.sh脚本
    echo -e "\033[32;1mRunning $sdk_auto_build_script ...\033[0m"
    echo -e "\033[36;1m==========================================\033[0m"
    
    # 进入SDK目录并运行脚本
    cd "$(dirname "$sdk_auto_build_script")" || error_exit "Failed to enter SDK directory: $(dirname "$sdk_auto_build_script")"
    ./$(basename "$sdk_auto_build_script") -r || error_exit "SDK auto_build.sh execution failed"
    
    echo -e "\033[36;1m==========================================\033[0m"
    echo -e "\033[32;1mSDK auto build completed successfully!\033[0m"
}

####################################################
# 获取版本信息（仅备份时调用）
####################################################
function getInfo(){
    [ ! -e "$info_file" ] && error_exit "$info_file not found"
    
    ver_info_line=$(sed -n '/#define VERSION_NUMBER/=' "$info_file")
    [ -z "$ver_info_line" ] && error_exit "VERSION_NUMBER not found in $info_file"
    
    version=$(awk -F '["]' -v line="$ver_info_line" 'NR==line {print $2}' "$info_file")
    [ -z "$version" ] && error_exit "Failed to parse version number from $info_file"
    
    tarName="${tarPrefix}_${version}_$(date +%m%d%H%M)$1"
}

####################################################
# 主函数
####################################################
function main() {
    # 检查make命令（编译相关选项需要）
    if [[ "$option" =~ ^(-all|-lib|-all-sdk)$ ]]; then
        check_make
    fi

    case "$option" in
        "-all")
            echo -e "\033[33;1mStarting full compilation (jobs: $BUILD_JOBS)...\033[0m"
            make all -j"$BUILD_JOBS" || error_exit "make all compilation failed"
            copy
            echo -e "\033[32;1mFull compilation and copy completed at: $(date)\033[0m"
            ;;
        "-cp")
            echo -e "\033[33;1mCopying files to target directories...\033[0m"
            copy
            echo -e "\033[32;1mFile copy completed at: $(date)\033[0m"
            ;;
        "-lib")
            echo -e "\033[33;1mCompiling library (jobs: $BUILD_JOBS)...\033[0m"
            make lib -j"$BUILD_JOBS" || error_exit "make lib compilation failed"
            echo -e "\033[32;1mLibrary compilation completed at: $(date)\033[0m"
            ;;
        "-bk")
            echo -e "\033[33;1mPreparing backup with version info...\033[0m"
            getInfo "$2"  # 仅备份时解析版本信息
            backup
            ;;
        "-sdk")
            echo -e "\033[33;1mCopying files to SDK and running auto_build.sh...\033[0m"
            copy_to_sdk_and_build
            ;;
        "-all-sdk")
            echo -e "\033[33;1mStarting full compilation + SDK copy + auto_build (jobs: $BUILD_JOBS)...\033[0m"
            # 编译核心逻辑：失败则终止后续操作
            make all -j"$BUILD_JOBS"
            if [ $? -ne 0 ]; then
                error_exit "make all compilation failed! Aborting SDK copy and auto_build."
            fi
            # 编译成功后执行拷贝和构建
            copy_to_sdk_and_build
            echo -e "\033[32;1mAll steps completed at: $(date)\033[0m"
            ;;
        *)
            echo -e "\033[36;1m****************** <menu> ******************\033[0m"
            echo -e "\033[36;1m  ./autoBuild.sh -all        make all (jobs: $BUILD_JOBS)\033[0m"
            echo -e "\033[36;1m  ./autoBuild.sh -lib        make lib (jobs: $BUILD_JOBS)\033[0m"
            echo -e "\033[36;1m  ./autoBuild.sh -cp         copy file to dir\033[0m"
            echo -e "\033[36;1m  ./autoBuild.sh -bk         backup files\033[0m"
            echo -e "\033[36;1m  ./autoBuild.sh -sdk        copy BIN and ROM to SDK and run auto_build.sh\033[0m"
            echo -e "\033[36;1m  ./autoBuild.sh -all-sdk    make all + copy to SDK + run auto_build.sh\033[0m"
            echo -e "\033[36;1m********************************************\033[0m"
            ;;
    esac
    exit 0
}

# 解析命令行参数
option="${1:-}"

# 执行主函数
main