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




## 2026-06-03（UNIT 房号设置页输入替换和 dirty 保存）

### 问题描述：

UNIT 房号设置页中，M1-M4 输入框上下或确认切换焦点时，原先会清空当前对话框里的房号；调整为不清空后，又发现已经输入或保存过的房号在第二次移动经过时仍会触发保存，可能因为重复校验导致输入框变黄，用户会误以为操作异常。

### 问题原因：

旧逻辑在 `home_id_set_key_up_up()`、`home_id_set_key_down_up()`、`home_id_set_key_ring_up()` 切换焦点时直接清空当前输入框；改为保留显示后，按键处理仍在切换前无条件调用 `set_home_id_number()`，无法区分“已有显示值”和“本次用户刚输入的值”，因此读出来的旧房号或已经保存过的房号也会被再次保存和校验。

### 解决方法：

焦点切换时不再清空已有房号，只在用户开始输入数字时替换旧值；同时为 M1-M4 增加本次输入标志，只有当前输入框本次按数字输入过，切换焦点时才允许触发保存。未输入过的旧房号只显示和移动焦点，不再重复保存。

### 涉及文件：

- `app_cu_datin/system/layout/layout_home_id_set.c`
- `tests/test_home_id_set_focus_replace.sh`

### 具体修改：

- 新增 `home_id_replace_on_next_input[]`，读房号刷新或切换到已有内容输入框后，标记下一次数字输入需要先清空旧值。
- 新增 `home_id_set_prepare_input_replace()` 和 `home_id_set_mark_replace_on_next_input()`，实现“选择不清空，输入时替换”。
- 新增 `home_id_input_dirty[]`，只有用户本次数字输入成功写入后才置为 dirty。
- 新增 `home_id_set_save_dirty_current()`，上/下/OK 切换焦点时未 dirty 直接移动，dirty 才调用保存逻辑。
- `set_home_id_number()` 从 `void` 改为 `bool`，返回是否真正发起 `Intercom.set_id()`，避免依赖旧的 `Intercom.status`。
- 保存失败或弹出房号已存在确认框时不移动焦点；真实发起保存或保存成功后清除 dirty，避免二次路过重复保存。
- `home_id_set_add_number()` 改为先判断 `index <= max_index` 再写 `show_id[index]`，避免满 4 位后继续按数字造成越界写入。
- 扩展静态测试，校验焦点切换不清空、数字输入前替换、输入后置 dirty、按键保存必须经过 dirty guard。



## 2026-06-02（待机时钟、字体缓冲和删卡越界修正）

### 问题描述：

新增卡管理提示框后，设备在待机或退出待机时可能出现时钟颜色异常、短暂白屏后死机等问题；同时卡管理删除房号时存在把卡号索引当作房号数组下标写入的风险，可能造成内存越界。

### 问题原因：

待机时钟释放逻辑中，`clock_dot_buffer` 分支错误释放了 `analog_clock_dst_buffer`，导致点缓冲泄漏并增加重复释放/错误释放风险；字体渲染临时缓冲未清零，旧像素可能参与红色或白色提示文字绘制；删卡线程参数实际为 `home_id * 10` 的卡片数据起始编号，旧逻辑却直接作为 `UserData.unit_number[]` 下标使用。

### 解决方法：

修正待机时钟各缓冲的释放对象和初始化逻辑；字体缓冲分配后先清零并增加分配失败处理；删卡时先按 `home_id` 在 `UserData.unit_number[]` 有效范围内查找真实位置，找到后前移后续房号并更新计数，不再直接使用卡片起始编号写房号数组。

### 涉及文件：

- `app_cu_datin/ui_lib/analog_clock.c`
- `app_cu_datin/ui_lib/font_decodec.c`
- `app_cu_datin/system/layout/layout_card_manage.c`
- `app_cu_datin/system/layout/layout_card_number.c`

### 具体修改：

- 在 `analog_clock_dst_init()` 中对 `analog_clock_dst_buffer` 分配后的内存清零。
- 在 `analog_clock_deinit()` 中将 `clock_dot_buffer` 与 `analog_clock_dst_buffer` 分别释放并置空。
- 在 `font_decodec()` 中增加字体缓冲分配失败判断，并在分配成功后执行 `memset` 清零。
- 在卡管理页和删卡房号页新增按 `home_id` 删除 `UserData.unit_number[]` 的逻辑，避免 `home_id * 10` 作为数组下标造成越界。
- 删除卡槽前增加 `card_base` 范围检查，避免访问超过 `USER_CARD_TOTAL` 的卡片数据。
- 提示框显示区域同步 `rfid_focus.png` 实际尺寸，使用 `{{152, 73}, {236, 66}}`。



## 2026-06-01（RFID 提示框资源替换）

### 问题描述：

