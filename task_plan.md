# 任务计划：制作当前风险版本复现升级包

## 目标
不修复源码，基于当前工程原始风险代码重新制作 `HALL_MACHINEOS`，用于整机验证绑定数量到 256 后是否出现卡死问题。

## 阶段

1. [complete] 恢复源码到风险版本
   - 已恢复 `swiping_card.c`、`user_data.c`、`layout_logo.c`、`layout_home_id_set.c` 到 HEAD 内容。
   - 静态确认 5 处 `unsigned char` + `get_int_conf(...)` 风险循环存在。

2. [complete] 重新制作未修复升级包
   - 重新编译应用并拷贝到 SDK。
   - 重新生成升级包 `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`。

3. [complete] 验证和交付
   - 确认源码无修复 diff。
   - 确认升级包存在、大小合理、包头有版本号。
   - 给出整机复现步骤。

4. [complete] 制作 DATA 预置测试包
   - 预置 `/app/data/data.ini` 中 `index3 = 255`。
   - 预置 `/app/data/user_data` 中 255 个不同房号。
   - 预置空 `card_data`，让测试只需要一张真实卡录第 256 个房号。
   - 只打包 `data.jffs2`，不更新 APP、系统或启动相关分区。
   - 输出包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`。

5. [complete] 修复 256 条后死循环并制作验证包
   - 先用静态检查确认风险循环仍存在。
   - 将按配置计数循环的下标从 `unsigned char` 改为不会 255 回绕的整数类型。
   - 保留 DATA 预置：`index3 = 255`，房号 `1001..1255`。
   - 重新构建 APP，并制作包含修复 APP + 预置 DATA 的验证升级包。
   - 输出包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`。

6. [complete] 修复卡管理房号错误提示状态残留
   - 错误提示显示期间禁止继续输入/删除房号，避免输入内容与 `Room number error` 重叠。
   - 进入和退出卡管理界面时重置状态提示计数，避免下次进入后错误提示不再显示。
   - 只修改 `app_cu_datin/system/layout/layout_card_manage.c`。
   - 已完成静态 diff 检查和构建验证。

7. [complete] 底部 TABA logo 文本改为跟随语言切换
   - 用户确认方案A：保留新的纯图片 `taba_icon.png`，程序叠加绘制文本。
   - 只修改底部 `taba_icon.png` 对应显示逻辑，不处理开机页 `TB_LG.png`。
   - 增加多语言文本 ID，并在 `taba_btn_display()` 中绘制较大的 `TABA` 和较小的 `Electronics`。
   - 使用静态回归测试、APP 编译和 APP-only 升级包验证。

8. [complete] 调整底部 TABA logo 文本垂直居中和波斯语 RTL 排布
   - 整体文字下移到 logo 颜色带中心区域。
   - 英语保持左 TABA、右 Electronics。
   - 波斯语改为右侧 `تابا`、左侧 `الکترونیک`，缩小两者间距。
   - 重新跑静态测试、APP 编译和 APP-only 打包验证。

9. [complete] 修复语言切换即时重绘和 calling 页面残点
   - 继续收紧波斯语底部 logo 两段文字间距。
   - 在系统设置语言切换时立即重绘底部 logo。
   - 在 calling 页面统一清除文本区域，修复四位英语房号无应答后右侧 `.` 残留。
   - 重新跑静态测试、APP 编译和 APP-only 打包验证。

10. [complete] 大楼机广播心跳和分支器断线 LED 闪烁
   - 用户确认方案A：主站广播心跳，分支器只接收不回复。
   - 新增设计文档：`docs/superpowers/specs/2026-05-25-brancher-heartbeat-led-design.md`。
   - 新增实施计划：`docs/superpowers/plans/2026-05-25-brancher-heartbeat-led.md`。
   - 新增静态回归测试：`tests/test_brancher_heartbeat.sh`。
   - 大楼机每 1 秒发送 `CMD_HEARTBEAT`。
   - 分支器 5 秒未收到心跳后 `POWER_LED` 每 500ms 闪烁，收到心跳后恢复常亮。
   - 分支器心跳处理不发送 ACK，避免多分支器场景下形成回复风暴。
   - 已重新跑静态测试、APP 编译和 APP-only 打包验证。

11. [complete] 心跳避让业务通信验证读房号问题
   - 用户实测无心跳版本连续几十次读房号不复现，要求先做短期“心跳避让业务通信”验证。
   - 新增实施计划：`docs/superpowers/plans/2026-05-26-intercom-heartbeat-deferral.md`。
   - 新增静态回归测试：`tests/test_intercom_heartbeat_deferral.sh`。
   - 大楼机侧业务发送和有效接收都会刷新总线活动时间。
   - 心跳检查发现 `500ms` 内有业务活动时，延后本次心跳发送。
   - 保留分支器固件和心跳协议不变，用于快速整机验证读房号不显示是否消失。

