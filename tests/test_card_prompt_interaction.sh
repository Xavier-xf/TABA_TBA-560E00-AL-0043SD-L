#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LANG_H="$ROOT_DIR/app_cu_datin/system/layout/language.h"
LANG_C="$ROOT_DIR/app_cu_datin/system/layout/language.c"
CARD_NUMBER_C="$ROOT_DIR/app_cu_datin/system/layout/layout_card_number.c"
CARD_MANAGE_C="$ROOT_DIR/app_cu_datin/system/layout/layout_card_manage.c"
SWIPING_CARD_C="$ROOT_DIR/app_cu_datin/system/src/swiping_card.c"
SWIPING_CARD_H="$ROOT_DIR/app_cu_datin/system/src/swiping_card.h"

grep -q 'STR_CARD_NUMBER_ADD_SUCCESS' "$LANG_H"
grep -q 'STR_CARD_NUMBER_DELETE_SUCCESS' "$LANG_H"
grep -q 'STR_CARD_NUMBER_TAG_ERROR' "$LANG_H"
grep -q 'STR_CARD_MANAGE_ERASE_ROOM' "$LANG_H"
grep -q 'STR_CARD_MANAGE_ERASE_TAG' "$LANG_H"
grep -q '{"Add card success"' "$LANG_C"
grep -q '{"Delete card success"' "$LANG_C"
grep -q '{"Card number error"' "$LANG_C"
grep -q '{"ERASE ROOM"' "$LANG_C"
grep -q '{"ERASE TAG"' "$LANG_C"
if grep -q '{"TAG confirmed"' "$LANG_C"; then
	echo "TAG confirmation must not use a popup message"
	exit 1
