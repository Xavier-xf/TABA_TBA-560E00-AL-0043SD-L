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
