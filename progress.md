# 进度记录

## 2026-05-21
- 用户澄清：需要的是“当前未修复工程”的复现升级包，用来验证是否会出现死机问题，不需要修复代码。
- 已读取并使用 `planning-with-files-zh`、`test-driven-development`、`verification-before-completion`。
- 已检查工作区状态：存在大量既有修改和生成产物，后续只处理本任务相关源码及构建产物。
- 已创建 `task_plan.md`、`findings.md`、`progress.md`。
- 已将此前误改的 4 个源码文件恢复到 HEAD 内容。
- 静态确认风险循环存在：`rg "for \\(unsigned char .*get_int_conf\\(" app_cu_datin/system` 命中 5 处。
- 下一步重新编译并制作“未修复风险版本”的升级包。
- 已重新运行 `app_cu_datin/autobuild.sh -all-sdk` 构建未修复风险版本应用。
- `mkfs.jffs2` 在沙箱内再次报“错误的系统调用”，已按记录提权重建 `config.jffs2/data.jffs2/tuya.jffs2` 并重新拼包。
- 最终复现升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 7,793,977 字节，包头版本 `20260521135825`。
- 验证源码无修复 diff：4 个风险源码文件 `git diff` 为空。

## 遇到的错误
| 错误 | 尝试次数 | 处理 |
|------|---------|------|
| 误制作了修复包 | 1 | 已恢复源码，重新制作未修复复现包 |
| `mkfs.jffs2` 错误的系统调用 | 1 | 后续如再次出现，需要提权重新生成 jffs2 分区镜像，然后重新执行 `partition_image.sh all` |