fi

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
grep -q 'STR_CARD_NUMBER_TAG_ERROR' "$CARD_MANAGE_C"
grep -q 'STR_CARD_MANAGE_ERASE_ROOM' "$CARD_MANAGE_C"
grep -q 'STR_CARD_MANAGE_ERASE_TAG' "$CARD_MANAGE_C"
grep -Fq 'position box_pos = {{152, 73}, {236, 66}};' "$CARD_MANAGE_C"
grep -q 'CARD_PROMPT_ERROR_COLOR 0xFFFF0000' "$CARD_MANAGE_C"
grep -q 'CARD_PROMPT_SUCCESS_COLOR 0xFFFFFFFF' "$CARD_MANAGE_C"
grep -q 'ROM_R_IMG_CARD_MANEAGE_RFID_FOCUS_PNG' "$CARD_MANAGE_C"
grep -q 'prompt.font_color = prompt_color;' "$CARD_MANAGE_C"
grep -q 'card_manage_page_redraw' "$CARD_MANAGE_C"
grep -q 'card_manage_result_text_clear' "$CARD_MANAGE_C"
grep -q 'card_manage_success_result_clear' "$CARD_MANAGE_C"
grep -q 'card_manage_clear_transient_result_state' "$CARD_MANAGE_C"
grep -q 'card_manage_reset_main_input_state' "$CARD_MANAGE_C"
grep -q 'card_manage_focus_reset_to_unit' "$CARD_MANAGE_C"
grep -q 'CARD_MANAGE_TAG_INPUT_MAX 10' "$CARD_MANAGE_C"
grep -q 'card_manage_raw_id_to_tag_digits' "$CARD_MANAGE_C"
grep -q 'card_manage_tag_input_add_number' "$CARD_MANAGE_C"
grep -q 'card_manage_card_result_display' "$CARD_MANAGE_C"
grep -q 'card_manage_fill_tag_by_card_id' "$CARD_MANAGE_C"
grep -q 'card_manage_find_saved_card_by_tag' "$CARD_MANAGE_C"
grep -q 'card_manage_find_saved_card_by_raw_id' "$CARD_MANAGE_C"
grep -q 'card_manage_set_unit_by_home_id' "$CARD_MANAGE_C"
grep -q 'card_manage_format_unit_display' "$CARD_MANAGE_C"
grep -q 'delete_single_card_by_tag' "$CARD_MANAGE_C"
grep -q 'delete_current_card' "$CARD_MANAGE_C"
grep -q 'SwipingCard.tag_fill_request' "$CARD_MANAGE_C"
grep -q 'card_manage_tag_confirmed' "$CARD_MANAGE_C"
grep -q 'card_manage_room_operation_active' "$CARD_MANAGE_C"
grep -q 'card_manage_enter_room_operation' "$CARD_MANAGE_C"
grep -q 'card_manage_leave_room_operation' "$CARD_MANAGE_C"
grep -q 'card_manage_tag_entry_enabled' "$CARD_MANAGE_C"
grep -Fq 'position pos = {{160, 71}, {240, 40}};' "$CARD_MANAGE_C"
grep -Fq 'position tag_pos = {{160, 71}, {240, 40}};' "$CARD_MANAGE_C"
grep -q 'SwipingCard.success_show = false;' "$CARD_MANAGE_C"
grep -q 'SwipingCard.mode == CARD_ADD_CARD_MODE' "$CARD_MANAGE_C"
awk '
	/static void room_card_string_buf_display/ { in_display = 1 }
	in_display && /card_manage_card_result_display\(SwipingCard\.string_buf\[10\]\)/ { saw_result_display = 1 }
	in_display && /card_manage_tag_input_set/ { mutates_tag_input = 1 }
	in_display && /card_manage_tag_input_clear/ { mutates_tag_input = 1 }
	in_display && /^}/ { in_display = 0 }
	END { exit (saw_result_display && !mutates_tag_input) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_card_result_display/ { in_display = 1 }
	in_display && /card_manage_raw_id_to_tag_digits/ { saw_convert = 1 }
	in_display && /text_display\(&tag_text, target_num\)/ { saw_display = 1 }
	in_display && /card_manage_tag_input_set/ { mutates_tag_input = 1 }
	in_display && /card_manage_tag_input_clear/ { mutates_tag_input = 1 }
	in_display && /^}/ { in_display = 0 }
	END { exit (saw_convert && saw_display && !mutates_tag_input) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/bool card_manage_fill_tag_by_card_id/ { in_fill = 1 }
	in_fill && /card_manage_find_saved_card_by_raw_id/ { saw_saved_lookup = 1 }
	in_fill && /CARD_MANAGE_STATUS_TAG_ERROR/ { saw_tag_error = 1 }
	in_fill && /CARD_MANAGE_STATUS_ERROR/ { saw_room_error = 1 }
	in_fill && /^}/ { in_fill = 0 }
	END { exit (saw_saved_lookup && saw_tag_error && !saw_room_error) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void Erase_font_display/ { in_erase = 1 }
	in_erase && /position pos = \{\{33, 114\}, \{1[8-9][0-9], 40\}\}/ { saw_wide = 1 }
	in_erase && /position pos = \{\{33, 114\}, \{2[0-9][0-9], 40\}\}/ { saw_wide = 1 }
	in_erase && /^}/ { in_erase = 0 }
	END { exit saw_wide ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_clear_transient_result_state/ { in_clear = 1 }
	in_clear && /CardManageClass\.room_card_info\.room_card_num = 0;/ { saw_count_clear = 1 }
	in_clear && /memset\(SwipingCard\.string_buf\[10\]/ { saw_tag_buf_clear = 1 }
	in_clear && /SwipingCard\.success_show = false;/ { saw_success_clear = 1 }
	in_clear && /memset\(card_manage_tag_input/ { saw_tag_input_clear = 1 }
	in_clear && /card_manage_tag_input_index = 0;/ { saw_tag_index_clear = 1 }
	in_clear && /card_manage_tag_confirmed = false;/ { saw_tag_confirm_clear = 1 }
	in_clear && /card_manage_tag_replace_on_next_input = false;/ { saw_tag_replace_clear = 1 }
	in_clear && /card_manage_result_text_clear\(\);/ { saw_result_area_clear = 1 }
	in_clear && /^}/ { in_clear = 0 }
	END { exit (saw_count_clear && saw_tag_buf_clear && saw_success_clear && saw_tag_input_clear && saw_tag_index_clear && saw_tag_confirm_clear && saw_tag_replace_clear && saw_result_area_clear) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_reset_main_input_state/ { in_reset = 1 }
	in_reset && /SwipingCard\.mode = CARD_IDLE_MODE;/ { saw_idle = 1 }
	in_reset && /SwipingCard\.tag_fill_request = false;/ { saw_tag_request_clear = 1 }
	in_reset && /card_manage_leave_room_operation\(\);/ { saw_leave_room = 1 }
	in_reset && /CardManageClass\.cur_focus\.layer = CARD_MANAGE_MAIN_LAYER;/ { saw_main_layer = 1 }
	in_reset && /card_manage_clear_transient_result_state\(\);/ { saw_clear = 1 }
	in_reset && /CardManageClass\.dialog_box->cursor\.index = 0;/ { saw_cursor_clear = 1 }
	in_reset && /^}/ { in_reset = 0 }
	END { exit (saw_idle && saw_tag_request_clear && saw_leave_room && saw_main_layer && saw_clear && saw_cursor_clear) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_focus_reset_to_unit/ { in_reset = 1 }
	in_reset && /CardManageClass\.cur_focus\.main = UNIT_FOCUS;/ { saw_unit = 1 }
	in_reset && /CardManageClass\.cur_focus\.layer = CARD_MANAGE_MAIN_LAYER;/ { saw_layer = 1 }
	in_reset && /display_current_card_manage_focus\(\);/ { saw_direct_display = 1 }
	in_reset && /^}/ { in_reset = 0 }
	END { exit (saw_unit && saw_layer && !saw_direct_display) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_prompt_close/ { in_close = 1 }
	in_close && /CARD_MANAGE_STATUS_DELETE_CARD/ { saw_delete = 1 }
	in_close && /CARD_MANAGE_STATUS_SAVE_CARD/ { saw_save = 1 }
	in_close && /closing_status != CARD_MANAGE_STATUS_TAG_ERROR/ { saw_tag_error_guard = 1 }
	in_close && /card_manage_room_operation_active/ { saw_room_operation_guard = 1 }
	in_close && /card_manage_focus_reset_to_unit\(\);/ { saw_reset = 1 }
	in_close && /card_manage_page_redraw\(\);/ && saw_reset { saw_redraw_after_reset = 1 }
	in_close && /^}/ { in_close = 0 }
	END { exit (saw_delete && saw_save && saw_tag_error_guard && saw_room_operation_guard && saw_reset && saw_redraw_after_reset) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_key_star_up/ { in_star = 1 }
	in_star && /CARD_MANAGE_MAIN_LAYER_CONFIRM/ { saw_confirm_path = 1 }
	in_star && /card_manage_reset_main_input_state\(\);/ { saw_reset = 1 }
	in_star && /os_layout_goto\(&layout_card_manage\)/ && saw_reset { saw_goto_after_reset = 1 }
	in_star && /^}/ { in_star = 0 }
	END { exit (saw_confirm_path && saw_reset && saw_goto_after_reset) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/SwipingCard\.mode == CARD_ADD_CARD_MODE/ { in_mode_check = 1 }
	in_mode_check && /room_card_numbe_display\(\);/ { redraw_count = 1 }
	in_mode_check && /room_card_string_buf_display\(\);/ { redraw_tag = 1 }
	in_mode_check && /card_manage_success_result_clear\(\);/ { clear_result = 1 }
	END { exit (redraw_count && redraw_tag && clear_result) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static int get_room_card_number_by_room_num/ { in_count = 1 }
	in_count && /break;/ { saw_break = 1 }
	in_count && /^}/ { in_count = 0 }
	END { exit saw_break ? 1 : 0 }
' "$CARD_MANAGE_C"
awk '
	/static bool card_manage_tag_entry_enabled/ { in_entry = 1 }
	in_entry && /CardManageClass\.cur_focus\.status == CARD_MANAGE_STATUS_NONE/ { saw_status = 1 }
	in_entry && /CardManageClass\.cur_focus\.layer == CARD_MANAGE_MAIN_LAYER/ { saw_main_layer = 1 }
	in_entry && /CardManageClass\.cur_focus\.main == TAG_FOCUS/ { saw_tag = 1 }
	in_entry && /!card_manage_room_operation_active/ { saw_room_guard = 1 }
	in_entry && /^}/ { in_entry = 0 }
	END { exit (saw_status && saw_main_layer && saw_tag && saw_room_guard) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_input_add_number/ { in_input = 1 }
	in_input && /card_manage_tag_entry_enabled\(\)/ { saw_entry = 1 }
	in_input && /card_manage_tag_input_add_number/ { saw_tag_input = 1 }
	in_input && /^}/ { in_input = 0 }
	END { exit (saw_entry && saw_tag_input) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_input_sub_number/ { in_input = 1 }
	in_input && /card_manage_tag_entry_enabled\(\)/ { saw_entry = 1 }
	in_input && /card_manage_tag_input_sub_number/ { saw_tag_input = 1 }
	in_input && /^}/ { in_input = 0 }
	END { exit (saw_entry && saw_tag_input) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_update_tag_fill_request/ { in_update = 1 }
	in_update && /card_manage_tag_entry_enabled\(\)/ { saw_tag_entry = 1 }
	in_update && /^}/ { in_update = 0 }
	END { exit saw_tag_entry ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static bool card_manage_prepare_unit_for_card_action/ { in_prepare = 1 }
	in_prepare && /card_manage_home_id_adjust/ { saw_adjust = 1 }
	in_prepare && /CardManageClass\.dialog_box->cursor\.index = CardManageClass\.dialog_box->cursor\.max_index \+ 1;/ { saw_index_done = 1 }
	in_prepare && /card_manage_dialog_box_font_change/ { redraws_padded_unit = 1 }
	in_prepare && /CardManageClass\.widget_show\.dialog_box/ { redraws_padded_unit = 1 }
	in_prepare && /^}/ { in_prepare = 0 }
	END { exit (saw_adjust && saw_index_done && !redraws_padded_unit) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_prompt_tag_error/ { in_error = 1 }
	in_error && /warn_sound_play\(\)/ { saw_warn = 1 }
	in_error && /CARD_MANAGE_STATUS_TAG_ERROR/ { saw_tag_error = 1 }
	in_error && /^}/ { in_error = 0 }
	END { exit (saw_warn && saw_tag_error) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static bool card_manage_try_delete_confirmed_tag/ { in_delete = 1 }
	in_delete && /card_manage_prepare_unit_for_card_action/ { saw_unit_check = 1 }
	in_delete && /card_manage_tag_has_value/ { saw_tag_check = 1 }
	in_delete && /card_manage_tag_confirmed/ { saw_tag_confirmed = 1 }
	in_delete && /card_manage_find_saved_card_by_tag/ { saw_saved_tag_match = 1 }
	in_delete && /card_manage_prompt_tag_error/ { saw_tag_error = 1 }
	in_delete && /delete_single_card_by_tag/ { saw_single_delete = 1 }
	in_delete && /delete_current_card/ { saw_whole_delete = 1 }
	in_delete && /^}/ { in_delete = 0 }
	END { exit (saw_unit_check && saw_tag_check && saw_tag_confirmed && saw_saved_tag_match && saw_tag_error && saw_single_delete && saw_whole_delete) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_key_ring_up/ { in_ring = 1 }
	in_ring && /ERASE_FOCUS/ { saw_erase = 1 }
	in_ring && /TAG_FOCUS/ { saw_tag = 1 }
	in_ring && /UNIT_FOCUS/ { saw_unit = 1 }
	in_ring && /card_manage_tag_input_clear/ { saw_unit_clears_tag = 1 }
	in_ring && /card_manage_confirm_tag_for_delete/ { saw_tag_confirm = 1 }
	in_ring && /CARD_MANAGE_STATUS_TAG_CONFIRMED/ { saw_tag_confirm_prompt = 1 }
	in_ring && /card_manage_try_delete_confirmed_tag/ { saw_delete_helper = 1 }
	in_ring && /^}/ { in_ring = 0 }
	END { exit (saw_erase && saw_tag && saw_unit && saw_unit_clears_tag && saw_tag_confirm && !saw_tag_confirm_prompt && saw_delete_helper) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static bool card_manage_confirm_tag_for_delete/ { in_confirm = 1 }
	in_confirm && /card_manage_prepare_unit_for_card_action/ { saw_unit_check = 1 }
	in_confirm && /CARD_MANAGE_STATUS_ERROR/ { saw_room_error = 1 }
	in_confirm && /get_room_card_number_by_room_num/ { saw_count = 1 }
	in_confirm && /room_card_numbe_display/ { saw_count_display = 1 }
	in_confirm && /card_manage_find_saved_card_by_tag/ { saw_tag_match = 1 }
	in_confirm && /card_manage_set_unit_by_home_id/ { saw_unit_fill = 1 }
	in_confirm && /card_manage_tag_confirmed = true;/ { saw_confirm = 1 }
	in_confirm && /card_manage_leave_room_operation/ { saw_leave_room = 1 }
	in_confirm && /card_manage_prompt_show\(CARD_MANAGE_STATUS_TAG_CONFIRMED\)/ { saw_popup = 1 }
	in_confirm && /^}/ { in_confirm = 0 }
	END { exit (!saw_unit_check && !saw_room_error && saw_count && saw_count_display && saw_tag_match && saw_unit_fill && saw_confirm && saw_leave_room && !saw_popup) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void Erase_font_display/ { in_erase = 1 }
	in_erase && /STR_CARD_MANAGE_ERASE_TAG/ { saw_tag = 1 }
	in_erase && /STR_CARD_MANAGE_ERASE_ROOM/ { saw_room = 1 }
	in_erase && /^}/ { in_erase = 0 }
	END { exit (saw_tag && saw_room) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_tag_input_add_number/ { in_add = 1 }
	in_add && /card_manage_tag_confirmed = false;/ { add_clears = 1 }
	in_add && /^}/ { in_add = 0 }
	/static void card_manage_tag_input_sub_number/ { in_sub = 1 }
	in_sub && /card_manage_tag_confirmed = false;/ { sub_clears = 1 }
	in_sub && /^}/ { in_sub = 0 }
	/static void card_manage_tag_input_set/ { in_set = 1 }
	in_set && /card_manage_tag_confirmed = false;/ { set_clears = 1 }
	in_set && /^}/ { in_set = 0 }
	/static void card_manage_tag_input_clear/ { in_clear = 1 }
	in_clear && /card_manage_tag_confirmed = false;/ { clear_clears = 1 }
	in_clear && /^}/ { in_clear = 0 }
	END { exit (add_clears && sub_clears && set_clears && clear_clears) ? 0 : 1 }