12. [complete] 心跳避让业务通信正式实现
   - 用户确认将短期验证改为正式逻辑，并要求覆盖读房号、设房号、呼叫、监控、开锁、读监控状态等业务通信。
   - 新增设计文档：`docs/superpowers/specs/2026-05-26-intercom-heartbeat-busy-design.md`。
   - 新增实施计划：`docs/superpowers/plans/2026-05-26-intercom-heartbeat-busy.md`。
   - 大楼机侧补充 `busy` 状态、`500ms` 恢复延时、`4000ms` 超时自动释放，并将接收处理前置到心跳检查前。
   - 分支器侧将“在线刷新”从心跳专用扩展到任意成功解析的主站命令，避免业务期误判断线闪灯。
   - 本次不并入“读房号重读”，避免在未实现“退出即取消事务、忽略旧回复”前引入新的界面状态污染风险。

13. [complete] 卡片提示弹框、输入框排版和英文 Logo 间距
   - 文件计划：
     - `app_cu_datin/system/layout/layout_card_manage.c`：卡管理页错误/添卡成功/删卡成功提示改为可取消弹框，任意按键先关闭，约 2 秒自动关闭。
     - `app_cu_datin/system/layout/layout_card_number.c`：房号输入/删卡页提示改为可取消弹框，删除成功文案独立显示。
     - `app_cu_datin/system/layout/language.h`、`app_cu_datin/system/layout/language.c`：新增 `Add card success`、`Delete card success` 文案。
     - `app_cu_datin/system/layout/layout_base.c`：收紧英文底部 `TABA` 与 `Electronics` 的显示间距。
     - `tests/test_card_prompt_interaction.sh`、`tests/test_bottom_logo_language.sh`：新增/更新静态回归检查。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh`、`bash tests/test_bottom_logo_language.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh`、`bash tests/test_brancher_heartbeat.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过，生成 `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`。
     - 包验证按既有方式单独执行 `partition_image.sh app_resource`，只打包 APP 分区。

14. [complete] 修正卡管理弹窗观感、残留清理和 Logo 裁剪
   - 文件计划：
     - `app_cu_datin/system/layout/layout_base.c`：恢复英文 `TABA` 足够宽度，收紧但不裁剪 `Electronics` 间距。
     - `app_cu_datin/system/layout/layout_card_manage.c`：弹窗背景改为 `#5D7798`，使用不透明擦除覆盖底字；关闭弹窗后重绘上半页，清掉残留；统一 UNIT/TAG/ERASE/SAVE 右侧列 x 坐标。
     - `app_cu_datin/system/layout/layout_card_number.c`：弹窗背景改为 `#5D7798`，关闭后重绘上半页。
     - `tests/test_card_prompt_interaction.sh`、`tests/test_bottom_logo_language.sh`：更新静态验收。
   - 明确约束：
     - 不修改 `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh`。
     - 构建后只打 APP 分区时，使用既有 `partition_image.sh app_resource`。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh`、`bash tests/test_bottom_logo_language.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh`、`bash tests/test_brancher_heartbeat.sh` 通过。
     - `cd app_cu_datin && make` 通过且无新增警告。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过，期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”。
     - `cd AK37E_SDK_V1.03/upgrade && env upgrade_bin_version=20260529172629 ./partition_image.sh app_resource` 通过。
     - 最终 `HALL_MACHINEOS` 大小 569K，分区头为 `# File Parttion: app.sqsh4 0 581632`。
     - 编译脚本 diff 为空。

15. [complete] 缩小卡管理提示框并清除保存/删除后的 TAG/SAVE 残留
   - 文件计划：
     - `app_cu_datin/system/layout/layout_card_manage.c`：提示框改为 `left=115, top=59, width=250, height=75`；保存/删除/成功提示关闭后清空 TAG 和 SAVE 后面的结果字段，并在退出添卡模式时清掉 `SwipingCard.success_show`，避免旧值被定时重画。
     - `app_cu_datin/system/layout/layout_card_number.c`：房号提示框同步改为 `left=115, top=59, width=250, height=75`。
     - `tests/test_card_prompt_interaction.sh`：增加提示框坐标尺寸和结果字段清理的静态验收。
   - 说明：
     - 当前 UI 库只有矩形 `gui_erase()` / `draw_rect()`，没有圆角矩形 API；本轮实现尺寸、位置、颜色和不透明度，暂不实现真实 `border-radius: 6px`。
   - 验证计划：
     - `bash tests/test_card_prompt_interaction.sh`
     - `bash tests/test_bottom_logo_language.sh`
     - `cd app_cu_datin && make`
     - `cd app_cu_datin && ./autobuild.sh -all-sdk`
     - `cd AK37E_SDK_V1.03/upgrade && env upgrade_bin_version=20260529174518 ./partition_image.sh app_resource`

