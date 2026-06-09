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


## 2026-06-09（Output 房号设置页错误后按键和 X 删除行为修正）

### 问题描述：

在 Output 中输入分支器号后进入房号设置页，读到房号如 `9,10,11,12`，如果把其中一个房号改成重复房号，例如把 `10` 改成 `9`，系统会正确报错变黄并显示 fail，但报错后无法再通过上/下键移动焦点。另一个问题是房号输入框中已有数字时，按 `X/*` 应先删除一位，删完后再退出当前页，但旧逻辑会直接退出。

### 问题原因：

问题发生在 `layout_home_id_set.c`。重复房号或已存在校验失败后，`HomeIdSetClass.set_status` 会进入失败状态，而上/下/确认键在主层检测到 `set_status != HOME_ID_SET_STATUS_NONE` 后直接返回，导致错误提示显示期间焦点移动被锁住。同时，失败后当前输入框的 dirty 状态没有清除，提示消失后移动焦点仍可能再次保存同一个错误值并再次失败。`home_id_set_key_star_up()` 原先无条件返回 Output，没有先处理当前输入框退格。

### 解决方法：

将失败/已存在提示改为可被按键关闭的非阻塞提示，上/下/确认/X 操作会先关闭当前错误提示，再继续正常处理焦点移动或返回逻辑；重复房号或已存在校验失败时清除当前输入框 dirty，避免同一个错误值反复阻塞移动；`X/*` 在当前输入框有内容时先删除一位并重画输入框，删空后再次按才返回 Output。

### 涉及文件：

- `app_cu_datin/system/layout/layout_home_id_set.c`
- `tests/test_home_id_set_focus_replace.sh`
- `task_plan.md`
- `findings.md`
- `progress.md`

### 具体修改：

- 新增 `home_id_set_status_prompt_close()`，用于关闭失败/已存在提示并清理提示计数和重绘标志。
- 上/下/确认键进入主层处理时，先调用提示关闭函数，再判断是否仍有不可打断状态。
- 重复房号或已存在校验失败时清除 `home_id_input_dirty[HomeIdSetClass.cur_focus]`。
- `home_id_set_sub_number()` 改为删除光标前一位，避免清错下标。
- `home_id_set_key_star_up()` 改为有输入时先退格并刷新显示，输入为空时才返回 `layout_OutPUT`。
- 扩展 `tests/test_home_id_set_focus_replace.sh`，锁定错误提示关闭、dirty 清理、退格删除和 `X/*` 退出边界。


## 2026-06-09（大楼机通话结束后功放延时恢复）

### 问题描述：

大楼机当前通话或监控结束时会关闭摄像头和咪头，但功放仍保持打开状态。用户要求通话结束时同步关闭摄像头、咪头和功放，并在等待 2 秒后由软件重新打开功放。

### 问题原因：

通话/监控开始路径会打开摄像头、功放和咪头，但结束路径集中在 `monitor_status_check()` 中，`INT_READ_MONITOR_STATUS` 800ms 收尾和 `INT_TALK` 2 分钟超时强制关闭都只关闭了摄像头和咪头，没有关闭功放。直接在结束路径阻塞等待 2 秒会影响 CAN 收包、心跳检查和业务状态释放，因此不能使用 `ak_sleep_ms(2000)` 做同步等待。

### 解决方法：

在通话输出关闭时统一关闭摄像头、咪头和功放，并记录关闭时间；由 `intercom_event_detect()` 周期检查，超过 2 秒后重新打开功放。如果 2 秒内又开始新的通话/监控，先取消上一次延时恢复状态，再立即打开功放，避免影响新通话。

### 涉及文件：

- `app_cu_datin/system/src/intercom.c`
- `app_cu_datin/system/src/intercom.h`
- `tests/test_intercom_amplifier_recovery.sh`
- `docs/superpowers/specs/2026-06-09-intercom-amplifier-delayed-reopen-design.md`
- `docs/superpowers/plans/2026-06-09-intercom-amplifier-delayed-reopen.md`

### 具体修改：

- 新增 `INTERCOM_AMPLIFIER_REOPEN_DELAY_MS 2000`，明确功放延时恢复时间为 2 秒。
- 新增功放延时恢复 pending 状态和 `g_amplifier_reopen_time` 时间记录。
- 新增 `intercom_talk_output_close_then_reopen_amp()`，统一关闭摄像头灯、摄像头电源、咪头和功放。
- 新增 `intercom_amplifier_reopen_check()`，在事件循环中超过 2 秒后重新打开功放。
- `intercom_monitor_start_process()` 新增取消 pending 逻辑，新通话开始时立即恢复功放打开状态。
- `monitor_status_check()` 中两个通话结束路径统一改为调用关闭输出 helper。
- 新增 `tests/test_intercom_amplifier_recovery.sh`，静态校验关闭摄像头/咪头/功放、2 秒恢复、新通话取消 pending 和两个结束路径覆盖。