## DATA 预置测试包
- 用户需要升级后直接测试，但没有 255 张卡。
- 方案：制作只包含 DATA 分区的升级包，预置 255 个不同房号，卡数据置空。
- 这样升级后进入绑卡页面，录入第 256 个不同房号并刷一张真实卡即可触发风险路径。
- 已完成 DATA-only 升级包：
  - `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 1,373,129 字节，版本 `20260521140832`。
  - 包头只包含 `data.jffs2 0 1373016`，不包含 app/config/tuya/kernel/rootfs 等分区。
  - `AK37E_SDK_V1.03/rootfs/rootfs/app/data/data.ini` 已设置 `index3 = 255`。
  - `user_data` 的房号数组预置为 `1001..1255`，第 256 个位置为 `-1`。
  - `card_data` 为空，测试时只需要一张真实卡绑定房号 `1256`。
- 验证：风险源码 4 个文件无修复 diff；`rg "for \\(unsigned char .*get_int_conf\\(" app_cu_datin/system` 仍命中 5 处风险循环。

## 修复验证包
- 用户已在整机复现：DATA 预置后输入 `1257`，`data.ini` 中 `index3` 变为 `256`，随后设备卡死。
- 当前目标：修复软件死循环问题，同时继续把 DATA 预置为 `index3 = 255`，制作可直接验证修复效果的升级包。
- 已修复 5 处 `unsigned char` + `get_int_conf(...)` 循环，将循环下标改为 `int`：
  - `app_cu_datin/system/src/swiping_card.c`
  - `app_cu_datin/system/src/user_data.c`
  - `app_cu_datin/system/layout/layout_logo.c`
  - `app_cu_datin/system/layout/layout_home_id_set.c`
- 已重新构建 APP，重建 DATA 预置，并制作 APP+DATA 验证包：
  - `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`
  - 大小 1,958,899 字节，版本 `20260521143852`
  - 包头只包含 `app.sqsh4` 和 `data.jffs2`
- 验证：
  - `rg "for \\(unsigned char .*get_int_conf\\(" app_cu_datin/system` 无命中。
  - `data.ini` 为 `index3 = 255`。
  - `user_data` 房号数组为 `1001..1255`，下一项 `-1`。
- 用户要求版本号 +1 便于确认升级成功。
- 已将 `app_cu_datin/system/layout/layout_base.h` 中 `VERSION_NUMBER` 从 `v2.1.0` 改为 `v2.1.1`，界面显示版本为 `v2.1.1_dev`。
- 已重新编译 APP、重建 `app.sqsh4`，并重新制作 APP+DATA 包：
  - `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`
  - 大小 1,958,899 字节，版本 `20260521145338`
  - 包头仍只包含 `app.sqsh4` 和 `data.jffs2`
  - `data.ini` 仍为 `index3 = 255`
- 用户确认修复后暂未发现问题，要求恢复 DATA 测试镜像并制作正式 APP 升级包。
- 已将 `AK37E_SDK_V1.03/upgrade/platform/data.jffs2` 恢复为 HEAD 原始镜像，大小 244 字节。
- 已按 `README.md` 既有格式补充 2026-05-21 死机问题修改日志。
- 已制作正式 APP-only 升级包：
  - `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`
  - 大小 585,839 字节，版本 `20260521152631`
  - 包头只包含 `app.sqsh4`
  - APP 内版本字符串为 `v2.1.1_dev`
- 用户指出测试用 `/app/data` 预置文件也应恢复。
- 已删除 `AK37E_SDK_V1.03/rootfs/rootfs/app/data/data.ini`、`user_data`、`card_data` 三个测试文件；该目录仅剩 `rings/`、`photo/` 目录。

## 卡管理房号错误提示修复
- 用户选择方案A，要求修复管理房号中 `Room number error` 与新输入房号重叠，以及退出后再次进入不再提示的问题。
- 已打开并使用 `using-superpowers`、`systematic-debugging`、`planning-with-files-zh`、`test-driven-development`。
- 根因：错误状态期间数字输入未被拦截；退出界面未重置 `card_number_status_count`，导致下次进入可能错过错误提示显示点。
- 已修改 `app_cu_datin/system/layout/layout_card_manage.c`：
  - `card_manage_input_add_number()` 在非 `CARD_MANAGE_STATUS_NONE` 时直接返回。
  - `card_manage_input_sub_number()` 在非 `CARD_MANAGE_STATUS_NONE` 时直接返回。
  - `layout_card_manage_enter()` 和 `layout_card_manage_quit()` 清零 `CardManageClass.room_card_info.card_number_status_count`。
- 验证：
  - `git diff --check -- app_cu_datin/system/layout/layout_card_manage.c task_plan.md findings.md progress.md` 通过。
  - `git diff --ignore-space-at-eol --stat -- app_cu_datin/system/layout/layout_card_manage.c` 显示实质为 10 行新增。
  - `app_cu_datin/autobuild.sh -all-sdk` 返回成功，APP 编译和 SDK 拷贝完成；过程中 `mkfs.jffs2` 在沙箱内仍出现已知“错误的系统调用”，但本次 APP 侧修复编译已通过。
- 发现自动构建后的 `HALL_MACHINEOS` 只有 16,384 bytes，包头未包含 app 分区；原因是交互式 `partition_image.sh` 未选中 APP。
- 已使用 `partition_image.sh app_resource` 重新生成 APP-only 升级包：
  - `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`：585,839 bytes。
  - 包头：`# File Partion: app.sqsh4 0 585728`。
  - 版本：`20260521165030`。

## 底部 TABA logo 语言切换
- 用户确认方案A：只改底部 `taba_icon.png` 对应显示逻辑，保留纯图片，程序叠加语言文字。
- 已启用并读取 `brainstorming`、`writing-plans`、`test-driven-development`、`systematic-debugging`、`executing-plans`、`subagent-driven-development`、`requesting-code-review`、`verification-before-completion`。
- 已创建实施计划：`docs/superpowers/plans/2026-05-22-bottom-logo-language.md`。
- 已创建 RED 静态回归测试：`tests/test_bottom_logo_language.sh`。
- 已根据代码审查补强测试：校验 `app_cu_datin/system/ui/r/img/taba_icon.png` 与根目录 `logonew.png` 完全一致，覆盖“底图为新纯图片”要求。
- 已修改：
  - `app_cu_datin/system/layout/language.h`：增加 `STR_LOGO_TABA`、`STR_LOGO_ELECTRONICS`。
  - `app_cu_datin/system/layout/language.c`：增加英文/波斯语 logo 文本。
  - `app_cu_datin/system/layout/layout_base.c`：`taba_btn_display()` 先显示纯图片，再叠加大号 `TABA` 和小号 `Electronics` 文本。