16. [complete] RFID 提示框改用 rfid_focus 资源并区分文字颜色
   - 文件计划：
     - `app_cu_datin/system/layout/layout_card_manage.c`：卡管理提示框不再使用纯色矩形，改为显示 `rfid_focus.png`；错误提示文字红色，成功提示文字白色。
     - `app_cu_datin/system/layout/layout_card_number.c`：删卡房号页提示框同步改为 `rfid_focus.png`；错误红色，成功白色。
     - `tests/test_card_prompt_interaction.sh`：更新静态验收，禁止旧纯色提示框，校验 `rfid_focus` 资源、文字颜色和新位置。
   - 用户指定位置：
     - 最终参考用户指定位置调整为 `left=152, top=73`。
     - 资源自身尺寸为 `236x66`，因此显示区域为 `{{152, 73}, {236, 66}}`。
     - UNIT/TAG/ERASE/SAVE 右侧输入/输出列统一移到 `x=160`，避免压到 `ERASE:` 标签显示区域。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `git diff --check -- app_cu_datin/system/layout/layout_card_manage.c app_cu_datin/system/layout/layout_card_number.c tests/test_card_prompt_interaction.sh` 通过。
     - `cd app_cu_datin && make` 通过。

17. [complete] 修正提示框后暴露的待机时钟、字体缓冲和删卡越界风险
   - 文件计划：
     - `app_cu_datin/ui_lib/analog_clock.c`：修正 `clock_dot_buffer` 释放对象错误，并清零 `analog_clock_dst_buffer`。
     - `app_cu_datin/ui_lib/font_decodec.c`：字体临时缓冲分配失败时安全退出，分配成功后清零，减少脏像素/阴影残留。
     - `app_cu_datin/system/layout/layout_card_manage.c`：删卡线程按 `home_id` 查找并移除 `UserData.unit_number[]` 项，避免把 `home_id * 10` 当数组下标。
     - `app_cu_datin/system/layout/layout_card_number.c`：删卡房号页同步修正相同删除风险。
     - `findings.md`、`progress.md`：补充根因、风险和验证记录。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_bottom_logo_language.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c app_cu_datin/system/layout/layout_card_number.c app_cu_datin/ui_lib/analog_clock.c app_cu_datin/ui_lib/font_decodec.c findings.md progress.md task_plan.md` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 在沙箱内仍打印已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `platform/config.jffs2`、`platform/data.jffs2`、`platform/tuya.jffs2`，避免 `autobuild` 后平台目录残缺。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260602144956 && ./partition_image.sh app_resource` 通过。
     - 最终 `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS` 只包含 `app.sqsh4`，大小 `594031 bytes`，包头 `# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

18. [complete] UNIT 房号设置页切换焦点不清空旧房号
   - 文件计划：
     - `app_cu_datin/system/layout/layout_home_id_set.c`：M1-M4 房号输入框上下/确认切换焦点时不再清空当前对话框内容；切换到已有内容输入框后，第一次数字输入前清空旧值并用新输入替换。
     - `tests/test_home_id_set_focus_replace.sh`：新增静态回归检查，锁定“焦点切换不清空、输入前替换”的行为。
   - 行为说明：
     - 读房号得到旧值或用户输入过 `1` 后，键盘上下选择到该框不会清掉显示。
     - 在该框开始输入 `22` 时，会先清掉旧的 `1` 或旧房号，再显示 `2 2`，不会变成 `1 2 2`。
     - 连续输入期间不会重复清空，避免第二个数字覆盖第一个数字。
   - 验证结果：
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_home_id_set.c tests/test_home_id_set_focus_replace.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 在沙箱内仍打印已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`，避免 `autobuild` 后平台目录残缺。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260603084103 && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

19. [complete] UNIT 房号设置页仅本次输入后才保存
   - 文件计划：
     - `app_cu_datin/system/layout/layout_home_id_set.c`：为 M1-M4 增加本次输入 `dirty` 状态；上/下/OK 切换焦点时只有当前框本次输入过数字才触发保存。
     - `tests/test_home_id_set_focus_replace.sh`：扩展静态检查，禁止按键处理函数直接调用 `set_home_id_number()`，要求通过 dirty guard 保存。
   - 行为说明：
     - 读出来的旧房号或已经保存过的房号，上下移动再次经过时不再重复保存，也不会因为重复校验变黄。
     - 用户本次输入数字后，按上/下/OK 才会保存；保存成功或发起保存后清除 dirty，避免二次路过重复触发。
     - 如果保存失败或进入“房号已存在”确认框，不切换焦点，保留输入状态给用户处理。
   - 验证结果：
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_home_id_set.c tests/test_home_id_set_focus_replace.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 在沙箱内仍打印已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`，避免 `autobuild` 后平台目录残缺。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260603100302 && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。