## 2026-06-09（波斯语设置页和密码等待提示显示修正）

### 问题描述：

波斯语下设置界面的 `کارت دسترسی` 显示区域不足，实际显示会被裁剪或错位，表现为类似 `شرب دسترسی` 的异常文本；输入进入设置密码时，如果连续三次密码错误，倒计时提示在英语 `Please wait %d seconds` 下正常，但波斯语会出现大量方框字符，只能看到数字部分。

### 问题原因：

设置页卡管理文本沿用了英语短文本坐标和宽度，波斯语文本更长且需要按 RTL 视觉方向显示，旧区域过窄并可能压到右侧焦点箭头。密码三次错误倒计时提示原先在代码里硬编码英语字符串，没有进入 `language.c` 的多语言表，也没有给波斯语预留更宽的显示区域。

### 解决方法：

为密码等待倒计时新增多语言字符串，波斯语下使用 `لطفا %d ثانیه صبر کنید` 并扩大显示区域；设置页 `کارت دسترسی` 在波斯语下使用独立坐标、更宽文本框、较小字号和右对齐显示，同时限制右边界不覆盖焦点箭头。

### 涉及文件：

- `app_cu_datin/system/layout/language.c`
- `app_cu_datin/system/layout/language.h`
- `app_cu_datin/system/layout/layout_password.c`
- `app_cu_datin/system/layout/layout_settings.c`

### 具体修改：

- 新增 `STR_PASSWORD_WAIT_SECONDS`，将 `Please wait %d seconds` 纳入多语言表。
- `language.c` 增加波斯语等待提示 `لطفا %d ثانیه صبر کنید`。
- `display_delay_message()` 改为通过 `font_str(STR_PASSWORD_WAIT_SECONDS)` 获取模板，不再硬编码英语。
- 波斯语密码等待提示区域从英语区域扩展为 `{{70, 150}, {340, 40}}`，避免长文本显示不全。
- `set_card_set_font_display()` 在波斯语下改用 `{{300, 96}, {120, 30}}`、字号 `18`、`RIGHT_MIDDLE` 对齐。
- 设置页波斯语卡管理文本会按右侧焦点箭头位置动态限制宽度，避免文字覆盖箭头。


## 2026-06-05（RFID TAG 单删状态返回和 ERASE 三态文案）

### 问题描述：

RFID 单张删卡流程中，TAG 输入或刷卡确认后会回填 `UNIT` 和 `SAVE`，但焦点仍停在 `TAG` 时，按返回键曾被当成 TAG 退格，可能把 `0015153840` 删除成 `001515383`；如果继续编辑已确认 TAG，也可能残留旧 UNIT/SAVE 上下文。另一个显示问题是 `ERASE` 行默认就显示整户删除含义，不利于区分默认状态、整户删除和单张删卡。

### 问题原因：

TAG 确认成功后没有切入与 UNIT 确认一致的确认层，`*` 键处理优先命中主层 TAG 输入退格逻辑；已确认 TAG 再编辑时，旧逻辑只取消 TAG 确认状态，没有同步清理由 TAG 反查出来的 UNIT/SAVE。`ERASE` 文案原先只按 TAG 是否确认做二分，未区分默认状态和 UNIT 房间操作状态。

### 解决方法：

TAG 确认成功后进入确认层并关闭主层 TAG 刷卡输入请求，使返回键走统一 reset 流程；已确认 TAG 被继续输入或退格前，先退出本次单删确认上下文并清理旧 UNIT/SAVE；`ERASE` 行改为三态显示：默认 `ERASE`，确认 UNIT 后显示 `ERASE ALL`，确认 TAG 后显示 `ERASE TAG`。

### 涉及文件：

- `app_cu_datin/system/layout/layout_card_manage.c`
- `app_cu_datin/system/layout/language.c`
- `app_cu_datin/system/layout/language.h`
- `tests/test_card_prompt_interaction.sh`

### 具体修改：

