#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LANG_H="$ROOT_DIR/app_cu_datin/system/layout/language.h"
LANG_C="$ROOT_DIR/app_cu_datin/system/layout/language.c"
CARD_NUMBER_C="$ROOT_DIR/app_cu_datin/system/layout/layout_card_number.c"
CARD_MANAGE_C="$ROOT_DIR/app_cu_datin/system/layout/layout_card_manage.c"

grep -q 'STR_CARD_NUMBER_ADD_SUCCESS' "$LANG_H"
grep -q 'STR_CARD_NUMBER_DELETE_SUCCESS' "$LANG_H"
grep -q '{"Add card success"' "$LANG_C"
grep -q '{"Delete card success"' "$LANG_C"

grep -q 'CARD_NUMBER_PROMPT_TIMEOUT_TICKS 67' "$CARD_NUMBER_C"
grep -q 'card_number_prompt_consume_key' "$CARD_NUMBER_C"
grep -q 'card_number_prompt_show' "$CARD_NUMBER_C"
grep -q 'card_number_prompt_close' "$CARD_NUMBER_C"
grep -q 'STR_CARD_NUMBER_DELETE_SUCCESS' "$CARD_NUMBER_C"
grep -Fq 'position box_pos = {{152, 73}, {236, 66}};' "$CARD_NUMBER_C"
grep -q 'CARD_PROMPT_ERROR_COLOR 0xFFFF0000' "$CARD_NUMBER_C"
grep -q 'CARD_PROMPT_SUCCESS_COLOR 0xFFFFFFFF' "$CARD_NUMBER_C"
grep -q 'ROM_R_IMG_CARD_MANEAGE_RFID_FOCUS_PNG' "$CARD_NUMBER_C"
grep -q 'prompt.font_color = prompt_color;' "$CARD_NUMBER_C"
grep -q 'card_number_page_redraw' "$CARD_NUMBER_C"
if grep -q 'CARD_NUMBER_PROMPT_BG_COLOR' "$CARD_NUMBER_C"; then
	echo "card number prompt must use rfid_focus image instead of solid color"
	exit 1
fi

grep -q 'CARD_MANAGE_PROMPT_TIMEOUT_TICKS 67' "$CARD_MANAGE_C"
grep -q 'card_manage_prompt_consume_key' "$CARD_MANAGE_C"
grep -q 'card_manage_prompt_show' "$CARD_MANAGE_C"
grep -q 'card_manage_prompt_close' "$CARD_MANAGE_C"
grep -q 'STR_CARD_NUMBER_ADD_SUCCESS' "$CARD_MANAGE_C"
grep -q 'STR_CARD_NUMBER_DELETE_SUCCESS' "$CARD_MANAGE_C"
grep -Fq 'position box_pos = {{152, 73}, {236, 66}};' "$CARD_MANAGE_C"
grep -q 'CARD_PROMPT_ERROR_COLOR 0xFFFF0000' "$CARD_MANAGE_C"
grep -q 'CARD_PROMPT_SUCCESS_COLOR 0xFFFFFFFF' "$CARD_MANAGE_C"
grep -q 'ROM_R_IMG_CARD_MANEAGE_RFID_FOCUS_PNG' "$CARD_MANAGE_C"
grep -q 'prompt.font_color = prompt_color;' "$CARD_MANAGE_C"
grep -q 'card_manage_page_redraw' "$CARD_MANAGE_C"
grep -q 'card_manage_result_text_clear' "$CARD_MANAGE_C"
grep -q 'card_manage_success_result_clear' "$CARD_MANAGE_C"
grep -q 'SwipingCard.success_show = false;' "$CARD_MANAGE_C"
grep -q 'SwipingCard.mode == CARD_ADD_CARD_MODE' "$CARD_MANAGE_C"
awk '
	/SwipingCard\.mode == CARD_ADD_CARD_MODE/ { in_mode_check = 1 }
	in_mode_check && /room_card_numbe_display\(\);/ { redraw_count = 1 }
	in_mode_check && /room_card_string_buf_display\(\);/ { redraw_tag = 1 }
	in_mode_check && /card_manage_success_result_clear\(\);/ { clear_result = 1 }
	END { exit (redraw_count && redraw_tag && clear_result) ? 0 : 1 }
' "$CARD_MANAGE_C"
grep -Fq '{UNIT_FOCUS, 160, 28}' "$CARD_MANAGE_C"
grep -Fq '{TAG_FOCUS, 160, 71}' "$CARD_MANAGE_C"
grep -Fq '{ERASE_FOCUS, 160, 114}' "$CARD_MANAGE_C"
grep -Fq '{SAVE_FOCUS, 160, 157}' "$CARD_MANAGE_C"

if grep -q 'if (CardManageClass.cur_focus.status != CARD_MANAGE_STATUS_NONE)[[:space:]]*return;' "$CARD_MANAGE_C"; then
    echo "card manage prompt must be dismissible instead of swallowing keys"
    exit 1
fi
if grep -q 'CARD_MANAGE_PROMPT_BG_COLOR' "$CARD_MANAGE_C"; then
	echo "card manage prompt must use rfid_focus image instead of solid color"
	exit 1
fi

echo "card prompt interaction checks passed"
