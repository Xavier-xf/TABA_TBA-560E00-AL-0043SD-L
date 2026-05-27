#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INTERCOM_C="$ROOT_DIR/app_cu_datin/system/src/intercom.c"
INTERCOM_H="$ROOT_DIR/app_cu_datin/system/src/intercom.h"

grep -q 'INTERCOM_HEARTBEAT_DEFER_MS' "$INTERCOM_H"
grep -q 'INTERCOM_BUS_BUSY_TIMEOUT_MS' "$INTERCOM_H"
grep -q 'intercom_mark_bus_activity();' "$INTERCOM_C"
grep -q 'intercom_can_send_cmd(' "$INTERCOM_C"
grep -q 'intercom_bus_busy_timed_out' "$INTERCOM_C"
grep -q 'intercom_business_begin' "$INTERCOM_C"
grep -q 'intercom_business_finish_delay' "$INTERCOM_C"
grep -q 'intercom_bus_busy_release' "$INTERCOM_C"
grep -q 'intercom_can_send_cmd,[[:space:]]*// send_cmd' "$INTERCOM_C"

heartbeat_block="$(sed -n '/static void intercom_heartbeat_check(void)/,/^}/p' "$INTERCOM_C")"
if ! grep -q 'intercom_bus_busy_timed_out' <<<"$heartbeat_block"; then
    echo "Heartbeat check must release busy state after timeout"
    exit 1
fi

if ! grep -q 'intercom_heartbeat_deferred' <<<"$heartbeat_block"; then
    echo "Heartbeat check must defer while intercom bus is busy"
    exit 1
fi

deferred_block="$(sed -n '/if (intercom_heartbeat_deferred(&now_time))/,/if (g_intercom_bus_busy)/p' "$INTERCOM_C")"
if grep -q 'g_heartbeat_send_time = now_time' <<<"$deferred_block"; then
    echo "Deferred heartbeat must not push heartbeat timestamp forward"
    exit 1
fi

if grep -q 'Intercom.send_cmd(CMD_HEARTBEAT' "$INTERCOM_C"; then
    echo "Heartbeat must bypass the public send callback to avoid marking itself as business activity"
    exit 1
fi

grep -q 'INTERCOM_BUSINESS_READ_HOME_ID' "$INTERCOM_C"
grep -q 'INTERCOM_BUSINESS_SET_HOME_ID' "$INTERCOM_C"
grep -q 'INTERCOM_BUSINESS_CALL' "$INTERCOM_C"
grep -q 'INTERCOM_BUSINESS_READ_MONITOR_STATUS' "$INTERCOM_C"

event_block="$(sed -n '/void intercom_event_detect(void)/,/^}/p' "$INTERCOM_C")"
if ! grep -q 'if (Intercom.receive_cmd' <<<"$event_block"; then
    echo "intercom_event_detect must still read CAN frames"
    exit 1
fi

receive_line="$(grep -n 'if (Intercom.receive_cmd' "$INTERCOM_C" | head -n1 | cut -d: -f1)"
heartbeat_line="$(grep -n 'intercom_heartbeat_check();' "$INTERCOM_C" | head -n1 | cut -d: -f1)"
if [[ -z "$receive_line" || -z "$heartbeat_line" || "$receive_line" -ge "$heartbeat_line" ]]; then
    echo "Receive processing must happen before heartbeat check"
    exit 1
fi

echo "Intercom heartbeat deferral static checks passed"