' "$CARD_MANAGE_C"
awk '
	/static void card_manage_set_unit_by_home_id/ { in_set = 1 }
	in_set && /char unit_string/ { saw_unit_string = 1 }
	in_set && /sprintf\(unit_string, "%d", home_id\)/ { saw_decimal = 1 }
	in_set && /card_manage_format_unit_display\(unit_string/ { saw_display_string = 1 }
	in_set && /CardManageClass\.dialog_box->cursor\.index = CardManageClass\.dialog_box->cursor\.max_index \+ 1/ { saw_full_index = 1 }
	in_set && /card_manage_dialog_box_font_change/ { saw_padded_redraw = 1 }
	in_set && /^}/ { in_set = 0 }
	END { exit (saw_unit_string && saw_decimal && saw_display_string && saw_full_index && !saw_padded_redraw) ? 0 : 1 }
' "$CARD_MANAGE_C"
grep -Fq '{UNIT_FOCUS, 160, 28}' "$CARD_MANAGE_C"
grep -Fq '{TAG_FOCUS, 160, 71}' "$CARD_MANAGE_C"
grep -Fq '{ERASE_FOCUS, 160, 114}' "$CARD_MANAGE_C"
grep -Fq '{SAVE_FOCUS, 160, 157}' "$CARD_MANAGE_C"