- 验证：
  - `sh tests/test_bottom_logo_language.sh` 通过。
  - `app_cu_datin/autobuild.sh -all-sdk` 返回成功；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，但 APP 编译和 `app.sqsh4` 生成完成。
  - 已用 `partition_image.sh app_resource` 生成 APP-only 升级包。
  - `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`：577,647 bytes。
  - 包头：`# File Parttion: app.sqsh4 0 577536`。

## 底部 TABA logo 位置和 RTL 调整
- 用户整机验证后反馈：文字整体偏上；波斯语 `تابا` 和 `Electronic` 间距偏大；波斯语应按从右到左排布。
- 调整方案：英语维持左 TABA、右 Electronics；波斯语使用单独坐标，把 `تابا` 放右侧、`الکترونیک` 放左侧，并整体下移。
- 已先更新静态测试，要求新坐标和 `language_persian` 分支。
- 已修改 `app_cu_datin/system/layout/layout_base.c`：
  - 英语坐标：`TABA` 使用 `{{120, 219}, {112, 44}}`，`Electronics` 使用 `{{228, 226}, {150, 30}}`。
  - 波斯语坐标：`تابا` 使用 `{{248, 219}, {86, 44}}`，`الکترونیک` 使用 `{{128, 226}, {126, 30}}`。
- 验证：
  - `sh tests/test_bottom_logo_language.sh` 通过。
  - `git diff --check -- app_cu_datin/system/layout/layout_base.c tests/test_bottom_logo_language.sh task_plan.md progress.md` 通过。
  - `app_cu_datin/autobuild.sh -all-sdk` 返回成功；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，但 APP 编译和 `app.sqsh4` 生成完成。
- 已用 `partition_image.sh app_resource` 生成 APP-only 升级包。
- `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`：577,647 bytes。
- 包头：`# File Parttion: app.sqsh4 0 577536`，版本 `20260522113838`。

## 语言切换即时重绘和 calling 页面残点
- 用户确认继续按方案A执行。
- 已写入设计文档：`docs/superpowers/specs/2026-05-22-logo-refresh-and-calling-cleanup-design.md`。
- 已写入实施计划：`docs/superpowers/plans/2026-05-22-logo-refresh-and-calling-cleanup.md`。
- 已扩展静态测试，新增三类断言：
  - 波斯语 logo 更紧凑的新坐标；
  - 语言切换分支内存在底部区域清除和 `taba_btn_display()` 重绘；
  - `layout_calling.c` 存在统一的 calling 文本区域清除函数。
- 已修改：
  - `app_cu_datin/system/layout/layout_base.c`：波斯语底部 logo 坐标改为更紧凑的排布。
  - `app_cu_datin/system/layout/layout_system_set.c`：语言切换后立即清除底部区域并重绘 logo。
  - `app_cu_datin/system/layout/layout_calling.c`：新增统一文本清理区域，呼叫中/呼叫警卫/无应答三条路径均先清理再绘制。
- 验证：
  - `sh tests/test_bottom_logo_language.sh` 通过。
  - `git diff --check -- app_cu_datin/system/layout/layout_base.c app_cu_datin/system/layout/layout_system_set.c app_cu_datin/system/layout/layout_calling.c tests/test_bottom_logo_language.sh docs/superpowers/specs/2026-05-22-logo-refresh-and-calling-cleanup-design.md docs/superpowers/plans/2026-05-22-logo-refresh-and-calling-cleanup.md task_plan.md progress.md` 通过。
  - `app_cu_datin/autobuild.sh -all-sdk` 返回成功；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，但 APP 编译和 `app.sqsh4` 生成完成。
  - 已用 `partition_image.sh app_resource` 生成 APP-only 升级包。
  - `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`：577,647 bytes。
  - 包头：`# File Parttion: app.sqsh4 0 577536`，版本 `20260522115734`。
- 已根据代码审查补强测试：
  - 不再依赖未跟踪的根目录 `logonew.png`，改为校验 `taba_icon.png` 的固定 SHA256。
  - 明确校验 `system_set_logo_refresh()` 的定义和调用。
  - 明确校验 `calling` 页英语房号路径和 `No Answer` 路径存在统一清理区域覆盖。

