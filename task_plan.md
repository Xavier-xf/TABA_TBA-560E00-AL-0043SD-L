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
