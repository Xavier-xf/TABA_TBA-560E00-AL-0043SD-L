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

## RFID 保存/删卡提示关闭后焦点位置
- `card_manage_prompt_close()` 原先关闭提示时只清状态并重绘页面，`CardManageClass.cur_focus.main` 保持在触发操作的 `SAVE_FOCUS` 或 `ERASE_FOCUS`，所以提示关闭后箭头仍停在 `SAVE:` 或 `ERASE:`。
- 用户的操作路径是“输入房号进入添卡模式 -> 保存/删除 -> 看提示 -> 继续下一户”，下一步通常是重新输入 `UNIT:`，因此保存/删除成功后焦点回 `UNIT:` 更符合连续操作。
- 错误提示不应强制回 `UNIT:`，因为错误通常需要用户继续处理当前操作，强制改变焦点会增加定位成本。
- 焦点重置应先改 `cur_focus` 状态，再调用 `card_manage_page_redraw()`；如果先按旧焦点重绘页面再单独显示 UNIT 箭头，可能留下旧 `SAVE/ERASE` 箭头残影或双箭头。

## RFID TAG 单张删卡
- 当前 `card_data` 存储的是原始刷卡 ID 字符串，格式为 `ID:` + 十六进制数据；屏幕 `TAG:` 显示的是从该原始 ID 转换出的纯数字字符串，因此手动输入 TAG 必须按屏幕显示输入，保留前导 0，例如 `0014246460`。
- 单张删卡不需要改数据文件结构。删除时只需要把当前 UNIT 的 10 个卡槽逐个转换成显示 TAG，与输入 TAG 比较，匹配后清空对应的一个 slot 并保存 `card_data`。
- 旧房间卡数量统计在遇到第一个空 slot 时 `break`，单删会产生中间空洞，必须改为遍历 10 个 slot 统计全部非空卡，否则后续显示数量会偏小。
- 后续添卡不会因为中间空洞出错：`save_card_id()` 原本就是从 slot0 到 slot9 查找第一个空位，单删留下的空 slot 会被优先复用。
- `ERASE` 在 TAG 为空或当前 UNIT 下找不到该 TAG 时不能再整户删除；否则用户只是忘记输入 TAG 就可能删掉整户卡。当前策略是只提示错误。
- 刷卡填充 TAG 必须限定在当前 UNIT 内匹配。卡存在但属于其他 UNIT 时不填充、不删除，提示错误，避免跨户误删。

## RFID TAG 与 UNIT 同级交互
- 当前代码已经有 TAG 单删基础，但手动 TAG 输入、`*` 删除 TAG 和刷卡填充 TAG 都依赖 `CARD_MANAGE_MAIN_LAYER_CONFIRM`，等价于必须先确认 `UNIT:` 后才能操作 `TAG:`。
- 用户期望 `TAG:` 与 `UNIT:` 同级，因此 TAG 输入和刷卡填充应在 `CARD_MANAGE_MAIN_LAYER` 也生效，焦点停在 `TAG_FOCUS` 即可接收数字键和刷卡。
- 当前 `card_manage_tag_input_display()` 使用 `{{160, 71}, {280, 40}}`，右边界到 `x=440`；选择箭头位置为 `{{430, ...}, {32, 32}}`，因此 TAG 输入或清除区域会覆盖最右侧箭头。
- 推荐把 TAG 文本显示/清除区域收窄到右边界小于 `x=430`，例如 `{{160, 71}, {250, 40}}` 或更稳妥的 `{{160, 71}, {240, 40}}`。
- 为了满足“TAG 输完后在 TAG 项按确认，然后 ERASE 才能删”的语义，建议新增 `tag_confirmed` 状态：TAG 输入或删除后清 false；TAG 焦点按确认且 TAG 非空时置 true；ERASE 只有在 UNIT 有效、TAG 有值且 tag_confirmed 为 true 时执行单删。
- 不推荐在未输入 UNIT 时仅凭 TAG 全局删除，因为相同或异常转换 TAG 的误删风险更高，也会让用户无法确认删除归属户；推荐仍要求 UNIT 有效。
- 实现时对刷卡填充做了额外保护：`CARD_IDLE_MODE` 下如果 `SwipingCard.tag_fill_request` 为真，刷卡只填充 TAG，不进入普通开门验证；这样 TAG 主层刷卡不会误触发开门或非法卡报警流程。
- TAG 填充从“必须当前 UNIT 下找到该卡”放宽为“能从原始卡 ID 转换出屏幕 TAG 数字即可填充”，删除时仍限定当前 UNIT 匹配；这样支持先刷卡填 TAG、再输入 UNIT、确认 TAG、ERASE 删除。
- TAG 显示/清理区域右边界从 `x=440` 收到 `x=400`，给 `x=430,w=32` 的焦点箭头留出完整空间。