## 大楼机广播心跳和分支器断线 LED 闪烁
- 用户确认方案A：主站广播心跳，分支器只接收不回复。
- 已启用并读取 `using-superpowers`、`writing-plans`、`test-driven-development`、`verification-before-completion`、`requesting-code-review`；本轮未启动子智能体，因为当前环境要求只有用户明确要求并行代理时才可派发。
- 已补写设计文档：`docs/superpowers/specs/2026-05-25-brancher-heartbeat-led-design.md`。
- 已补写实施计划：`docs/superpowers/plans/2026-05-25-brancher-heartbeat-led.md`。
- 已新增静态回归测试：`tests/test_brancher_heartbeat.sh`。
- 已修改：
  - `app_cu_datin/system/src/intercom.h`：新增 `CMD_HEARTBEAT 0xBA` 和 `INTERCOM_HEARTBEAT_INTERVAL_MS 1000`。
  - `app_cu_datin/system/src/intercom.c`：新增 `intercom_heartbeat_check()`，在 `intercom_event_detect()` 中周期广播心跳。
  - `switch/code/include/kevin_function.h`：同步新增 `CMD_HEARTBEAT 0xBA`。
  - `switch/code/msg_event.c`：收到心跳只刷新在线时间；5 秒无心跳后用 `cpu_count` 驱动 `POWER_LED` 每 500ms 闪烁；收到心跳后恢复常亮。
- 验证：
  - `bash tests/test_brancher_heartbeat.sh` 通过。
  - `app_cu_datin/autobuild.sh -all-sdk` 返回成功；期间 `mkfs.jffs2` 仍有沙箱内已知“错误的系统调用”，但 APP 编译和 `app.sqsh4` 生成完成。
  - 已用 `partition_image.sh app_resource` 生成 APP-only 升级包。
  - `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`：577,647 bytes。
  - 包头：`# File Parttion: app.sqsh4 0 577536`，版本 `20260525155823`。
- 注意：该功能需要大楼机 APP 升级和分支器 MCU 固件烧录两端同时具备；当前仓库分支器侧只有 Keil 工程，未在本机完成 MCU 命令行编译。

## 心跳避让业务通信验证读房号问题
- 用户实测：无心跳版本连续几十次 Output 读分支器 0 房号不复现，带心跳版本约第八次左右可能出现房号不显示。
- 当前目标：先做短期验证方案，让心跳在业务通信期间暂停或延后，确认读房号问题是否消失。
- 已启用并读取 `using-superpowers`、`systematic-debugging`、`test-driven-development`、`context-fundamentals`；上下文管理按最小高信号内容加载，未展开全部 context-engineering 技能正文。
- 已新增实施计划：`docs/superpowers/plans/2026-05-26-intercom-heartbeat-deferral.md`。
- 已新增静态回归测试：`tests/test_intercom_heartbeat_deferral.sh`。
- 已修改：
  - `app_cu_datin/system/src/intercom.h`：新增 `INTERCOM_HEARTBEAT_DEFER_MS 500`。
  - `app_cu_datin/system/src/intercom.c`：新增业务总线活动时间戳；公共 `send_cmd` 改为 `intercom_can_send_cmd()` 包装函数；所有非心跳业务发送和有效接收刷新活动时间；心跳检查在活动窗口内延后发送。
  - `tests/test_brancher_heartbeat.sh`：适配心跳现在直接调用底层发送函数，避免心跳本身被当成业务活动。
- 已完成静态验证：
  - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
  - `bash tests/test_brancher_heartbeat.sh` 通过。
- 待完成：APP 编译、APP-only 升级包制作和包头验证。

## 心跳避让业务通信正式实现
- 用户确认将短期验证改为正式实现：业务通信期间不发心跳；业务结束后延迟恢复；分支器任意有效主站命令都刷新在线；增加 busy 状态超时保护。
- 已补写设计文档：`docs/superpowers/specs/2026-05-26-intercom-heartbeat-busy-design.md`。
- 已补写实施计划：`docs/superpowers/plans/2026-05-26-intercom-heartbeat-busy.md`。
- 已按 TDD 先收紧静态测试：
  - `tests/test_intercom_heartbeat_deferral.sh` 新增 busy 超时释放、收帧优先于心跳检查等断言。
  - `tests/test_brancher_heartbeat.sh` 新增“任意有效主站命令刷新在线状态”断言。