- 新增 `card_manage_cancel_confirmed_tag_context()`，已确认 TAG 被继续输入或退格前，清除旧 UNIT、旧 SAVE、删除用 TAG 缓存和房号缓存。
- TAG 确认成功后设置 `CardManageClass.cur_focus.layer = CARD_MANAGE_MAIN_LAYER_CONFIRM`，并调用 `card_manage_update_tag_fill_request()`，避免返回键继续按 TAG 输入退格处理。
- 默认 `ERASE` 文案去掉冒号，旧 `ERASE ROOM` 改为 `ERASE ALL`。
- `Erase_font_display()` 改为三态选择：默认显示 `ERASE`，`card_manage_room_operation_active` 时显示 `ERASE ALL`，`card_manage_tag_confirmed` 时显示 `ERASE TAG`。
- `STR_CARD_MANAGE_ERASE_ROOM` 重命名为 `STR_CARD_MANAGE_ERASE_ALL`，同步更新多语言表。
- 扩展静态测试，校验 TAG 确认后进入确认层、已确认 TAG 编辑会退出单删上下文、`ERASE` 文案三态显示，并禁止旧 `ERASE ROOM`。

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
## 2026-06-04（RFID 房间操作状态隔离、残留清理和按键音修正）

### 问题描述：

确认 `UNIT` 进入房间操作后，如果焦点移动到 `TAG` 或刷卡，容易让用户误以为进入了单张删卡或卡未添加；确认 UNIT 后未保存直接返回，再进入 TAG 输入错误卡号，弹窗关闭后可能重画上一次留下的 `SAVE:0` 或旧 TAG 卡号；另外在 TAG 焦点刷入未保存卡后，错误提示消失可能导致按键音消失。

### 问题原因：

UNIT 房间操作和 TAG 单张删卡曾共用部分焦点和刷卡填充路径，房间操作中的刷卡显示可能写入 TAG 单删输入缓存；未保存退出路径没有统一清理房间卡数量、旧刷卡 TAG、TAG 输入和成功显示标志；`CARD_TAG_FILL_MODE` 结束时依赖可能已被提示框清掉的 `tag_fill_request` 判断返回模式，导致主层 TAG 刷卡失败后可能错误回到 `CARD_ADD_CARD_MODE`，而该模式会抑制普通按键音。

### 解决方法：

通过显式房间操作状态隔离 UNIT 路径和 TAG 单删路径：确认 UNIT 后进入房间操作模式，TAG 行只作为当前卡号显示，不再接收键盘输入、退格或刷卡填充；未保存返回和 TAG 错误关闭时统一清理临时结果；TAG 填充流程结束后按当前页面层级决定回到 idle 还是 add-card，避免错误提示后按键音丢失。

### 涉及文件：

- `app_cu_datin/system/layout/layout_card_manage.c`
- `app_cu_datin/system/src/swiping_card.c`
- `tests/test_card_prompt_interaction.sh`

### 具体修改：

- 新增 `card_manage_room_operation_active`，确认 UNIT 成功后进入房间操作模式，退出或成功提示关闭时离开该模式。
- 新增 `card_manage_tag_entry_enabled()`，只有 RFID 主层、TAG 焦点、未进入房间操作且无提示框时，才允许 TAG 键盘输入或刷卡填充。
- 房间操作模式下，TAG 行不再接收数字输入、退格或刷卡单删填充，避免客户误以为可以通过 TAG 输入添卡。
- TAG 单删准备成功不播放成功音；卡不存在或卡号错误时播放错误音并显示 `Card number error`。
- TAG 反查补全 UNIT 时，显示层按直接输入格式显示，短房号如 `9` 不再显示为 `0009`。
- 新增 `card_manage_clear_transient_result_state()`，统一清理 SAVE 数量、旧刷卡 TAG、TAG 输入、TAG 确认状态和刷卡成功标志。
- 新增 `card_manage_reset_main_input_state()`，从确认层按 `*` 返回主界面前回到 `UNIT_FOCUS`、`CARD_MANAGE_MAIN_LAYER`、`CARD_IDLE_MODE`，并清空旧房号输入和临时结果。
- TAG 错误提示关闭时不再按旧 `CARD_ADD_CARD_MODE` 缓存重画 TAG/SAVE，避免错误弹窗消失后旧值重新出现。
- 新增 `card_manage_card_result_display()`，房间操作中的刷卡结果只做原始卡 ID 到屏幕 TAG 数字的显示转换，不修改 TAG 单删输入状态。
- `CARD_TAG_FILL_MODE` 结束时按 `CardManageClass.cur_focus.layer` 返回：RFID 主层回 `CARD_IDLE_MODE`，确认层房间操作回 `CARD_ADD_CARD_MODE`，避免 TAG 错误提示后按键音消失。

## 2026-06-04（RFID TAG 单张删卡与同级输入）

### 问题描述：

RFID 原有删除逻辑主要面向整户删除，无法在主界面直接输入或刷入 `TAG` 后删除单张卡；早期单删方案还存在必须先确认 `UNIT`、`TAG` 输入区域遮挡右侧箭头、`TAG confirmed` 弹窗观感奇怪、单删错误显示房号错误、`ERASE ROOM` 显示不全等问题。

### 问题原因：