卡管理提示框使用纯色背景时辨识度不足，错误/成功提示不够清晰；用户新增 `rfid_focus.png` 作为 RFID 消息框背景，需要替换旧的纯色消息框，并按结果类型区分文字颜色。

### 问题原因：

旧提示框由程序绘制纯色矩形，容易与背景和底层文字混在一起；提示框尺寸和坐标未跟随新资源统一，且 UNIT、TAG、ERASE、SAVE 右侧输入/输出列位置过靠左，部分内容会压到 `ERASE:` 标签显示区域。

### 解决方法：

卡管理页和删卡房号页统一使用 `rfid_focus.png` 作为提示框背景；错误提示使用红色文字，成功提示使用白色文字；提示框按用户确认的位置显示，并统一右侧输入/输出列的横坐标。

### 涉及文件：

- `app_cu_datin/system/layout/layout_card_manage.c`
- `app_cu_datin/system/layout/layout_card_number.c`
- `app_cu_datin/system/ui/rom.h`
- `app_cu_datin/system/ui/r/img/card_maneage/rfid_focus.png`
- `tests/test_card_prompt_interaction.sh`

### 具体修改：

- 将卡管理页提示框改为绘制 `ROM_R_IMG_CARD_MANEAGE_RFID_FOCUS_PNG`。
- 将删卡房号页提示框同步改为绘制 `ROM_R_IMG_CARD_MANEAGE_RFID_FOCUS_PNG`。
- 错误提示文字颜色设置为 `0xFFFF0000`，成功提示文字颜色设置为 `0xFFFFFFFF`。
- 提示框位置使用 `left=152, top=73`，显示区域使用资源实际尺寸 `236x66`。
- 将 UNIT、TAG、ERASE、SAVE 右侧输入/输出列统一移动到 `x=160`。
- 静态测试禁止继续使用旧的纯色提示框宏，并校验 RFID 资源、提示颜色和位置。



## 2026-05-29（卡管理提示框缩小和残留清理）

### 问题描述：

卡管理提示框尺寸偏大、位置不够合适；保存或删除提示框消失后，`TAG:` 后的卡号以及 `SAVE:` 后的数量仍可能残留在界面上。

### 问题原因：

提示框关闭后只做局部擦除和重绘，未区分当前是否仍处于添卡模式；保存和删除操作已经将刷卡模式切回空闲，但旧的卡号缓存、房间卡数量和成功显示标记仍可能被定时刷新逻辑再次绘制出来。

### 解决方法：

缩小并调整提示框位置；提示框关闭时根据 `SwipingCard.mode` 判断是否仍在添卡模式，如果仍在添卡模式则重画当前卡号和数量，如果已经退出添卡模式则清空 TAG 和 SAVE 后面的结果信息。

### 涉及文件：

- `app_cu_datin/system/layout/layout_card_manage.c`
- `app_cu_datin/system/layout/layout_card_number.c`
- `tests/test_card_prompt_interaction.sh`

### 具体修改：

- 将卡管理页提示框调整为 `left=115, top=59, width=250, height=75`。
- 将删卡房号页提示框同步调整为 `left=115, top=59, width=250, height=75`。
- 新增结果字段清理逻辑，清除 `TAG:` 后的卡号和 `SAVE:` 后的数量显示。
- 保存或删除后清空 `SwipingCard.string_buf[10]`、房间卡数量缓存和 `SwipingCard.success_show`，避免旧值被定时重画。
- 提示框关闭时按 `SwipingCard.mode == CARD_ADD_CARD_MODE` 区分重画旧值还是清掉旧值。
- 当前 GUI 库没有圆角矩形 API，因此未实现真实 `border-radius: 6px`；如后续必须圆角，需要新增底层绘制能力或使用圆角图片资源。



## 2026-05-29（卡管理弹窗和 Logo 修正）

### 问题描述：

英文底部 `TABA Electronics` logo 中 `TABA` 最后一个 `A` 显示异常；卡管理错误/成功提示框与背景颜色接近，覆盖底字后不够像独立弹窗；弹窗关闭后可能留下输入框、卡号或标签残影；UNIT、TAG、ERASE、SAVE 右侧列位置不统一。

### 问题原因：

英文 `TABA` 文本框宽度过窄导致字体被裁剪；旧弹窗背景与页面背景接近，且只擦除局部区域时无法稳定清理底层文字；卡管理右侧输入/输出区域坐标不一致，`ERASE` 行更容易出现对齐和覆盖问题。

### 解决方法：

恢复 `TABA` 足够宽的文本显示区域，同时收紧 `Electronics` 与 `TABA` 的视觉间距；提示框改为更明显的背景色并覆盖底字；关闭弹窗后重绘上半页；统一卡管理右侧输入/输出列坐标。

### 涉及文件：

