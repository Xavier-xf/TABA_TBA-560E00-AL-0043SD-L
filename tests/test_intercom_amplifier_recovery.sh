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
