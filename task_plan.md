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

20. [complete] RFID 保存/删卡提示关闭后焦点回 UNIT
   - 文件计划：
     - `app_cu_datin/system/layout/layout_card_manage.c`：保存/删除成功提示关闭后，将 `CardManageClass.cur_focus.main` 重置为 `UNIT_FOCUS`，并回到 `CARD_MANAGE_MAIN_LAYER`。
     - `tests/test_card_prompt_interaction.sh`：扩展静态回归，锁定保存/删除提示关闭后先重置焦点状态、再重绘页面。
   - 行为说明：
     - 用户输入房号进入添卡模式后，如果在 `SAVE:` 或 `ERASE:` 触发成功提示，提示按任意键关闭或超时关闭后，箭头回到 `UNIT:`。
     - 错误提示不重置焦点，避免用户处理错误时丢失当前操作位置。
     - 焦点状态先改回 `UNIT_FOCUS` 再整页重绘，避免先绘制旧 `SAVE/ERASE` 焦点后再补画 `UNIT` 造成双箭头残留。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c tests/test_card_prompt_interaction.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 在沙箱内仍打印已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`，避免 `autobuild` 后平台目录残缺。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260603105305 && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

21. [complete] RFID TAG 单张删卡
   - 文件计划：
     - `app_cu_datin/system/layout/layout_card_manage.c`：新增 TAG 纯数字输入缓冲、刷卡填充 TAG、当前 UNIT 内按 TAG 匹配并删除单张卡。
     - `app_cu_datin/system/layout/layout_card_manage.h`：暴露 `card_manage_fill_tag_by_card_id()` 给刷卡状态机调用。
     - `app_cu_datin/system/src/swiping_card.c`、`app_cu_datin/system/src/swiping_card.h`：新增 `CARD_TAG_FILL_MODE` 和 `tag_fill_request`，TAG 焦点下刷卡只填充 TAG，不新增卡。
     - `tests/test_card_prompt_interaction.sh`：扩展静态回归，锁定 TAG 输入、刷卡填充、单删和计数规则。
   - 行为说明：
     - `TAG:` 输入格式为屏幕显示的纯数字字符串，保留前导 0，例如 `0014246460`。
     - `TAG` 有值时，`ERASE` 只删除当前 UNIT 下匹配的这一张卡。
     - `TAG` 为空或当前 UNIT 下匹配不到时，`ERASE` 不执行整户删除，只提示错误。
     - 单删后卡槽会出现空位，后续新增卡仍会按原 `save_card_id()` 逻辑填入第一个空 slot；房间卡数量改为遍历 10 个 slot 统计非空，避免中间空位导致计数错误。
     - 删除后如果该 UNIT 已无任何卡，则移除 `UserData.unit_number[]` 中的房号；如果仍有其他卡则保留房号。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c app_cu_datin/system/layout/layout_card_manage.h app_cu_datin/system/src/swiping_card.c app_cu_datin/system/src/swiping_card.h tests/test_card_prompt_interaction.sh` 通过。
     - `cd app_cu_datin && make` 通过且无新增警告。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 在沙箱内仍打印已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`，避免 `autobuild` 后平台目录残缺。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260603120557 && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

22. [complete] RFID TAG 与 UNIT 同级输入方案
   - 目标：
     - `TAG:` 与 `UNIT:` 在 RFID 主界面同级选择，不再要求先确认 UNIT 才能输入 TAG。
     - `TAG:` 焦点下可手动输入纯数字 TAG，也可刷卡自动填充 TAG。
     - `TAG:` 输入完成后按确认标记本次 TAG 已确认，之后到 `ERASE:` 才允许删除对应单张卡。
     - TAG 输入显示区域右边界避开最右侧选择箭头，避免文字或清屏区域覆盖箭头。
   - 实现结果：
     - `TAG:` 主层即可手动输入纯数字 TAG，`*` 可删除 TAG 数字。
     - `TAG:` 焦点下刷卡会填充 TAG，不再要求先确认 UNIT；普通开门刷卡路径被避让。
     - `TAG:` 按确认后设置本次 TAG 已确认；TAG 继续输入、删除、清空或刷卡重填会取消确认。
     - `ERASE:` 删除前必须通过 UNIT 有效、TAG 有值、TAG 已确认、当前 UNIT 下存在该 TAG 四项检查。
     - TAG 显示和清理区域改为 `{{160, 71}, {240, 40}}`，避开右侧箭头区域。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- ...` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `platform/config.jffs2`、`platform/data.jffs2`、`platform/tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260604103253 && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

23. [complete] RFID 删除路径和 TAG 提示修正
   - 问题：
     - 输入短房号如 `9` 后，删除路径会把内部房号补齐成 `0009` 并重画输入框，导致旧 `9` 与新 `0` 重叠。
     - RFID 页旧的整户删除线程在单张删卡实现中被替换，导致 TAG 为空时整户删除不可用。
     - 单删失败和房号失败共用 `Room number error`，用户无法区分是房号错还是卡号错。
     - TAG 按确认后没有明显提示，用户不知道 TAG 是否已确认。
   - 修复：
     - `card_manage_prepare_unit_for_card_action()` 仍做内部右对齐，但不再重画 `0009` 到输入框，避免残影/重叠。
     - 恢复 RFID 页 `delete_current_card()` 整户删除线程，并沿用按 `home_id` 安全移除房号列表的逻辑。
     - `ERASE:` 行为改为：TAG 为空时删除当前 UNIT 的整户卡；TAG 有值时必须先确认 TAG，再删除当前 UNIT 下匹配的单张卡。
     - 上一版新增 `Card number error` 和 `TAG confirmed` 两个提示文案；TAG 错误不再显示房号错误。其中 `TAG confirmed` 弹窗已在阶段 24 废弃。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c app_cu_datin/system/layout/layout_card_manage.h app_cu_datin/system/layout/language.c app_cu_datin/system/layout/language.h tests/test_card_prompt_interaction.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `platform/config.jffs2`、`platform/data.jffs2`、`platform/tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260604114904 && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

