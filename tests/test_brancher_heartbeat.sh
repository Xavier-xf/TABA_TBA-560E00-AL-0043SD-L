#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
APP_HEADER="$ROOT_DIR/app_cu_datin/system/src/intercom.h"
APP_SOURCE="$ROOT_DIR/app_cu_datin/system/src/intercom.c"
SWITCH_HEADER="$ROOT_DIR/switch/code/include/kevin_function.h"
SWITCH_SOURCE="$ROOT_DIR/switch/code/msg_event.c"

app_cmd="$(sed -n 's/^#define[[:space:]]\+CMD_HEARTBEAT[[:space:]]\+\(0x[0-9A-Fa-f]\+\).*/\1/p' "$APP_HEADER")"
switch_cmd="$(sed -n 's/^#define[[:space:]]\+CMD_HEARTBEAT[[:space:]]\+\(0x[0-9A-Fa-f]\+\).*/\1/p' "$SWITCH_HEADER")"

if [[ -z "$app_cmd" || -z "$switch_cmd" || "$app_cmd" != "$switch_cmd" ]]; then
    echo "CMD_HEARTBEAT must exist in app and switch headers with the same value"
    exit 1
fi

grep -q 'INTERCOM_HEARTBEAT_INTERVAL_MS' "$APP_HEADER"
grep -q 'send_can_cmd_encode(CMD_HEARTBEAT' "$APP_SOURCE"
grep -q 'intercom_heartbeat_check();' "$APP_SOURCE"

grep -q 'case CMD_HEARTBEAT:' "$SWITCH_SOURCE"
grep -q 'intercom_receive_heartbeat();' "$SWITCH_SOURCE"
grep -q 'intercom_refresh_link_online' "$SWITCH_SOURCE"
grep -q 'BRANCHER_HEARTBEAT_TIMEOUT_MS' "$SWITCH_SOURCE"
grep -q 'BRANCHER_LED_BLINK_INTERVAL_MS' "$SWITCH_SOURCE"
grep -q 'brancher_link_status_check();' "$SWITCH_SOURCE"

heartbeat_block="$(sed -n '/case CMD_HEARTBEAT:/,/break;/p' "$SWITCH_SOURCE")"
if grep -q 'send_can_cmd_encode' <<<"$heartbeat_block"; then
    echo "Brancher heartbeat handling must not reply on the bus"
    exit 1
fi

if grep -q 'set_timer(TIMER0.*led' "$SWITCH_SOURCE"; then
    echo "Brancher LED blink must not use TIMER0 because unlock debounce owns it"
    exit 1
fi

intercom_block="$(sed -n '/static void sys_intercome_check(void)/,/^}/p' "$SWITCH_SOURCE")"
if ! grep -q 'intercom_refresh_link_online();' <<<"$intercom_block"; then
    echo "Any valid master command must refresh brancher online state"
    exit 1
fi

echo "Brancher heartbeat static checks passed"