grep -q 'CARD_TAG_FILL_MODE' "$SWIPING_CARD_H"
grep -q 'tag_fill_request' "$SWIPING_CARD_H"
grep -q 'card_manage_fill_tag_by_card_id' "$SWIPING_CARD_C"
grep -q 'CARD_TAG_FILL_MODE' "$SWIPING_CARD_C"
awk '
	/case CARD_IDLE_MODE:/ { in_idle = 1 }
	in_idle && /SwipingCard\.tag_fill_request/ { saw_request = 1 }
	in_idle && /card_manage_fill_tag_by_card_id/ { saw_fill = 1 }
	in_idle && /CARD_TAG_FILL_MODE/ { saw_fill_mode = 1 }
	in_idle && /swiping_card_sound_play/ { saw_success_sound = 1 }
	in_idle && /warn_sound_play/ { saw_error_sound = 1 }
	in_idle && /case CARD_SWIPING_CARD_MODE:/ { in_idle = 0 }
	END { exit (saw_request && saw_fill && saw_fill_mode && !saw_success_sound && saw_error_sound) ? 0 : 1 }
' "$SWIPING_CARD_C"
awk '
	/case CARD_ADD_CARD_MODE:/ { in_add = 1 }
	in_add && /SwipingCard\.tag_fill_request/ { saw_request = 1 }
	in_add && /CardManageClass\.cur_focus\.main == TAG_FOCUS/ { saw_focus_shortcut = 1 }
	in_add && /card_manage_fill_tag_by_card_id/ { saw_fill = 1 }
	in_add && /warn_sound_play/ { saw_error_sound = 1 }
	in_add && /case CARD_UNLOCK_MODE:/ { in_add = 0 }
	END { exit (saw_request && saw_fill && !saw_focus_shortcut && saw_error_sound) ? 0 : 1 }