24. [complete] RFID TAG 确认去弹窗和 ERASE 文案区分
   - 问题：
     - `TAG confirmed` 弹窗会让用户感觉奇怪，也会打断当前 RFID 操作流。
     - 用户期望 TAG 确认后像 UNIT 确认一样刷新当前房号信息和卡数量，并通过 `ERASE` 行文字明确下一步操作。
     - 有 TAG 时按 `ERASE` 不应再走房号错误语义，TAG 未确认或当前房号下找不到该卡时应提示卡号错误。
   - 修复：
     - 移除 `TAG confirmed` 弹窗文案和确认弹窗路径，TAG 确认只更新内部 `card_manage_tag_confirmed` 状态。
     - TAG 确认时校验当前 UNIT，刷新当前房号卡数量，并在当前 UNIT 下匹配 TAG 后自动回填标准 TAG 显示值。
     - `ERASE` 行改为动态文案：未确认 TAG 显示 `ERASE ROOM`，确认 TAG 后显示 `ERASE TAG`，不再使用 `ERASE:` 冒号语义。
     - `ERASE` 删除逻辑保持兼容：TAG 为空删除整户；TAG 有值但未确认或不匹配显示 `Card number error`；TAG 已确认且匹配时删除单张卡。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c app_cu_datin/system/layout/layout_card_manage.h app_cu_datin/system/layout/language.c app_cu_datin/system/layout/language.h app_cu_datin/system/src/swiping_card.c app_cu_datin/system/src/swiping_card.h tests/test_card_prompt_interaction.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260604124527 && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

25. [complete] RFID TAG 反查房号单删修正
   - 问题：
     - TAG 焦点刷卡填充后按确认仍显示房号错误。
     - 有 TAG 后移动到 `ERASE` 删除仍可能显示房号错误，无法单删。
     - 未保存卡刷入 TAG 焦点时不应按成功填充处理，应提示卡号错误。
     - `ERASE ROOM` 显示范围不够长。
   - 修复：
     - TAG 确认不再先校验当前 UNIT，而是通过 TAG 全局反查已保存卡槽，找到后自动补全 `UNIT` 房号。
     - TAG 确认成功后刷新该房号卡数量到 `SAVE`，并切换 `ERASE` 文案到 `ERASE TAG`。
     - `ERASE TAG` 删除按已保存 TAG 反查到的卡槽房号执行单张删除，不再被当前 UNIT 输入状态误导。
     - 刷卡填充 TAG 时先确认原始卡 ID 已保存；未保存卡显示 `Card number error`。
     - UNIT 确认成功会清掉 TAG 状态并回到 `ERASE ROOM` 整户删除模式。
     - `ERASE` 文案区域加宽到 `{{33, 114}, {200, 40}}`。
   - 验证结果：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c tests/test_card_prompt_interaction.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260604160654 && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