- 已完成代码修改：
  - `app_cu_datin/system/src/intercom.h`：新增 `INTERCOM_BUS_BUSY_TIMEOUT_MS 4000`。
  - `app_cu_datin/system/src/intercom.c`：引入 `g_intercom_bus_busy`、busy 进入/释放、超时兜底；`intercom_event_detect()` 改为先收帧再做心跳检查。
  - `switch/code/msg_event.c`：新增 `intercom_refresh_link_online()`，成功解析任意有效主站命令后统一刷新在线状态。
- 已完成静态验证：
  - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
  - `bash tests/test_brancher_heartbeat.sh` 通过。
- 代码审查补正：
  - 初版实现被独立审查指出“仍偏向最近 500ms 活动窗口，不是真正事务 busy”。
  - 已将 APP 侧调整为事务型 busy：业务开始进入 busy，显式结束后再走 `500ms` 恢复窗口，`4000ms` 仅作为异常卡住兜底。
  - 已同步修正静态测试，明确禁止 defer 期间重置心跳时间戳。
- 已完成编译与打包：
  - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；沙箱内 `mkfs.jffs2` 仍有已知“错误的系统调用”，但 APP 编译和 `app.sqsh4` 生成成功。
  - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=$(date +%Y%m%d%H%M%S) && ./partition_image.sh app_resource` 通过。
  - APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`
  - 包头：`# File Parttion: app.sqsh4 0 581632`
  - 包大小：`581,743 bytes`

## 2026-05-29 卡管理弹窗和 Logo 修正
- 用户指出本轮应继续使用文件式 planning，并补写 `findings.md` / `progress.md`；当前环境没有可用的 `planning-with-files` skill，但继续维护 `task_plan.md`、`findings.md`、`progress.md`。
- 已恢复上一轮误改的编译脚本：
  - `app_cu_datin/autobuild.sh` 无 diff。
  - `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。
  - 删除上一轮为脚本改动新增的 `tests/test_autobuild_app_resource_packaging.sh`。
- 本轮约束：不要通过改脚本实现 app-only；需要只打 APP 分区时，按既有流程在 SDK `upgrade` 目录执行 `partition_image.sh app_resource`。
- 已定位 UI 问题：
  - `TABA` 文本框过窄导致最后 `A` 裁剪。
  - 弹窗颜色与背景接近且使用混合绘制，不够像独立对话框。
  - 关闭弹窗只擦局部，容易留下输入框/卡号/标签残影。
  - `ERASE` 行右侧列 x 坐标与其他行不一致。
- 已更新静态测试：
  - `tests/test_bottom_logo_language.sh`
  - `tests/test_card_prompt_interaction.sh`
- 已修改界面实现：
  - 英文 `TABA` 使用足够宽的文本框，`Electronics` 保持更近但不压到 `TABA`。
  - 弹窗背景改为 `#5D7798`，并使用不透明 `gui_erase()` 覆盖底字。
  - 弹窗关闭后清理上半页并重绘页面内容。
  - 卡管理右侧列统一到 `x = 120`。
- 已完成验证：
  - `bash tests/test_card_prompt_interaction.sh` 通过。
  - `bash tests/test_bottom_logo_language.sh` 通过。
  - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
  - `bash tests/test_brancher_heartbeat.sh` 通过。
  - `cd app_cu_datin && make` 通过且无新增警告。
  - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；原始脚本仍会在沙箱内打印 `mkfs.jffs2` “错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成完成。
  - `cd AK37E_SDK_V1.03/upgrade && env upgrade_bin_version=20260529172629 ./partition_image.sh app_resource` 通过。
  - 最终 `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS` 为 569K，包头只包含 `# File Parttion: app.sqsh4 0 581632`。
  - `app_cu_datin/autobuild.sh`、`AK37E_SDK_V1.03/upgrade/make_image.sh`、`tests/test_autobuild_app_resource_packaging.sh` 无 diff。

