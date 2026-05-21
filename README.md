```markdown
# TABA_TBA-560E00-AL-0043SD-L 大楼机系统

大楼机系统包含两大部分：

- **主控程序**（大厅机 / Hall Machine）
- **分支器程序**（楼层分支器 / Floor Branch）


## 编译环境要求

- **交叉编译工具链**：arm-anykav500-linux-uclibcgnueabi- （已知路径：/opt/arm-anykav500-linux-uclibcgnueabi/bin/）
- **主机系统**：推荐 Ubuntu 18.04 / 20.04
- **必须工具**：make、gcc、bash、dd、tr、mkfs.jffs2、mksquashfs 等

## 内核 / 系统编译与打包流程

项目提供一键式编译脚本 `build.sh`，支持模块化与全量编译。

### 使用方式

```bash
# 显示帮助
./build.sh

# 全量编译 + 打包（最常用）
./build.sh -a

# 单独编译某部分（调试时常用）
./build.sh -u        # 只编译 uboot
./build.sh -k        # 只编译 kernel
./build.sh -e        # 只制作 env 分区表
./build.sh -r        # 构建 rootfs 并制作升级包
./build.sh -cp       # 拷贝生成的升级文件到 burntool 目录
```

### 详细步骤说明（-a 选项执行的完整流程）

1. **编译 U-Boot**  
   → 生成 `os/ubd/u-boot.bin`

2. **编译 Linux Kernel**  
   → 生成 `os/kbd/arch/arm/boot/uImage`  
   → 生成设备树 `EVB_CBDR_AK3760E_V1.0.1.dtb`

3. **制作环境变量分区（env）**  
   → 生成 `tools/envtool/env_ak3760e_nor.img`

4. **构建根文件系统 & 制作升级镜像**  
   - 制作 squashfs 只读分区：root.sqsh4、usr.sqsh4、app.sqsh4  
   - 制作 jffs2 可写分区：config.jffs2、data.jffs2、tuya.jffs2  
   - 整合所有文件 + uboot + kernel + env + logo  
   → 最终生成升级文件：`upgrade/HALL_MACHINEOS`

5. **拷贝到烧录工具目录**  
   → `tools/burntool/platform/`

## 应用层（大厅机程序）编译

应用层代码位于 `app_cu_datin/` 目录，使用独立的编译脚本。

```bash
cd app_cu_datin

# 常用命令
./autobuild.sh -all       # 全量编译
./autobuild.sh -all-sdk   # 编译 + 自动拷贝到 SDK 目录并触发 SDK 打包
./autobuild.sh -lib       # 只编译库文件
./autobuild.sh -cp        # 只拷贝生成的文件
./autobuild.sh -sdk       # 只拷贝到 SDK 并运行 SDK 的打包脚本
./autobuild.sh -bk        # 备份当前版本代码（会自动读取版本号）
```

**最推荐的开发/测试流程**：

```bash
cd app_cu_datin
./autobuild.sh -all-sdk
```

该命令会：

1. 编译应用
2. 拷贝 `ANYKA37E.BIN` 和 `system/ui/rom.bin` 到 SDK 目录
3. 自动调用 SDK 的 `build.sh -r` 重新打包升级镜像

## 分支器（楼层分支器）烧录流程

1. 打开烧录软件  
   路径：`楼层分支器/笙泉单片机烧录软件/Database Installer/IcpProgrammer.exe`

2. 载入用户程序  
   ```
   载入文件 → AP → 分支器D32\KeilPrj\OBJ\kevin.hex
   ```

3. 插入 ISP 引导代码  
   ```
   插入ISP代码 → 用户自定义的ISP代码 → 分支器D32\KeilPrj\OBJ\b2000_bootloader.hex
   ```

4. IAP 存储区域设置为 **2.0K**

5. 烧录器设置  
   - 将烧录器连接电脑 → 点击【设置脱机模式】
   - 烧录器连接目标板 → 按下烧录器上的烧录按钮
   - 提示灯闪烁 → 等待烧录完成（灯停止闪烁）

## 升级方式（大厅机）

1. 将生成的升级文件 `HALL_MACHINEOS` 拷贝到 SD 卡根目录
2. 设备上电进入 **U-Boot 命令行**
3. 执行命令：

   ```
   sd_upgrade
   ```

4. 等待 LCD 显示进度条完成，设备自动重启

---

**注意事项**

- 交叉编译工具链路径必须正确，否则编译会失败
- 升级包文件名必须为 `HALL_MACHINEOS`（无后缀）
- 分支器烧录请务必确认 IAP 区域为 **2.0K**
- 建议每次重要改动后使用 `./autobuild.sh -bk` 备份代码

## 2026-05-21

### 问题描述：

录入绑定大量带标签设备信息后，继续录入新房号会导致设备卡死。实测通过预置 255 个已绑定房号后，继续输入 1257，`/app/data/data.ini` 中 `index3` 增加到 256，随后设备卡死或重启后仍无法正常恢复。

### 问题原因：

部分遍历已绑定房号/单元号的循环使用 `unsigned char` 作为下标，同时循环上限来自 `get_int_conf(...)` 的整型配置值。当绑定数量达到 256 时，`unsigned char` 从 255 自增后回绕为 0，导致循环无法结束。

### 解决方法：

将相关循环下标由 `unsigned char` 改为 `int`，避免数量达到 256 后发生回绕死循环。

### 涉及文件：

- `app_cu_datin/system/src/swiping_card.c`
- `app_cu_datin/system/src/user_data.c`
- `app_cu_datin/system/layout/layout_logo.c`
- `app_cu_datin/system/layout/layout_home_id_set.c`
- `app_cu_datin/system/layout/layout_base.h`

### 具体修改：

- 修复 `unit_number_exist()` 中遍历 `UserData.unit_number` 的下标类型。
- 修复 `deleteAllCard()` 中打印单元号列表的下标类型。
- 修复 `printf_user_data()` 中打印房号和单元号列表的下标类型。
- 修复 `home_id_exist()` 中遍历 `UserData.home_id` 的下标类型。
- 版本号由 `v2.1.0_dev` 更新为 `v2.1.1_dev`，便于升级后确认版本。