26. [complete] RFID UNIT 房间操作与 TAG 单删入口互斥
   - 问题：
     - 确认 `UNIT` 后已经进入房间操作/添卡模式，但焦点仍可移动到 `TAG`，刷已保存卡会被判定为 TAG 单删准备成功，容易让客户误以为这张卡没有添加过。
     - 确认 `UNIT` 后如果 `TAG` 仍可键盘输入，会让客户误以为可以通过输入卡号去添卡，但当前系统并没有这个功能。
     - TAG 单删准备成功不应播放成功音；只有卡号不存在时播放错误音并显示 `Card number error`。
     - TAG 反查补全 `UNIT` 的显示应与直接输入一致，例如房号 `9` 显示 `9`，不显示 `0009`。
   - 修复计划：
     - `app_cu_datin/system/layout/layout_card_manage.c`：新增房间操作状态，确认 `UNIT` 成功后进入房间操作模式；房间操作模式下 `TAG` 行不再接收键盘输入、退格或刷卡单删填充。
     - `app_cu_datin/system/src/swiping_card.c`：TAG 填充流程只由 `SwipingCard.tag_fill_request` 触发，不再凭 `TAG_FOCUS` 触发；TAG 填充成功不播放成功音，失败播放错误音。
     - `tests/test_card_prompt_interaction.sh`：扩展静态回归，锁定 UNIT/TAG 路径互斥、TAG 错误音和 UNIT 显示规则。
   - 已完成验证：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c app_cu_datin/system/src/swiping_card.c tests/test_card_prompt_interaction.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && env upgrade_bin_version=20260604171110 ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

27. [complete] RFID 未保存退出后 TAG/SAVE 残留清理
   - 问题：
     - 在 RFID 中确认 `UNIT=9` 后，如果没有保存直接返回，再进入 `TAG` 输入不存在卡号，错误弹窗关闭后会重画上一次 UNIT 路径留下的 `SAVE:0`。
     - 如果确认 UNIT 期间刷过卡，旧 `SwipingCard.string_buf[10]` 仍保留，后续 TAG 错误弹窗关闭后还会重画旧 `TAG:0015153840`。
     - 保存后不复现，因为保存成功提示关闭会走已有结果清理；未保存直接返回没有走同等清理。
   - 修复计划：
     - `app_cu_datin/system/layout/layout_card_manage.c`：新增统一 transient 结果清理函数，清掉 `SAVE` 数量、旧刷卡 TAG、TAG 输入确认状态和刷卡成功标志。
     - `app_cu_datin/system/layout/layout_card_manage.c`：从 `CARD_MANAGE_MAIN_LAYER_CONFIRM` 按 `*` 返回 RFID 主层前，回到 `UNIT_FOCUS` / `CARD_MANAGE_MAIN_LAYER` / `CARD_IDLE_MODE`，并清空旧房号输入和临时结果。
     - `app_cu_datin/system/layout/layout_card_manage.c`：关闭 `CARD_MANAGE_STATUS_TAG_ERROR` 时不按旧 `CARD_ADD_CARD_MODE` 缓存重画 TAG/SAVE。
     - `tests/test_card_prompt_interaction.sh`：新增静态回归锁定未保存返回和 TAG 错误关闭的清理路径。
   - 已完成验证：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c tests/test_card_prompt_interaction.sh task_plan.md findings.md progress.md` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=$(date +%Y%m%d%H%M%S) && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

28. [complete] RFID TAG 刷卡失败按键音和 UNIT 刷卡删除语义修正
   - 问题：
     - RFID 主界面索引到 `TAG` 后，如果刷入未保存卡，会播错误提示并显示 `Card number error`，但提示消失后按键音消失。
     - RFID 输入并确认 `UNIT` 后，如果在房间操作状态刷卡，整户删除功能会失效，表现像被切到了 TAG 单张删除语义。
   - 修复计划：
     - `app_cu_datin/system/src/swiping_card.c`：`CARD_TAG_FILL_MODE` 结束时按当前 RFID 层级返回，主层 TAG 刷卡失败必须回到 `CARD_IDLE_MODE`，不能依赖可能已被提示框清掉的 `tag_fill_request`。
     - `app_cu_datin/system/layout/layout_card_manage.c`：房间操作中的刷卡结果只用于显示当前刷卡号，不写入 TAG 单删输入状态，避免 `ERASE ROOM` 被普通刷卡污染成单删语义。
     - `tests/test_card_prompt_interaction.sh`：扩展静态回归，锁定 TAG 错误返回模式和 UNIT 房间操作刷卡显示/输入状态隔离。
   - 已完成验证：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=$(date +%Y%m%d%H%M%S) && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 593920`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