## 2026-05-29 卡管理提示框缩小和残留清理
- 用户反馈：Logo 已正常；提示框太大且位置不好，建议 `left=115, top=59, width=250, height=75, opacity=1, border-radius=6px`；保存/删除提示框销毁后 `TAG:` 后的卡号和 `SAVE:` 后的数量仍残留。
- 已确认当前环境没有可用的 `planning-with-file-zh` skill；继续按该流程维护 `task_plan.md`、`findings.md`、`progress.md`。
- 已按 TDD 更新 `tests/test_card_prompt_interaction.sh`：
  - 校验卡管理页和房号页提示框坐标尺寸为 `{{115, 59}, {250, 75}}`。
  - 校验卡管理页存在结果字段清理函数。
- 已修改：
  - `layout_card_manage.c`：提示框改为 `250x75`、位置 `115,59`；保存/删除/成功提示关闭后清空 `TAG` 和 `SAVE` 结果字段，并清空 `SwipingCard.string_buf[10]`、房间卡数量缓存和 `SwipingCard.success_show`，避免旧值被定时重画。
  - `layout_card_number.c`：提示框同步改为 `250x75`、位置 `115,59`。
- 用户进一步确认：如果仍在 `CARD_ADD_CARD_MODE` 可以重画旧值；保存/删除后已经退出添卡模式，应清掉信息。已按 `SwipingCard.mode == CARD_ADD_CARD_MODE` 作为分支条件核对并补强。
- 说明：当前 GUI 库没有圆角矩形 API，本轮没有实现真实 `border-radius: 6px`；如必须圆角，需要新增底层圆角绘制或做一张圆角背景图片资源。
- 已完成验证：
  - `bash tests/test_card_prompt_interaction.sh` 通过。
  - `cd app_cu_datin && make` 通过。
  - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；原始脚本仍打印已知 `mkfs.jffs2` 沙箱错误。
  - `cd AK37E_SDK_V1.03/upgrade && env upgrade_bin_version=20260529174518 ./partition_image.sh app_resource` 通过。

## 2026-06-01 RFID 提示框资源替换
- 用户新增 `rfid_focus` 作为 RFID 消息框，要求不再使用原先纯色消息框；错误信息红色，成功信息白色。
- 用户反馈 `{{120, 52}, {241, 77}}` 偏上偏左，要求参考 `top=73px, left=152px`；当前 `rfid_focus.png` 尺寸为 `236x66`，代码显示区域调整为 `{{152, 73}, {236, 66}}`。
- 用户反馈 UNIT/ERASE/SAVE 输入内容压到 `ERASE:` 显示区域；已将 UNIT/TAG/ERASE/SAVE 右侧输入/输出列统一移动到 `x=160`。
- 已按 TDD 更新 `tests/test_card_prompt_interaction.sh`：
  - 校验卡管理页和删卡房号页使用 `ROM_R_IMG_CARD_MANEAGE_RFID_FOCUS_PNG`。
  - 校验错误颜色 `0xFFFF0000`、成功颜色 `0xFFFFFFFF`。
  - 禁止继续使用 `CARD_MANAGE_PROMPT_BG_COLOR` / `CARD_NUMBER_PROMPT_BG_COLOR` 纯色提示框。
- 已修改：
  - `app_cu_datin/system/layout/layout_card_manage.c`：提示框改为绘制 `rfid_focus.png`，错误提示红色，添卡/删卡/成功提示白色。
  - `app_cu_datin/system/layout/layout_card_number.c`：提示框改为绘制 `rfid_focus.png`，房号错误红色，删除成功白色。
  - `tests/test_card_prompt_interaction.sh`：移除无关的底部 Logo 位置断言，Logo 独立测试仍由 `tests/test_bottom_logo_language.sh` 覆盖。
- 已完成验证：
  - `bash tests/test_card_prompt_interaction.sh` 通过。
  - `git diff --check -- app_cu_datin/system/layout/layout_card_manage.c app_cu_datin/system/layout/layout_card_number.c tests/test_card_prompt_interaction.sh` 通过。
  - `cd app_cu_datin && make` 通过。