旧逻辑没有独立的 TAG 输入和确认状态，刷卡数据存储为原始 `ID:` 十六进制字符串，而界面显示的是转换后的纯数字 TAG；同时 TAG 操作路径曾绑定在 `CARD_MANAGE_MAIN_LAYER_CONFIRM`，导致 TAG 不能与 UNIT 同级输入，删除时也容易被当前 UNIT 输入框状态误导。

### 解决方法：

将 `TAG` 与 `UNIT` 作为 RFID 主界面同级入口：`TAG` 焦点可手动输入纯数字卡号，也可刷已保存卡自动填充；TAG 确认时从已保存卡数据中反查所属房号，自动补全 UNIT 并刷新该房号卡数；`ERASE` 行用 `ERASE ROOM` / `ERASE TAG` 区分整户删除和单张删除，TAG 错误统一显示 `Card number error`。

### 涉及文件：

- `app_cu_datin/system/layout/layout_card_manage.c`
- `app_cu_datin/system/layout/layout_card_manage.h`
- `app_cu_datin/system/layout/language.c`
- `app_cu_datin/system/layout/language.h`
- `app_cu_datin/system/src/swiping_card.c`
- `app_cu_datin/system/src/swiping_card.h`
- `tests/test_card_prompt_interaction.sh`
- `docs/superpowers/plans/2026-06-04-rfid-tag-peer-input.md`

### 具体修改：

- 新增 TAG 输入缓冲、TAG 确认状态和 TAG 输入替换标志，支持主界面直接输入纯数字 TAG。
- 新增原始卡 ID 到屏幕 TAG 纯数字的转换逻辑，TAG 格式保留前导 0，例如 `0014246460`。
- 新增按 TAG 反查已保存卡槽、按原始卡 ID 判断是否已保存、按 TAG 反查房号并回填 UNIT 的逻辑。
- 新增 `CARD_TAG_FILL_MODE` 和 `SwipingCard.tag_fill_request`，TAG 焦点刷卡只填充 TAG，不走普通开门验证。
- TAG 刷卡填充前先确认卡已保存；未保存卡显示 `Card number error` 并播放错误音。
- TAG 确认后不再弹出 `TAG confirmed`，而是刷新 UNIT、SAVE 卡数和 ERASE 文案。
- `ERASE` 行改为动态显示：TAG 未确认或无 TAG 时显示 `ERASE ROOM`，TAG 确认后显示 `ERASE TAG`。
- `ERASE` 删除逻辑调整为：TAG 为空时删除当前 UNIT 整户；TAG 有值且确认成功时删除反查到的单张卡；TAG 未确认或找不到时显示 `Card number error`。
- 删除单张卡后重新统计当前房号 10 个卡槽的非空数量，避免中间空槽导致数量显示错误；如果该房号已无卡，则从 `UserData.unit_number[]` 中移除该房号。
- TAG 显示区域收窄到 `{{160, 71}, {240, 40}}`，避免覆盖右侧选择箭头。
- `ERASE` 文案显示区域加宽到 `{{33, 114}, {200, 40}}`，避免 `ERASE ROOM` 显示不全。
- 新增 `Card number error`、`ERASE ROOM`、`ERASE TAG` 等多语言字符串。





## 2026-06-03（RFID 保存/删除提示关闭后焦点回 UNIT）

### 问题描述：

RFID 卡管理页中，输入房间号进入添卡或删卡操作后，如果在 `SAVE` 或 `ERASE` 上触发成功提示，提示关闭后选择箭头仍停留在 `SAVE` 或 `ERASE`，连续操作下一户时不够方便。

### 问题原因：

提示框关闭逻辑只清除状态并重绘页面，没有根据保存或删除成功场景重置当前焦点；如果在页面重绘后再单独移动焦点，还可能留下旧焦点箭头残影。

### 解决方法：

关闭保存或删除成功提示时，先将焦点状态切回 `UNIT_FOCUS` 和 `CARD_MANAGE_MAIN_LAYER`，再统一重绘 RFID 页面。错误提示不强制回 UNIT，避免用户处理当前错误时丢失操作位置。

### 涉及文件：

- `app_cu_datin/system/layout/layout_card_manage.c`
- `tests/test_card_prompt_interaction.sh`

### 具体修改：

- 新增保存/删除成功提示关闭后的焦点重置逻辑。
- 仅在关闭 `CARD_MANAGE_STATUS_DELETE_CARD` 或 `CARD_MANAGE_STATUS_SAVE_CARD` 时回到 `UNIT`。
- 焦点状态先重置再整页重绘，避免旧 `SAVE/ERASE` 箭头残留。
- 扩展静态测试，校验提示关闭后回到 `UNIT_FOCUS` 和主层。






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
