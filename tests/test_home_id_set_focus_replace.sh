#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HOME_ID_SET_C="$ROOT_DIR/app_cu_datin/system/layout/layout_home_id_set.c"

grep -q 'home_id_set_prepare_input_replace' "$HOME_ID_SET_C"
grep -q 'home_id_replace_on_next_input' "$HOME_ID_SET_C"
grep -q 'home_id_input_dirty' "$HOME_ID_SET_C"
grep -q 'home_id_set_mark_replace_on_next_input' "$HOME_ID_SET_C"
grep -q 'home_id_set_save_dirty_current' "$HOME_ID_SET_C"
grep -q 'home_id_set_status_prompt_close' "$HOME_ID_SET_C"
grep -q 'font.string1\[0\] != '\''\\0'\''' "$HOME_ID_SET_C"

awk '
	/static void home_id_set_add_number/ { in_add = 1 }
	in_add && /home_id_set_prepare_input_replace\(\);/ { saw_prepare = 1 }
	in_add && /home_id_input_dirty\[HomeIdSetClass\.cur_focus\] = true;/ { saw_dirty = 1 }
	in_add && /unsigned char index = HomeIdSetClass.dialog_box/ {
		if (!saw_prepare) {
			exit 1
		}
	}
	/^}/ && in_add { in_add = 0 }
	END { if (!saw_prepare || !saw_dirty) exit 1 }
' "$HOME_ID_SET_C"

awk '
	/static void home_id_set_key_up_up/ { fn = "up"; next }
	/static void home_id_set_key_down_up/ { fn = "down"; next }
	/static void home_id_set_key_ring_up/ { fn = "ring"; next }
	/^}/ { fn = "" }
	fn && /set_home_id_number\(\);/ {
		print "key handler must save through dirty guard instead of direct set_home_id_number in " fn
		exit 1
	}
	fn && /home_id_set_save_dirty_current\(\)/ { saw_guard = 1 }
	END { exit saw_guard ? 0 : 1 }
' "$HOME_ID_SET_C"

grep -q 'home_id_input_dirty\[HomeIdSetClass.save_id_falg\] = false;' "$HOME_ID_SET_C"
grep -q 'memset(home_id_input_dirty, 0, sizeof(home_id_input_dirty));' "$HOME_ID_SET_C"
grep -q 'home_id_input_dirty\[HomeIdSetClass.cur_focus\] = false;' "$HOME_ID_SET_C"

awk '
	/static void home_id_set_key_up_up/ { fn = "up"; next }
	/static void home_id_set_key_down_up/ { fn = "down"; next }
	/static void home_id_set_key_ring_up/ { fn = "ring"; next }
	/^}/ { fn = "" }
	fn && /goto_(prev|next)_home_id_set_focus\(\);/ { after_goto = 1; next }
	fn && after_goto && /clear_current_show_home_id\(\);/ {
		print "focus selection must not clear current room number in " fn
		exit 1
	}
	fn && after_goto && /home_id_set_mark_replace_on_next_input\(\);/ { saw_mark = 1 }
	fn && after_goto && /home_id_set_input_number_display\(\);/ { after_goto = 0 }
	END { exit saw_mark ? 0 : 1 }
' "$HOME_ID_SET_C"

awk '
	/static void home_id_set_status_prompt_close/ { in_close = 1 }
	in_close && /HomeIdSetClass\.set_status = HOME_ID_SET_STATUS_NONE;/ { saw_status_clear = 1 }
	in_close && /home_id_set_status_show_count = 0;/ { saw_count_clear = 1 }
	in_close && /home_id_set_status_redisplay_flag = 0;/ { saw_redisplay_clear = 1 }
	in_close && /home_id_set_status_font_erase\(\);/ { saw_erase = 1 }
	in_close && /^}/ { in_close = 0 }
	END { exit (saw_status_clear && saw_count_clear && saw_redisplay_clear && saw_erase) ? 0 : 1 }
' "$HOME_ID_SET_C"

awk '
	/static void home_id_set_key_up_up/ { fn = "up"; next }
	/static void home_id_set_key_down_up/ { fn = "down"; next }
	/static void home_id_set_key_ring_up/ { fn = "ring"; next }
	/^}/ { fn = "" }
	fn && /home_id_set_status_prompt_close\(\);/ { saw_close = 1; close_before_status = 1 }
	fn && /HomeIdSetClass\.set_status != HOME_ID_SET_STATUS_NONE/ && !close_before_status {
		print "focus keys must close status prompt before deciding whether to return in " fn
		exit 1
	}
	END { exit saw_close ? 0 : 1 }
' "$HOME_ID_SET_C"

awk '
	/static bool set_home_id_number/ { in_set = 1 }
	in_set && /isSameAsOtherInputBoxNumber\(\) == true/ { saw_repeat = 1 }
	in_set && saw_repeat && /home_id_input_dirty\[HomeIdSetClass\.cur_focus\] = false;/ { saw_dirty_clear = 1 }
	in_set && /^}/ { in_set = 0 }
	END { exit (saw_repeat && saw_dirty_clear) ? 0 : 1 }
' "$HOME_ID_SET_C"

awk '
	/void home_id_set_sub_number/ { in_sub = 1 }
	in_sub && /unsigned char delete_index = index - 1;/ { saw_delete_index = 1 }
	in_sub && /show_id\[HomeIdSetClass\.cur_focus\]\[delete_index\] = 0;/ { saw_delete_clear = 1 }
	in_sub && /home_id_set_dialog_box_font_change\(\);/ { saw_font_change = 1 }
	in_sub && /static void home_id_set_input_number_display/ { in_sub = 0 }
	END { exit (saw_delete_index && saw_delete_clear && saw_font_change) ? 0 : 1 }
' "$HOME_ID_SET_C"

awk '
	/static void home_id_set_key_star_up/ { in_star = 1 }
	in_star && /cursor\.index > 0/ { saw_index_guard = 1 }
	in_star && /home_id_set_status_prompt_close\(\);/ { saw_prompt_close = 1 }
	in_star && /home_id_set_sub_number\(\);/ { saw_sub = 1 }
	in_star && /home_id_input_dirty\[HomeIdSetClass\.cur_focus\] = \(HomeIdSetClass\.dialog_box\[HomeIdSetClass\.cur_focus\]->cursor\.index > 0\);/ { saw_dirty = 1 }
	in_star && /home_id_set_input_number_display\(\);/ { saw_display = 1 }
	in_star && /os_layout_goto\(&layout_OutPUT\);/ { saw_exit = 1 }
	in_star && /^}/ { in_star = 0 }
	END { exit (saw_index_guard && saw_prompt_close && saw_sub && saw_dirty && saw_display && saw_exit) ? 0 : 1 }
' "$HOME_ID_SET_C"

echo "home id set focus replace checks passed"