## RFID 删除路径和 TAG 提示
- 短房号显示重叠的直接原因是 `card_manage_prepare_unit_for_card_action()` 在删除前复用了 `card_manage_home_id_adjust()`，把输入 `9` 内部右对齐为 `0009` 后又调用 `card_manage_dialog_box_font_change()` 和 `widget_show.dialog_box()` 重画；原 `9` 区域未被可靠清干净，因此和第一个 `0` 重叠。
- 房号右对齐本身是旧逻辑，用于让输入 `9` 按房号 `0009/9` 计算，而不是 `9000`。正确修复不是取消内部右对齐，而是避免把内部补齐值重新画到输入框。
- RFID 页整户删除不可用的原因是单张删卡实现替换了旧 `delete_current_card()` 路径，只保留 `delete_single_card_by_tag()`；因此 TAG 为空时没有整户删除分支。
- 兼容策略调整为：`ERASE:` 时 TAG 为空表示整户删除；TAG 有值表示单张删除，且必须先在 `TAG:` 上确认。
- 错误文案必须按失败对象区分：UNIT 无效使用 `Room number error`，TAG 未确认或当前 UNIT 下找不到该 TAG 使用 `Card number error`。
- 旧方案曾考虑 TAG 确认后显示 `TAG confirmed` 弹窗；该方案已废弃，原因是会打断 RFID 主界面操作流，且用户反馈观感奇怪。

## RFID TAG 确认去弹窗和 ERASE 文案区分
- `TAG confirmed` 弹窗不适合作为确认反馈：它会进入提示框关闭流程，打断 RFID 主界面操作，并且容易让用户误以为出现异常提示。
- 更稳妥的反馈是让 `ERASE` 行承载当前动作语义：TAG 未确认或无 TAG 时显示 `ERASE ROOM`，TAG 确认并匹配后显示 `ERASE TAG`。
- TAG 确认流程需要复用 UNIT 确认的数据刷新，但不应先校验当前手动 UNIT；正确流程是先用 TAG 在全局已保存卡数据中反查卡槽，再用卡槽编号计算房号并补全 UNIT，最后刷新该房号的卡数量。
- 单删错误必须和房号错误分开：UNIT 无效才显示 `Room number error`；TAG 未确认、TAG 为空但执行单删语义、或当前 UNIT 下找不到该 TAG，应显示 `Card number error`。
- 删除兼容策略最终定为：TAG 为空时 `ERASE` 删除当前 UNIT 整户；TAG 有值时必须先在 `TAG` 项确认，确认成功后 `ERASE` 按 TAG 反查到的已保存卡槽删除单张卡。

## RFID TAG 反查房号单删修正
- 用户实测发现 TAG 焦点刷卡填充后按确认仍显示 `Room number error`，根因是旧确认路径先调用 `card_manage_prepare_unit_for_card_action()`，即使 TAG 正确也会因为 UNIT 未输入或状态不满足而走房号错误。
- TAG 与 UNIT 同级后，TAG 确认应具备独立入口：通过 `card_manage_find_saved_card_by_tag()` 全局遍历已保存卡槽，找到卡后用 `card_index / 10` 得到房号，再调用 `card_manage_set_unit_by_home_id()` 回填 UNIT 显示。
- 刷卡填充 TAG 时不能只把原始卡 ID 转成显示数字就算成功；必须先用 `card_manage_find_saved_card_by_raw_id()` 确认该卡已保存。未保存卡刷入 TAG 焦点时显示 `Card number error`，并播放错误提示。
- `ERASE TAG` 删除不应再重新校验当前 UNIT 输入框；它应使用已确认 TAG 反查出的卡槽房号作为删除基准，避免 UNIT 输入框状态导致单删误报房号错误。
- 用户重新确认 UNIT 时，应清掉 TAG 和 TAG confirmed 状态，回到 `ERASE ROOM` 整户删除模式，避免上一张 TAG 的单删状态残留。
- `ERASE ROOM` 文本比旧 `ERASE:` 长，原 `{{33, 114}, {125, 40}}` 显示区域不足；需要加宽到 `200` 左右，避免显示不全。