## 2026-06-02 待机时钟、字体缓冲和删卡越界修正
- 用户要求继续上次未完成的优化，并明确使用 Superpowers 和 context-engineering-marketplace；当前会话中 Superpowers 插件已启用，context-engineering-marketplace 未作为可调用插件暴露，本轮继续维护 `task_plan.md`、`findings.md`、`progress.md`。
- 已按 `systematic-debugging` 先复核根因和当前 diff：
  - `analog_clock_deinit()` 释放 `clock_dot_buffer` 时错放成 `analog_clock_dst_buffer`。
  - `font_decodec()` 分配临时缓冲后未清零，可能让旧像素参与字体绘制。
  - `layout_card_manage.c` 和 `layout_card_number.c` 的删卡线程参数是 `home_id * 10`，旧代码把它当作 `UserData.unit_number[]` 下标写入，存在越界风险。
- 已修改：
  - `app_cu_datin/ui_lib/analog_clock.c`：`analog_clock_dst_buffer` 分配后清零；`clock_dot_buffer` 分支释放自身；`analog_clock_dst_buffer` 单独释放并置空。
  - `app_cu_datin/ui_lib/font_decodec.c`：字体缓冲分配失败时返回失败；分配成功后 `memset` 清零。
  - `app_cu_datin/system/layout/layout_card_manage.c`：新增按 `home_id` 查找并移除 `UserData.unit_number[]` 的辅助函数；删卡前检查 `card_base` 范围；删除卡槽后只在找到房号时递减 `UNIT_NUMBER_INDEX`。
  - `app_cu_datin/system/layout/layout_card_number.c`：同步修正删卡房号页的房号列表删除逻辑和卡槽范围检查。
  - `app_cu_datin/system/layout/layout_card_manage.c`、`app_cu_datin/system/layout/layout_card_number.c`：提示框位置保持 `{{152, 73}}`，尺寸同步资源实际 `236x66`。
- 已完成静态验证：
  - `bash tests/test_card_prompt_interaction.sh` 通过。
  - `bash tests/test_bottom_logo_language.sh` 通过。
  - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
  - `bash tests/test_brancher_heartbeat.sh` 通过。
  - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_card_manage.c app_cu_datin/system/layout/layout_card_number.c app_cu_datin/ui_lib/analog_clock.c app_cu_datin/ui_lib/font_decodec.c` 通过。
- 已完成编译与打包：
  - `cd app_cu_datin && make` 通过。
  - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；期间 `mkfs.jffs2` 在沙箱内仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
  - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`，避免 `autobuild` 后平台目录残缺。
  - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260602144956 && ./partition_image.sh app_resource` 通过。
  - 最终 APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`。
  - 包头：`# File Parttion: app.sqsh4 0 593920`。
  - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

## 2026-06-03 UNIT 房号设置页输入替换
- 当前会话可用 skills 已包含 Superpowers 插件和 context-engineering 拆分 skills，已使用 `using-superpowers`、`brainstorming`、`test-driven-development`、`planning-with-files-zh`、`verification-before-completion`。
- 用户要求：UNIT 界面键盘上下选择对话框时不要清除对话框里的房间号，开始输入时再清除；例如原房号 `1`，上下选择到该框不清，输入 `22` 时直接替换为 `22`。
- 已定位到 `app_cu_datin/system/layout/layout_home_id_set.c`：
  - 原 `home_id_set_key_up_up()` / `home_id_set_key_down_up()` / `home_id_set_key_ring_up()` 在切换到新焦点后调用 `clear_current_show_home_id()`，导致选择焦点即清空旧房号。
  - `home_id_set_add_number()` 原先在判断最大长度前先写 `show_id[index]`，满 4 位后继续按数字存在越界写风险。
- 已按 TDD 新增 `tests/test_home_id_set_focus_replace.sh`：
  - RED：确认当前没有输入替换标志、焦点切换仍会清空。
  - GREEN：实现后校验焦点切换不再调用清空函数，切换后标记“下次输入替换”，数字输入前先走替换准备逻辑。