29. [complete] README 补充 RFID 修改记录
   - 用户要求：
     - 将后续已经完成但 `README.md` 中尚未记录的修改按既有格式补进去。
   - 已补充：
     - `2026-06-03（RFID 保存/删除提示关闭后焦点回 UNIT）`。
     - `2026-06-04（RFID TAG 单张删卡与同级输入）`。
     - `2026-06-04（RFID 房间操作状态隔离、残留清理和按键音修正）`。
   - 覆盖范围：
     - 保存/删除成功提示关闭后焦点回 UNIT。
     - TAG 单张删卡、TAG 与 UNIT 同级输入、刷卡填充 TAG、ERASE ROOM/TAG 动态文案。
     - UNIT 房间操作与 TAG 单删入口互斥。
     - 未保存退出后的 TAG/SAVE 残留清理。
     - TAG 刷不存在卡后按键音消失修正。

30. [complete] RFID 已确认 TAG 被编辑后退出单删上下文
   - 问题：
     - TAG 焦点输入正确卡号并确认后，会反查显示 UNIT 和 SAVE。
     - 此时焦点仍停在 TAG，如果按 `*` 删除 TAG 里的卡号，旧 UNIT 和 SAVE 仍显示，用户会感觉仍处于单删卡模式。
   - 修复：
     - 新增 `card_manage_cancel_confirmed_tag_context()`。
     - 已确认 TAG 被继续输入或退格前，先取消本次 TAG 确认状态。
     - 取消时清除删除用 TAG 缓存、旧 UNIT 输入、旧 SAVE 数量和房号缓存，并刷新 `ERASE` 文案。
     - 保留用户正在编辑的 TAG 输入本身，不直接清空 TAG。
   - 验证：
     - `bash tests/test_card_prompt_interaction.sh` 通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；沙箱内 `mkfs.jffs2` 仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=$(date +%Y%m%d%H%M%S) && ./partition_image.sh app_resource` 通过。

31. [complete] RFID TAG 确认后返回键行为修正
   - 问题：
     - `TAG` 焦点输入或刷卡填充正确卡号后按确认，会回填 `UNIT` 和 `SAVE`。
     - 此时焦点仍在 `TAG`，按 `*` 应与确认 `UNIT` 后按返回一样，退出本次操作并回 RFID 主界面。
     - 旧路径仍处于 `CARD_MANAGE_MAIN_LAYER`，`card_manage_key_star_up()` 会先命中 TAG 退格分支，把 `0015153840` 删除成 `001515383`。
   - 修复计划：
     - `app_cu_datin/system/layout/layout_card_manage.c`：TAG 确认成功后切入 `CARD_MANAGE_MAIN_LAYER_CONFIRM`。
     - `app_cu_datin/system/layout/layout_card_manage.c`：TAG 确认成功后刷新 `SwipingCard.tag_fill_request`，避免继续处于主层 TAG 刷卡输入态。
     - `tests/test_card_prompt_interaction.sh`：新增静态回归，锁定 TAG 确认成功后必须进入确认层并刷新 TAG 请求状态。
   - 已完成验证：
     - `bash tests/test_card_prompt_interaction.sh` 先 RED 后通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c tests/test_card_prompt_interaction.sh task_plan.md findings.md progress.md` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；沙箱内 `mkfs.jffs2` 仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=$(date +%Y%m%d%H%M%S) && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `598127 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 598016`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

