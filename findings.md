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

## 卡管理弹窗和底部 Logo 显示问题
- 英文底部 Logo 中 `TABA` 的文本框从 `112` 缩到 `86` 后，36 号字体下最后一个 `A` 容易被裁剪；应保留足够宽度，再通过移动起点收紧与 `Electronics` 的视觉间距。
- 弹窗使用 `draw_rect()` 会按 ARGB 混合绘制，如果底层已有文字，视觉上仍可能显得“底下有字”；提示弹窗应使用 `gui_erase()` 以不透明颜色直接覆盖。
- 弹窗关闭时只擦弹窗局部区域不够稳妥，因为弹窗覆盖了卡管理页面多个文本行；关闭后应清理上半页并按当前页面状态重绘字体、输入框、焦点和当前卡号信息。
- 卡管理页右侧输入/输出列应统一使用 `x = 120`，避免 UNIT/TAG/ERASE/SAVE 对齐不一致。
- 保存/删除成功提示关闭后，不能继续按旧的 `room_card_string_buf_display()` 和 `room_card_numbe_display()` 重画结果值；否则会把 `TAG:` 后的卡号和 `SAVE:` 后的数量再次画回来。
- 保存/删除已经将 `SwipingCard.mode` 置为 `CARD_IDLE_MODE`，所以提示关闭后需要按“非添卡模式”清理；同时要清掉 `SwipingCard.success_show`，否则定时器仍可能再走一次刷卡成功重画，把旧 TAG/SAVE 值补回来。
- 当前 UI 基础库只有矩形绘制/擦除，没有圆角矩形 API；提示框可以按要求调整位置、尺寸、颜色和不透明度，但 `border-radius: 6px` 需要新增底层绘制能力或使用圆角图片资源。

## RFID 提示框资源替换
- 用户新增 `app_cu_datin/system/ui/r/img/card_maneage/rfid_focus.png` 作为 RFID 消息框背景，`rom.h` 已生成 `ROM_R_IMG_CARD_MANEAGE_RFID_FOCUS_PNG`，资源尺寸为 `236x66`。
- 旧方案使用 `gui_erase(..., 0xFF5D7798)` 画纯色提示框；新方案应直接绘制 `rfid_focus.png`，避免继续维护纯色矩形背景。
- 提示框最终参考用户指定位置调整为 `left=152, top=73`，结合资源尺寸，代码显示区域使用 `{{152, 73}, {236, 66}}`。
- 卡管理标签从 `x=33` 开始、宽 `120`，显示区域到 `x=153`；旧右侧列 `x=120` 会压进 `ERASE:` 标签区域，因此右侧输入/输出列统一移动到 `x=160`。
- 提示文字颜色按结果区分：错误信息使用红色 `0xFFFF0000`，成功信息使用白色 `0xFFFFFFFF`。
- `tests/test_card_prompt_interaction.sh` 原先混入底部 Logo 位置检查，本轮将其从卡片提示测试移除；Logo 仍由 `tests/test_bottom_logo_language.sh` 独立覆盖，避免无关断言影响 RFID 提示框验证。

## 提示框后待机死机和删卡越界风险
- `analog_clock_deinit()` 在 `clock_dot_buffer != NULL` 分支里释放了 `analog_clock_dst_buffer`，导致 `clock_dot_buffer` 泄漏，并可能让 `analog_clock_dst_buffer` 被错误释放路径处理；待机退出和重新进入时属于高风险内存错误。
- `analog_clock_dst_buffer` 作为指针旋转/合成的中间缓冲，分配后如果不清零，复用到脏内存时可能放大残影或异常颜色。
- `font_decodec()` 的临时灰度缓冲分配后未清零，FreeType 字形只写入实际覆盖区域；未覆盖区域的旧数据可能参与后续绘制，表现为红字/白字边缘有重叠阴影或脏像素。
- 删卡线程参数实际是卡片数据起始编号 `home_id * 10`，不是 `UserData.unit_number[]` 的数组下标；旧代码 `UserData.unit_number[(int)arg] = -1` 会把卡号索引当房号列表下标，典型房号如 `1001` 会写到 `10010`，超过 `unit_number[10000]`。
- 正确删除房号的方式应按 `home_id` 在 `UserData.unit_number[]` 当前有效范围内查找，找到后前移后续项并将 `UNIT_NUMBER_INDEX` 减 1；如果找不到，不应盲目减少计数。
- 当前 C 源多数是 CRLF 文件，普通 `git diff --check` 会把行尾 CR 报为 trailing whitespace；本轮使用 `git -c core.whitespace=cr-at-eol diff --check` 检查真实空白问题。

## UNIT 房号设置页输入替换
- `layout_home_id_set.c` 中 M1-M4 焦点切换原先在 `home_id_set_key_up_up()`、`home_id_set_key_down_up()`、`home_id_set_key_ring_up()` 里调用 `clear_current_show_home_id()`，因此用户只是上下选择到某个房号框时，旧房号会立即被清掉。
- 用户期望是“选择不清空，输入时替换”：例如框内已有 `1`，上下选择到该框仍显示 `1`；开始输入 `22` 时先清空旧值，再显示 `2 2`。
- 修复策略使用每个输入框独立的 `home_id_replace_on_next_input[]` 标志。读房号刷新或焦点切换到已有内容的输入框时置位；`home_id_set_add_number()` 第一次数字输入前按标志清空旧值，并立即清标志，保证连续输入 `22` 不会第二个数字再次清空第一个数字。
- 同时修正 `home_id_set_add_number()` 的边界顺序：旧代码在判断 `index <= max_index` 前先写 `show_id[index]`，满 4 位后继续按数字有越界写风险；现在先判断再写。

## UNIT 房号设置页重复保存
- 读取旧房号后，M1-M4 输入框里有显示内容但并不代表用户本次编辑过。旧逻辑在上/下/OK 切换焦点前无条件调用 `set_home_id_number()`，因此第二次移动经过已有房号时也会走保存和重复校验，表现为无操作也可能变黄。
- 正确语义应区分“已有显示值”和“本次输入值”：只有用户按数字键后，当前输入框才具备保存资格。
- 修复策略使用每个输入框独立的 `home_id_input_dirty[]` 标志。数字输入成功写入后置 `true`；焦点切换时通过 `home_id_set_save_dirty_current()` 判断，未 dirty 直接移动，dirty 才调用保存逻辑。
- `set_home_id_number()` 改为返回是否真正发起 `Intercom.set_id()`，避免依赖可能残留的 `Intercom.status` 判断；只有真实发起保存后才清除 dirty。保存失败或弹出已存在确认框时不移动焦点，保留输入状态。
