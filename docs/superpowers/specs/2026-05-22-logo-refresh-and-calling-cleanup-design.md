# Bottom Logo Refresh And Calling Cleanup Design

## Goal

修复三个界面问题：

1. 波斯语底部 logo 中 `تابا` 与 `الکترونیک` 间距偏大。
2. 在系统设置的语言设置界面切换语言时，底部 logo 文字不能立即跟随切换。
3. `calling` 界面英语四位房号在无应答后，最右侧有 `.` 残留。

## Scope

只修改以下现有布局代码：

- `app_cu_datin/system/layout/layout_base.c`
- `app_cu_datin/system/layout/layout_system_set.c`
- `app_cu_datin/system/layout/layout_calling.c`
- `tests/test_bottom_logo_language.sh`

不改资源图，不改其他页面的布局切换流程，不改语言表内容。

## Design

### 1. Tighten Persian Logo Layout

继续保留英语和波斯语分支坐标。

- 英语坐标保持不变。
- 波斯语坐标单独收紧：
  - `تابا` 向左移动一点。
  - `الکترونیک` 向右移动一点。
  - 目标是减小两段文字视觉间距，不改变字号。

### 2. Refresh Logo On Language Toggle

当前 `layout_system_set.c` 在语言切换时只执行：

- `language_set(...)`
- `font_file_reload()`
- `SystemSetClass.widget_show.font()`

这会让顶部文字立即更新，但不会重绘底部 logo。

修复方式：

- 在语言切换分支中增加底部区域擦除。
- 之后立即调用 `taba_btn_display()` 重绘底部 logo。

这样当前页面不退出也能看到 logo 文字随语言即时变化。

### 3. Unify Calling Text Clear Area

`layout_calling.c` 中：

- 呼叫中房号文本直接绘制，没有统一清空区域。
- `No Answer` 使用了较小的单独清除区域。

四位英语房号时，`...` 更靠右，旧内容可能超出 `No Answer` 的擦除区域。

修复方式：

- 抽一个统一的 calling 文本区域清除函数。
- `calling_ring_font_display()`、`calling_guard_font_display()`、`no_answer_font_display()` 都先擦同一块区域再绘制。
- 区域略大于当前文本显示范围，覆盖四位房号和省略号。

## Testing

使用现有 shell 静态测试扩展以下断言：

- 波斯语 logo 使用新的紧凑坐标。
- 语言切换分支里包含底部区域 `gui_erase()` 和 `taba_btn_display()`。
- `layout_calling.c` 存在统一的 calling 文本清除函数，且三种显示路径都调用它。

然后执行：

- `sh tests/test_bottom_logo_language.sh`
- `cd app_cu_datin && ./autobuild.sh -all-sdk`
- `cd AK37E_SDK_V1.03/upgrade && upgrade_bin_version=$(date +%Y%m%d%H%M%S) ./partition_image.sh app_resource`