- `app_cu_datin/system/layout/layout_base.c`
- `app_cu_datin/system/layout/layout_card_manage.c`
- `app_cu_datin/system/layout/layout_card_number.c`
- `tests/test_bottom_logo_language.sh`
- `tests/test_card_prompt_interaction.sh`

### 具体修改：

- 英文 `TABA` 使用足够宽的文本框，避免最后一个 `A` 被裁剪。
- 调整 `Electronics` 坐标，使其与 `TABA` 更靠近但不重叠。
- 卡管理提示框背景改为 `#5D7798`，提高与原页面背景的区分度。
- 弹窗关闭后清理并重绘上半页，减少输入框、卡号和标签残影。
- 卡管理 UNIT、TAG、ERASE、SAVE 右侧输入/输出列统一对齐。
- 保持 `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 不改动；需要只打 APP 分区时继续使用 `partition_image.sh app_resource`。



## 2026-05-26（心跳避让业务通信正式实现）

### 问题描述：

大楼机增加广播心跳后，用户实测在 Output 中连续读取分支器房号时，约第八次左右可能出现“房号不显示”；升级为无心跳版本后，连续几十次未再复现。需要将短期验证过的“业务通信期间暂停心跳”做成正式实现，同时避免分支器在长业务期误判断线闪灯。

### 问题原因：

当前读房号采用链式通信：大楼机发送 `CMD_READ_HOME_ID HOME_ID1`，收到 `CMD_SEND_HOME_ID` 后再继续读取 HOME_ID2/3/4。周期心跳若在业务帧之间插入，会放大链路时序竞争，导致后续房号刷新链条中断；同时分支器在线状态原先只依赖 `CMD_HEARTBEAT`，若业务期暂停心跳，则可能把主站误判为离线。

### 解决方法：

在大楼机侧引入正式的事务型 `bus busy` 机制：业务开始时进入 busy，事务显式结束后继续等待 `500ms` 再恢复心跳，若业务状态异常卡住超过 `4000ms` 才强制释放。分支器侧则把“主站在线刷新”从心跳专用扩展到任意成功解析的有效主站命令。

### 涉及文件：

- `app_cu_datin/system/src/intercom.h`
- `app_cu_datin/system/src/intercom.c`
- `switch/code/msg_event.c`
- `tests/test_intercom_heartbeat_deferral.sh`
- `tests/test_brancher_heartbeat.sh`

### 具体修改：

- 在大楼机协议头中新增 `INTERCOM_BUS_BUSY_TIMEOUT_MS 4000`，与已有 `INTERCOM_HEARTBEAT_DEFER_MS 500` 配合使用。
- 在 `intercom.c` 中增加事务型业务状态：呼叫、设房号、读房号、读监控状态等业务开始时进入 busy，收到完成回包或状态结束后进入 `500ms` 恢复窗口。
- 将 `intercom_event_detect()` 的顺序调整为“先处理接收到的业务帧，再做心跳检查”，减少已到达业务帧前插入心跳的概率。
- 心跳仍然通过底层 `send_can_cmd_encode()` 直接发送，且 defer 期间不再重置心跳时间戳，保证业务结束后约 `500ms` 即可恢复。
- 在分支器 `msg_event.c` 中新增 `intercom_refresh_link_online()`，只要成功解析任意有效主站命令就刷新在线时间；`CMD_HEARTBEAT` 仍保持只接收不回复。
- 收紧静态回归脚本，新增 busy 超时释放、收帧优先于心跳检查、任意有效主站命令刷新在线状态等断言。



## 2026-05-25（主站广播心跳与分支器断线 LED 闪烁）

### 问题描述：

需要在一台大楼机对应多个分支器的拓扑下，实现主站与分支器通信线路断开时，分支器当前常亮的电源 LED 自动改为闪烁；恢复通信后再回到常亮。

### 问题原因：

当前分支器程序只有业务命令的 ACK/重发处理，没有独立的“主站在线/离线”状态检测机制，无法覆盖总线空闲或中途拔线等场景；同时若要求每个分支器都回复心跳，在几十到上百个分支器场景下会造成总线流量随节点数线性增长。

### 解决方法：

采用方案A：由大楼机周期广播心跳命令，所有分支器只接收不回复。分支器本地记录最近一次收到心跳的时间，若超过阈值未收到心跳，则将 `POWER_LED` 从常亮切换为周期闪烁；收到心跳后立即恢复常亮。

### 涉及文件：

- `app_cu_datin/system/src/intercom.h`
- `app_cu_datin/system/src/intercom.c`
- `switch/code/include/kevin_function.h`
- `switch/code/msg_event.c`
- `tests/test_brancher_heartbeat.sh`

### 具体修改：

- 在大厅机和分支器协议头文件中同步新增 `CMD_HEARTBEAT = 0xBA`。
- 在大厅机 `intercom_event_detect()` 中增加周期心跳发送逻辑，默认每 `1000ms` 广播一次心跳。
- 在分支器接收处理 `sys_intercome_check()` 中增加 `CMD_HEARTBEAT` 分支，收到后仅刷新在线状态，不发送 ACK 或其他回复。
- 在分支器中增加基于 `cpu_count` 的离线检测与 LED 状态机：超过 `5000ms` 未收到心跳则进入离线闪烁，`POWER_LED` 每 `500ms` 翻转一次。
- 收到新的心跳后立即将 `POWER_LED` 恢复为常亮状态。
- 新增静态回归脚本 `tests/test_brancher_heartbeat.sh`，用于检查两端命令号一致、分支器不回复心跳，以及离线闪烁逻辑未占用 `TIMER0`。



## 2026-05-22（底部 Logo 语言切换与界面残留修复）

### 问题描述：

底部 `TABA` logo 改为纯图片后，需要由程序根据当前语言动态叠加文字。初始实现中，波斯语 `تابا` 与 `الکترونیک` 间距偏大；在系统设置界面切换语言时，底部 logo 的文字不会立即跟随切换，必须退出页面后才更新；另外在 `calling` 界面中，英语四位房号如 `1234` 无应答后，最右侧会残留一个 `.`。

### 问题原因：

底部 logo 文字坐标对英语和波斯语共用固定布局，导致波斯语排布不够紧凑；系统设置语言切换逻辑只刷新了界面文字和字库，没有重绘底部 logo；`calling` 界面呼叫中和无应答提示使用的文本清除区域不一致，四位房号带省略号时右侧残留未被完全擦除。

### 解决方法：

为底部 logo 保留纯图片背景，并按语言分支分别设置英语和波斯语文字坐标；在系统设置界面切换语言后立即清除底部区域并重绘 logo；在 `calling` 界面增加统一的文本清理区域，呼叫用户、呼叫警卫和无应答三种显示路径都先清理再绘制。

### 涉及文件：

- `app_cu_datin/system/layout/layout_base.c`
- `app_cu_datin/system/layout/layout_system_set.c`
- `app_cu_datin/system/layout/layout_calling.c`
- `app_cu_datin/system/layout/language.h`
- `app_cu_datin/system/layout/language.c`

### 具体修改：

- 在底部 logo 显示函数中增加 `TABA` / `Electronics` 的多语言文字叠加逻辑。
- 调整波斯语底部 logo 的 `تابا` 和 `الکترونیک` 坐标，缩小两者间距。
- 在系统设置语言上下切换分支中，切换语言和重载字库后立即重绘底部 logo。
- 在 `calling` 界面新增统一文本清理区域，修复英语四位房号无应答后右侧 `.` 残留。


## 2026-05-22（波斯语文案调整）

### 问题描述：

波斯语菜单中的部分文案不符合最新要求，包括设置标题、开锁密码标题、恢复出厂文案以及快捷功能栏中的擦除文案。

### 问题原因：

多语言表中仍保留旧版波斯语翻译，未同步为当前确认的目标文案。

### 解决方法：

直接更新波斯语多语言字符串表，将对应条目替换为新的目标翻译。

### 涉及文件：

- `app_cu_datin/system/layout/language.c`

### 具体修改：

- 将 `گذرواژه` / `تنظیم گذرواژه` 对应设置标题修改为 `تنظیمات رمز ورودی`。
- 将 `رمز باز کردن قفل` 对应开锁密码标题修改为 `تنظیمات رمز درب بازکن`。
- 将 `بازنشانی کارخانه` 修改为 `بازگشت به کارخانه`。
- 将快捷功能栏中的 `محو` 修改为 `پاک کردن`。



## 2026-05-21（卡管理房号错误提示修复）

### 问题描述：

在管理房号界面中，当输入非法房号后显示 `Room number error`，继续输入新房号会与错误提示重叠；如果直接退出该界面，再次进入后输入错误房号有时不会再次弹出错误提示。

### 问题原因：

错误状态显示期间，数字输入和删除操作没有被拦截，导致输入内容继续刷新对话框；同时错误提示计数 `card_number_status_count` 在进入/退出界面时未重置，可能错过错误状态的显示触发点。

### 解决方法：

在卡管理界面处于非空闲状态时禁止继续输入或删除房号，并在进入和退出卡管理界面时清零状态计数，保证错误提示每次都能正常显示。

### 涉及文件：

- `app_cu_datin/system/layout/layout_card_manage.c`

### 具体修改：

- 在 `card_manage_input_add_number()` 中增加状态判断，非空闲状态直接返回。
- 在 `card_manage_input_sub_number()` 中增加状态判断，非空闲状态直接返回。
- 在 `layout_card_manage_enter()` 中重置 `card_number_status_count`。
- 在 `layout_card_manage_quit()` 中重置 `card_number_status_count`。



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