- 已修改 `layout_home_id_set.c`：
  - 新增 `home_id_replace_on_next_input[HOME_ID_SET_TOTAL_DIALOG_BOX]`。
  - 新增 `home_id_set_mark_replace_on_next_input()`：切换到已有内容的 M 输入框时，标记下次数字输入需要替换。
  - 新增 `home_id_set_prepare_input_replace()`：数字输入前如标志存在且当前文本非空，则清空旧值并清标志。
  - 上下键和确认键切换 M1-M4 后不再清空对话框内容。
  - 读房号刷新 M1-M4 后，对已有内容的输入框设置替换标志。
  - `home_id_set_add_number()` 改为先判断 `index <= max_index` 再写 `show_id[index]`。
- 已完成验证：
  - `bash tests/test_home_id_set_focus_replace.sh` 通过。
  - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
  - `bash tests/test_brancher_heartbeat.sh` 通过。
  - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_home_id_set.c tests/test_home_id_set_focus_replace.sh` 通过。
  - `cd app_cu_datin && make` 通过。
  - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；沙箱内 `mkfs.jffs2` 仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
  - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
  - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260603084103 && ./partition_image.sh app_resource` 通过。
  - APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`，包头 `# File Parttion: app.sqsh4 0 593920`。
  - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

## 2026-06-03 UNIT 房号设置页 dirty 保存
- 用户整机测试确认上一版“切换焦点不清空、输入时替换”没问题，但发现：如果某个位置已经输入/保存过房号，第二次上下移动经过该位置仍会触发保存，导致重复保存校验并变黄。
- 已按用户确认方案实现“本次输入后才保存”：
  - 新增 `home_id_input_dirty[HOME_ID_SET_TOTAL_DIALOG_BOX]`。
  - `home_id_set_add_number()` 在数字真正写入 `show_id` 后将当前输入框置 dirty。
  - 新增 `home_id_set_save_dirty_current()`，未 dirty 时直接允许焦点移动，不调用保存；dirty 时才调用保存。
  - `set_home_id_number()` 从 `void` 改为 `bool`，返回是否真正发起 `Intercom.set_id()`，避免依赖旧的 `Intercom.status`。
  - 上/下/OK 主界面按键不再直接调用 `set_home_id_number()`。
  - 保存失败或弹出已存在确认框时不切换焦点；真实发起保存或保存成功后清除 dirty，避免二次路过重复保存。
- 已扩展 `tests/test_home_id_set_focus_replace.sh`：
  - 校验 `home_id_input_dirty` 和 `home_id_set_save_dirty_current` 存在。
  - 校验数字输入会置 dirty。
  - 校验按键处理函数不能直接调用 `set_home_id_number()`。
  - 校验初始化和保存成功会清 dirty。
- 已完成验证：
  - `bash tests/test_home_id_set_focus_replace.sh` 通过。
  - `bash tests/test_intercom_heartbeat_deferral.sh` 通过。
  - `bash tests/test_brancher_heartbeat.sh` 通过。
  - `git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/layout/layout_home_id_set.c tests/test_home_id_set_focus_replace.sh` 通过。
  - `cd app_cu_datin && make` 通过。
  - `cd app_cu_datin && ./autobuild.sh -all-sdk` 通过；沙箱内 `mkfs.jffs2` 仍打印已知“错误的系统调用”，但 APP 编译、SDK 拷贝和 `app.sqsh4` 生成成功。
  - 已在沙箱外重建 `AK37E_SDK_V1.03/upgrade/platform/config.jffs2`、`data.jffs2`、`tuya.jffs2`。
  - `cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=20260603100302 && ./partition_image.sh app_resource` 通过。
  - APP-only 升级包：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`，大小 `594031 bytes`，包头 `# File Parttion: app.sqsh4 0 593920`。
  - `app_cu_datin/autobuild.sh` 和 `AK37E_SDK_V1.03/upgrade/make_image.sh` 无 diff。

## 2026-06-03 README 补充 UNIT 修改记录
- 用户要求将本次 UNIT 改动按 `README.md` 既有格式写入文档。
- 已在 `README.md` 新增 `2026-06-03（UNIT 房号设置页输入替换和 dirty 保存）`：
  - 记录焦点切换不清空、输入时替换旧房号。
  - 记录 `dirty` 保存策略，避免第二次移动经过旧房号时重复保存并变黄。
  - 记录涉及文件和具体修改点。
- 已验证：
  - `git diff --check -- README.md` 通过。
