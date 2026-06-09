# Intercom Amplifier Delayed Reopen Design

## 目标

大楼机通话或监控结束时，软件应在关闭摄像头和咪头的同时关闭功放，并在 2 秒后恢复打开功放，避免功放在通话结束后一直保持打开。

## 当前行为

- `intercom_monitor_start_process()` 在监控/通话开始时打开摄像头、功放和咪头。
- `monitor_status_check()` 在读监控状态 800ms 收尾和 2 分钟超时强制关闭时，会关闭摄像头和咪头。
- 这两个结束路径当前没有关闭功放。
- `amplifier_gpio_control(true)` 表示打开功放，`amplifier_gpio_control(false)` 表示关闭功放。

## 方案

新增一个通话输出关闭 helper，在所有通话结束路径统一关闭摄像头、咪头和功放：

- `camera_led_gpio_control(false)`
- `camera_power_gpio_control(false)`
- `mic_mute_gpio_control(false)`
- `amplifier_gpio_control(false)`

关闭后记录时间并设置功放延时恢复 pending。`intercom_event_detect()` 周期执行时检查 pending，超过 `INTERCOM_AMPLIFIER_REOPEN_DELAY_MS = 2000` 后调用 `amplifier_gpio_control(true)`，然后清除 pending。

如果 2 秒内又进入新的监控/通话，开始流程先取消 pending，再立即打开功放，避免上一轮延时任务影响新通话。

## 不采用的方案

- 不使用 `ak_sleep_ms(2000)` 阻塞等待，因为会阻塞 CAN 收包、心跳检查和界面事件。
- 不新增独立线程，因为当前 `intercom_event_detect()` 已经有周期检查机制，新增线程会增加同步和生命周期风险。

## 验收

- 静态测试能确认常量、关闭 helper、延时恢复和开始流程取消 pending 都存在。
- 旧心跳相关静态测试继续通过。
- `app_cu_datin` 编译通过。
- 不修改 `autobuild.sh` 或 SDK 打包脚本。