' "$SWIPING_CARD_C"
awk '
	/if \(SwipingCard\.tag_fill_request\)/ { in_tag_fill = 1 }
	in_tag_fill && /card_manage_fill_tag_by_card_id/ { saw_fill = 1 }
	in_tag_fill && /swiping_card_sound_play/ { saw_success_sound = 1 }
	in_tag_fill && /warn_sound_play/ { saw_error_sound = 1 }
	in_tag_fill && /CARD_TAG_FILL_MODE/ { in_tag_fill = 0 }
	END { exit (saw_fill && !saw_success_sound && saw_error_sound) ? 0 : 1 }
' "$SWIPING_CARD_C"
awk '
	/case CARD_TAG_FILL_MODE:/ { in_tag_mode = 1 }
	in_tag_mode && /CARD_MANAGE_MAIN_LAYER/ { saw_main_layer_return = 1 }
	in_tag_mode && /CARD_IDLE_MODE/ { saw_idle_return = 1 }
	in_tag_mode && /CARD_ADD_CARD_MODE/ { saw_add_return = 1 }
	in_tag_mode && /SwipingCard\.tag_fill_request/ { saw_request_dependency = 1 }
	in_tag_mode && /case CARD_ADD_CARD_PROCESS_MODE:/ { in_tag_mode = 0 }
	END { exit (saw_main_layer_return && saw_idle_return && saw_add_return && !saw_request_dependency) ? 0 : 1 }
' "$SWIPING_CARD_C"

if grep -q 'if (CardManageClass.cur_focus.status != CARD_MANAGE_STATUS_NONE)[[:space:]]*return;' "$CARD_MANAGE_C"; then
    echo "card manage prompt must be dismissible instead of swallowing keys"
    exit 1
fi
if grep -q 'CARD_MANAGE_PROMPT_BG_COLOR' "$CARD_MANAGE_C"; then
	echo "card manage prompt must use rfid_focus image instead of solid color"
	exit 1
fi

echo "card prompt interaction checks passed"