## RFID UNIT 房间操作与 TAG 单删入口互斥
- 确认 `UNIT` 后系统语义已经进入房间操作/添卡模式，此时焦点移动到 `TAG` 只应是浏览当前卡号显示行，不应继续作为单张删卡入口。
- 原刷卡状态机在 `CARD_ADD_CARD_MODE` 下只要 `CardManageClass.cur_focus.main == TAG_FOCUS` 就会调用 `card_manage_fill_tag_by_card_id()`；这会让已保存卡在房间操作模式下刷卡被判定为 TAG 删除准备成功，覆盖原本“已添加卡应提示错误”的添卡语义。
- 正确边界是通过显式状态区分：`UNIT` 路径进入 `card_manage_room_operation_active`，此时禁用 TAG 键盘输入、退格和刷卡填充；`TAG` 路径只能在 RFID 主层、未确认 UNIT、焦点在 TAG 时启用。
- TAG 单删准备成功只是“选中了要删除的卡”，不是新增卡成功，也不是开门成功，因此不应播放成功音；只有卡不存在或卡号错误时播放错误音并显示 `Card number error`。
- TAG 反查补全 UNIT 时内部仍需保持右对齐后的真实房号数组用于计算，但显示层应按直接输入格式显示：房号 `9` 显示 `9`，不是 `0009`；四位房号显示为带空格的输入格式，例如 `1 0 0 1`。
- 提示框显示期间也应刷新 `SwipingCard.tag_fill_request`，避免错误提示还在屏幕上时继续刷卡被当作 TAG 填充。

## RFID 未保存退出后的 TAG/SAVE 残留
- 用户实测补充：如果在 `UNIT` 房间操作中执行 `SAVE`，保存成功提示关闭会走已有结果清理流程，所以不会复现；如果确认 `UNIT` 后没有保存就直接返回，再进入 `TAG` 输入错误卡号，错误弹窗关闭后会重画上一次 `UNIT` 路径留下的旧 `SAVE` 数量和旧 `TAG` 卡号。
- 根因是未保存返回只走 `CARD_MANAGE_MAIN_LAYER_CONFIRM` 的 `*` 返回路径，没有统一清理 `CardManageClass.room_card_info.room_card_num`、`SwipingCard.string_buf[10]`、`SwipingCard.success_show` 和 `card_manage_tag_input` 这些临时结果缓存。
- `card_manage_prompt_close()` 关闭 TAG 错误提示时会重绘页面；如果这些临时缓存仍保留，就会在清掉弹窗后把旧 `SAVE:0` 或旧刷卡 TAG 再画出来。
- 正确边界是：只有仍处于真实 `CARD_ADD_CARD_MODE` 且 `card_manage_room_operation_active` 的添卡流程，才允许关闭普通提示后重画当前刷卡结果；TAG 错误提示和未保存退出路径都必须清 transient 结果状态。
- 修复策略使用统一清理函数清掉临时 TAG/SAVE/刷卡缓存；未保存按 `*` 从确认层返回时额外回到 `UNIT_FOCUS`、`CARD_MANAGE_MAIN_LAYER`、`CARD_IDLE_MODE`，并清空房号输入框，避免旧房间操作状态污染后续 TAG 单删入口。

## RFID TAG 刷卡失败按键音和 UNIT 刷卡删除语义
- 主界面 `TAG` 焦点刷入未保存卡时，提示框显示会刷新 `SwipingCard.tag_fill_request`；旧 `CARD_TAG_FILL_MODE` 结束时如果继续依赖 `tag_fill_request` 判断返回模式，就可能错误回到 `CARD_ADD_CARD_MODE`。
- `user_main.c` 在 `SwipingCard.mode == CARD_ADD_CARD_MODE` 时会抑制普通按键音，因此 TAG 刷卡失败后错误回到添卡模式，会表现为提示消失后按键音消失。
- 正确返回依据不是 `tag_fill_request`，而是当前 RFID 页面层级：主层 TAG 填充失败/结束后回 `CARD_IDLE_MODE`；确认层房间操作中的刷卡处理结束后才回 `CARD_ADD_CARD_MODE`。
- UNIT 房间操作里的刷卡结果只是当前刷卡显示和添卡反馈，不应写入 `card_manage_tag_input` 或清理 TAG 单删输入；否则 `ERASE` 会因为 TAG 输入缓存有值而从整户删除语义切到单张删除语义。
- 修复边界是把“刷卡结果显示”和“TAG 单删输入状态”拆开：`room_card_string_buf_display()` 只调用只读显示函数，`card_manage_card_result_display()` 只把原始卡 ID 转成屏幕数字并绘制，不修改 TAG 输入/确认状态。

