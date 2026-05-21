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
