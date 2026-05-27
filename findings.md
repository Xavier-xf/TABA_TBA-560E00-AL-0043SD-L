# 发现记录

## 根因
- `UNIT_NUMBER_INDEX` 和 `SET_HOME_ID_INDEX` 是 `int` 配置计数。
- 多处代码使用 `unsigned char` 作为循环变量，并以 `get_int_conf(...)` 返回值作为上限。
- 当计数达到 256 或更大时，`unsigned char` 从 255 自增后回到 0，循环无法结束，可能在绑定查重、开机打印或清空数据时卡死。

## 风险位置
- `app_cu_datin/system/src/swiping_card.c`：`unit_number_exist()`。
- `app_cu_datin/system/layout/layout_logo.c`：`printf_user_data()`。
- `app_cu_datin/system/src/user_data.c`：`deleteAllCard()`。
- `app_cu_datin/system/layout/layout_home_id_set.c`：`home_id_exist()`。

## 构建路径
- 应用构建脚本：`app_cu_datin/autobuild.sh`。
- 推荐命令：`cd app_cu_datin && ./autobuild.sh -all-sdk`。
- SDK 打包由脚本调用：`AK37E_SDK_V1.03/build.sh -r`。
- 升级包目标：`AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`。

## 卡管理房号错误提示问题
- `Room number error` 由 `layout_card_manage.c` 的 `CARD_MANAGE_STATUS_ERROR` 状态触发显示。
- 原输入函数只拦截 `CARD_MANAGE_MAIN_LAYER_CONFIRM`，没有拦截非 `CARD_MANAGE_STATUS_NONE` 状态，因此错误提示显示期间仍能输入房号，造成文本重叠。
- 错误提示显示计数使用 `CardManageClass.room_card_info.card_number_status_count`，退出卡管理界面时未清零，直接退出后再次进入可能错过 `== 3` 的显示触发点。
- 修复策略：状态提示期间禁止输入/删除，进入和退出卡管理界面时重置提示计数。

## 分支器通信断开 LED 提示
- 当前分支器没有独立的“主站在线/离线”状态机，只有业务命令 ACK 重试，不能覆盖总线空闲或线路中途断开的场景。
- 一台大楼机对应几十到上百个分支器时，不适合做逐个轮询或要求每个分支器回复心跳，否则总线流量会随分支器数量线性增长。
- 选定方案A：大楼机广播 `CMD_HEARTBEAT`，分支器只接收不回复。这样心跳流量固定，不受分支器数量影响。
- 分支器已有 `POWER_LED` 常亮初始化，旧 `led_blink()` 使用 `TIMER0`；但 `TIMER0` 已被开锁防抖占用并会被 `kill_timer(TIMER0)` 清掉，因此离线闪烁不能复用旧 `led_blink()`。
- 修复策略：新增 `CMD_HEARTBEAT = 0xBA`；大楼机每 1 秒发送心跳；分支器 5 秒未收到心跳后使用 `cpu_count` 驱动 `POWER_LED` 每 500ms 翻转；收到心跳后立即恢复常亮。

## Output 连续读房号偶发不显示
- 用户实测：升级没有心跳包的版本后，连续几十次进入 Output、输入分支器 0、读房号、退出再进入，未再出现房号不显示；带心跳版本约第八次左右可能出现。
- 该对比说明硬件 CAN/线路完全损坏的概率较低，心跳帧插入业务读房号链路更像触发条件。
- 当前读房号是链式通信：大楼机发送 `CMD_READ_HOME_ID HOME_ID1`，收到 `CMD_SEND_HOME_ID HOME_ID1` 后再发送 HOME_ID2/3/4；中间任意一次被心跳或半帧解析打断，后续显示就不会完整刷新。
- 短期验证策略：大楼机侧在任意业务发送或有效接收后的 `500ms` 内延后心跳发送，避免心跳穿插在读房号、设置房号、呼叫、监控、开锁等业务通信中。
- 该策略用于验证问题是否由心跳插帧放大；长期仍建议补充完整帧解析、读房号事务超时重试和退出页面取消读事务。

## 心跳避让业务通信正式化
- 短期验证已经证明：仅做“业务通信期间延后心跳”后，读房号问题明显不再复现，说明主因更接近协议时序竞争而不是纯硬件故障。
- 正式实现不应只依赖“最近活动时间戳”，还需要明确的 busy 状态和超时兜底；否则异常路径可能让心跳长期停发或恢复时机不稳定。
- 分支器在线判定不能只靠 `CMD_HEARTBEAT`，因为业务通信期间大楼机会暂停心跳；如果分支器不把其他有效主站命令也视为在线活动，长业务期会误判断线闪灯。
- 本次先不并入“读房号重读”，因为重读若没有“退出页面取消事务、忽略旧回复”的配套机制，用户在房号未返回时直接退出页面，迟到回复仍可能污染界面状态。