32. [complete] RFID ERASE 三态文案调整
   - 目标：
     - RFID 默认状态显示 `ERASE`。
     - 确认 `UNIT` 后进入房间操作/整户删除语义时显示 `ERASE ALL`。
     - 确认 `TAG` 后进入单张删卡语义时显示 `ERASE TAG`。
   - 修复计划：
     - `app_cu_datin/system/layout/language.h`：将旧 `STR_CARD_MANAGE_ERASE_ROOM` 改为 `STR_CARD_MANAGE_ERASE_ALL`。
     - `app_cu_datin/system/layout/language.c`：默认文案去掉冒号，旧 `ERASE ROOM` 改为 `ERASE ALL`。
     - `app_cu_datin/system/layout/layout_card_manage.c`：`Erase_font_display()` 改为默认、房间操作、TAG 确认三态选择。
     - `tests/test_card_prompt_interaction.sh`：更新静态回归，禁止旧 `ERASE ROOM`。
   - 已完成验证：
     - `bash tests/test_card_prompt_interaction.sh` 先 RED 后通过。
     - `bash tests/test_home_id_set_focus_replace.sh` 通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/language.h app_cu_datin/system/layout/language.c app_cu_datin/system/layout/layout_card_manage.c tests/test_card_prompt_interaction.sh task_plan.md findings.md progress.md` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；沙箱内 `mkfs.jffs2` 仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=$(date +%Y%m%d%H%M%S) && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `598127 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 598016`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

33. [complete] README 补充 2026-06-05 RFID 修改记录
   - 用户要求：
     - 遵守 Superpowers 和 context-engineering-marketplace 的文件化上下文约定，将本次 RFID 修改按 README 既有格式写入 `README.md`。
   - 已补充：
     - `2026-06-05（RFID TAG 单删状态返回和 ERASE 三态文案）`。
   - 覆盖范围：
     - TAG 确认后按 `*` 与 UNIT 确认后返回行为一致，不再删除 TAG 数字。
     - 已确认 TAG 被继续编辑或退格时退出单删确认上下文，清理旧 UNIT/SAVE。
     - `ERASE` 文案三态显示：默认 `ERASE`，房间操作 `ERASE ALL`，单张删卡 `ERASE TAG`。
   - 验证：
     - `git diff --check -- README.md task_plan.md progress.md findings.md` 通过。
     - `git diff -- app_cu_datin/autobuild.sh AK37E_SDK_V1.03/upgrade/make_image.sh` 无输出，确认未改编译脚本。

34. [complete] 大楼机通话结束后功放延时恢复
   - 目标：
     - 大楼机通话/监控结束时，在关闭摄像头和咪头的同时关闭功放。
     - 关闭功放后等待 2 秒，再由软件重新打开功放。
   - 修复计划：
     - `app_cu_datin/system/src/intercom.h`：新增 `INTERCOM_AMPLIFIER_REOPEN_DELAY_MS 2000`。
     - `app_cu_datin/system/src/intercom.c`：新增功放延时恢复状态、关闭输出 helper 和周期恢复检查。
     - `app_cu_datin/system/src/intercom.c`：通话开始时取消上一次 pending，避免新通话被延时恢复状态影响。
     - `tests/test_intercom_amplifier_recovery.sh`：新增静态回归，锁定关闭摄像头/咪头/功放和 2 秒恢复逻辑。
   - 验证计划：
     - `bash tests/test_intercom_amplifier_recovery.sh` 先 RED 后通过。
     - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
     - `bash tests/test_brancher_heartbeat.sh` 通过。
     - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/src/intercom.c app_cu_datin/system/src/intercom.h tests/test_intercom_amplifier_recovery.sh task_plan.md findings.md progress.md docs/superpowers/specs/2026-06-09-intercom-amplifier-delayed-reopen-design.md docs/superpowers/plans/2026-06-09-intercom-amplifier-delayed-reopen.md` 通过。
     - `cd app_cu_datin && make` 通过。
     - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；沙箱内 `mkfs.jffs2` 仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
     - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
     - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=$(date +%Y%m%d%H%M%S) && ./partition_image.sh app_resource` 通过。
     - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `598127 bytes`。
     - 包头：`# File Parttion: app.sqsh4 0 598016`。
     - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

35. [complete] README 补充波斯语文本修正和功放延时恢复记录
   - 用户要求：
     - 将本次“大楼机通话结束后功放延时恢复”修改写入 `README.md`。
     - 将上次“波斯语设置界面 `کارت دسترسی` 显示异常”和“密码错误三次等待提示波斯语显示异常”一并写入 `README.md`。
   - 已补充：
     - `2026-06-09（波斯语设置页和密码等待提示显示修正）`。
     - `2026-06-09（大楼机通话结束后功放延时恢复）`。
   - 覆盖范围：
     - 设置页波斯语卡管理文本独立坐标、宽度、字号、右对齐和避免覆盖焦点箭头。
     - 密码三次错误等待提示新增多语言字符串，波斯语使用独立宽显示区域。
     - 通话/监控结束时关闭摄像头、咪头和功放，2 秒后事件循环恢复功放。
   - 验证：
     - `git diff --check -- README.md task_plan.md progress.md` 通过。
     - `git diff -- app_cu_datin/autobuild.sh AK37E_SDK_V1.03/upgrade/make_image.sh` 无输出，确认未改编译脚本。
     - `rg -n "波斯语设置页和密码等待提示显示修正|大楼机通话结束后功放延时恢复" README.md` 确认两个新增标题存在。
