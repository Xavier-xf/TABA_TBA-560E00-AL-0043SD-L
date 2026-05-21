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