## RFID 已确认 TAG 被编辑后的单删状态退出
- TAG 确认成功后会反查并回填 `UNIT` 和 `SAVE`，这些值代表“当前 TAG 已确认并绑定到某户”的上下文，而不是用户直接输入的房间状态。
- 旧退格逻辑只把 `card_manage_tag_confirmed` 置为 `false` 并重画 `ERASE` 文案，但没有清掉由 TAG 确认推导出来的 `UNIT` 和 `SAVE`；用户在 TAG 焦点按 `*` 删除卡号后，界面仍显示旧房号和旧卡数，容易误以为仍处于单删卡模式。
- 正确边界是：只要用户在已确认 TAG 后继续输入或退格，就必须退出本次单删确认上下文，清除旧 `UNIT`、旧 `SAVE`、删除用 TAG 缓存和房号输入缓存；保留用户正在编辑的 TAG 输入本身。
- 该清理不能直接复用 `card_manage_clear_transient_result_state()`，因为那个函数会把 TAG 输入也清空；本场景需要清理“确认上下文”，但允许用户继续编辑当前 TAG。

## RFID TAG 确认后返回键语义
- `card_manage_key_star_up()` 的判断顺序是先处理主层 TAG 输入退格，再处理 `CARD_MANAGE_MAIN_LAYER_CONFIRM` 的返回 reset。
- TAG 确认成功后如果仍停留在 `CARD_MANAGE_MAIN_LAYER`，即使已经回填了 `UNIT` 和 `SAVE`，`*` 仍会被当成 TAG 删除一位处理，表现为 `0015153840` 变成 `001515383`。
- TAG 确认成功后的语义更接近 UNIT 确认后的操作层：用户看到的是已确认卡号对应房号和卡数，此时 `*` 应走 `card_manage_reset_main_input_state()`，返回 RFID 主操作界面，而不是编辑 TAG 字符串。
- 修复边界是 TAG 确认成功后设置 `CardManageClass.cur_focus.layer = CARD_MANAGE_MAIN_LAYER_CONFIRM`，并调用 `card_manage_update_tag_fill_request()` 让 TAG 刷卡输入请求同步关闭。

## RFID ERASE 三态文案
- 旧 `Erase_font_display()` 只按 `card_manage_tag_confirmed` 二分：未确认 TAG 显示 `ERASE ROOM`，已确认 TAG 显示 `ERASE TAG`。
- 用户希望默认状态不要暗示整户删除，因此主层未进入 UNIT 房间操作、也未确认 TAG 时应只显示 `ERASE`。
- 房间操作态由 `card_manage_room_operation_active` 表示，确认 `UNIT` 后进入，此时 `ERASE` 的实际动作是整户删除，应显示 `ERASE ALL`。
- TAG 单删态仍由 `card_manage_tag_confirmed` 表示，优先级高于房间操作态，应显示 `ERASE TAG`。

## 大楼机通话结束后功放延时恢复
- `amplifier_gpio_control(true)` 在当前硬件抽象中表示打开功放，实际把 `IO_AMPLIFIER_CTRL` 拉低；`amplifier_gpio_control(false)` 表示关闭功放，实际把该 GPIO 拉高。
- 监控/通话开始路径 `intercom_monitor_start_process()` 会打开摄像头、功放和咪头。
- 监控/通话结束路径集中在 `monitor_status_check()`：`INT_READ_MONITOR_STATUS` 800ms 收尾和 `INT_TALK` 2 分钟超时强制关闭都会关闭摄像头和咪头，但旧代码没有关闭功放。
- 不能在结束路径直接 `ak_sleep_ms(2000)` 后再打开功放，因为这会阻塞 `intercom_event_detect()`，影响 CAN 收包、心跳发送/避让和业务状态释放。
- 修复边界应放在 `intercom.c` 内：关闭输出时同步关功放并记录时间，后续由事件循环轮询 2 秒恢复；如果 2 秒内重新开始通话，开始流程取消 pending 并立即打开功放。

## Output 房号设置页重复错误后按键锁定
- 用户场景发生在 Output 输入分支器号后进入的 `layout_home_id_set.c`，不是 `layout_OutPUT.c` 首页。
- 重复房号校验失败后，`HomeIdSetClass.set_status` 会变成 `HOME_ID_SET_STATUS_FAIL`，`home_id_set_key_up_up()`、`home_id_set_key_down_up()`、`home_id_set_key_ring_up()` 在主层一开始就判断 `set_status != HOME_ID_SET_STATUS_NONE` 并直接 `return`，因此错误提示显示期间上下/确认都被锁住。
- 数字输入路径 `home_id_set_add_number()` 没有同样的 `set_status` 拦截，所以用户会感觉“只能重新输入，不能上下移动”。
- 只在按键时清掉 `set_status` 还不够；重复错误后 `home_id_input_dirty[]` 仍然为 true，提示消失后上下移动会再次尝试保存同一个错误值并再次失败，表现为持续被卡住。
- 房号设置页 `home_id_set_key_star_up()` 当前无条件 `os_layout_goto(&layout_OutPUT)`，没有像 Output 首页一样先判断当前输入框是否有输入，也没有调用已有 `home_id_set_sub_number()`，所以按 X 会直接退出而不是删除一位。
