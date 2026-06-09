# Intercom Amplifier Delayed Reopen Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 通话/监控结束时关闭摄像头、咪头和功放，并在 2 秒后自动恢复功放。

**Architecture:** 在 `intercom.c` 内部增加通话输出关闭 helper 和功放延时恢复状态，不改变 GPIO 抽象、不新增线程、不阻塞事件循环。通话开始时取消延时恢复，结束路径统一调用关闭 helper。

**Tech Stack:** Embedded C, Anyka OS time API, Bash static regression tests.

---

### Task 1: Add Static Regression Test

**Files:**
- Create: `tests/test_intercom_amplifier_recovery.sh`

- [ ] **Step 1: Write the failing test**

创建 Bash 静态测试，检查：

```bash
#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INTERCOM_C="$ROOT_DIR/app_cu_datin/system/src/intercom.c"
INTERCOM_H="$ROOT_DIR/app_cu_datin/system/src/intercom.h"

grep -q 'INTERCOM_AMPLIFIER_REOPEN_DELAY_MS[[:space:]]\+2000' "$INTERCOM_H"
grep -q 'g_amplifier_reopen_pending' "$INTERCOM_C"
grep -q 'intercom_talk_output_close_then_reopen_amp' "$INTERCOM_C"
grep -q 'intercom_amplifier_reopen_check();' "$INTERCOM_C"
grep -q 'intercom_amplifier_reopen_cancel();' "$INTERCOM_C"

close_block="$(sed -n '/static void intercom_talk_output_close_then_reopen_amp(void)/,/^}/p' "$INTERCOM_C")"
grep -q 'camera_led_gpio_control(false);' <<<"$close_block"
grep -q 'camera_power_gpio_control(false);' <<<"$close_block"
grep -q 'mic_mute_gpio_control(false);' <<<"$close_block"
grep -q 'amplifier_gpio_control(false);' <<<"$close_block"
grep -q 'g_amplifier_reopen_pending = true;' <<<"$close_block"

reopen_block="$(sed -n '/static void intercom_amplifier_reopen_check(void)/,/^}/p' "$INTERCOM_C")"
grep -q 'ak_diff_ms_time(&now_time, &g_amplifier_reopen_time) >= INTERCOM_AMPLIFIER_REOPEN_DELAY_MS' <<<"$reopen_block"
grep -q 'amplifier_gpio_control(true);' <<<"$reopen_block"
grep -q 'g_amplifier_reopen_pending = false;' <<<"$reopen_block"

start_block="$(sed -n '/static void intercom_monitor_start_process(void)/,/^}/p' "$INTERCOM_C")"
if ! grep -q 'intercom_amplifier_reopen_cancel();' <<<"$start_block"; then
    echo "Monitor start must cancel pending amplifier reopen"
    exit 1
fi
if ! grep -q 'amplifier_gpio_control(true);' <<<"$start_block"; then
    echo "Monitor start must still open amplifier immediately"
    exit 1
fi

if [[ "$(grep -c 'intercom_talk_output_close_then_reopen_amp();' "$INTERCOM_C")" -lt 2 ]]; then
    echo "Both monitor close paths must use the talk output close helper"
    exit 1
fi

echo "Intercom amplifier delayed reopen static checks passed"
```

- [ ] **Step 2: Run test to verify it fails**

Run: `bash tests/test_intercom_amplifier_recovery.sh`

Expected: FAIL because the constant and helper do not exist yet.

### Task 2: Implement Delayed Reopen

**Files:**
- Modify: `app_cu_datin/system/src/intercom.h`
- Modify: `app_cu_datin/system/src/intercom.c`

- [ ] **Step 1: Add constant**

Add:

```c
#define INTERCOM_AMPLIFIER_REOPEN_DELAY_MS 2000
```

- [ ] **Step 2: Add state and helper functions**

Add static state in `intercom.c`:

```c
static struct ak_timeval g_amplifier_reopen_time = {0, 0};
static bool g_amplifier_reopen_pending = false;
```

Add helper functions:

```c
static void intercom_amplifier_reopen_cancel(void)
{
	g_amplifier_reopen_pending = false;
}

static void intercom_talk_output_close_then_reopen_amp(void)
{
	camera_led_gpio_control(false);
	camera_power_gpio_control(false);
	mic_mute_gpio_control(false);
	amplifier_gpio_control(false);
	ak_get_ostime(&g_amplifier_reopen_time);
	g_amplifier_reopen_pending = true;
}

static void intercom_amplifier_reopen_check(void)
{
	struct ak_timeval now_time;

	if (!g_amplifier_reopen_pending)
	{
		return;
	}

	ak_get_ostime(&now_time);
	if (ak_diff_ms_time(&now_time, &g_amplifier_reopen_time) >= INTERCOM_AMPLIFIER_REOPEN_DELAY_MS)
	{
		amplifier_gpio_control(true);
		g_amplifier_reopen_pending = false;
	}
}
```

- [ ] **Step 3: Wire helpers into flow**

In `intercom_monitor_start_process()`, call `intercom_amplifier_reopen_cancel()` before opening GPIO outputs.

Replace the duplicated close sequences in `monitor_status_check()` with `intercom_talk_output_close_then_reopen_amp();`.

Call `intercom_amplifier_reopen_check();` from `intercom_event_detect()` after `monitor_status_check();`.

### Task 3: Verify

**Files:**
- Test: `tests/test_intercom_amplifier_recovery.sh`
- Test: `tests/test_intercom_heartbeat_deferral.sh`
- Test: `tests/test_brancher_heartbeat.sh`

- [ ] **Step 1: Run static tests**

Run:

```bash
bash tests/test_intercom_amplifier_recovery.sh
bash tests/test_intercom_heartbeat_deferral.sh
bash tests/test_brancher_heartbeat.sh
```

Expected: all pass.

- [ ] **Step 2: Run whitespace check**

Run:

```bash
git -c core.whitespace=cr-at-eol diff --check -- app_cu_datin/system/src/intercom.c app_cu_datin/system/src/intercom.h tests/test_intercom_amplifier_recovery.sh task_plan.md findings.md progress.md
```

Expected: no output, exit 0.

- [ ] **Step 3: Build app**

Run:

```bash
cd app_cu_datin && make
```

Expected: exit 0.

- [ ] **Step 4: Package APP partition**

Run:

```bash
cd app_cu_datin && ./autobuild.sh -all-sdk
cd AK37E_SDK_V1.03/upgrade && export upgrade_bin_version=$(date +%Y%m%d%H%M%S) && ./partition_image.sh app_resource
```

Expected: `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS` contains only `app.sqsh4`.
