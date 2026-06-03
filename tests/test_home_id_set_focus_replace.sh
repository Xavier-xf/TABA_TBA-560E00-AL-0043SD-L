#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HOME_ID_SET_C="$ROOT_DIR/app_cu_datin/system/layout/layout_home_id_set.c"

grep -q 'home_id_set_prepare_input_replace' "$HOME_ID_SET_C"
grep -q 'home_id_replace_on_next_input' "$HOME_ID_SET_C"
grep -q 'home_id_input_dirty' "$HOME_ID_SET_C"
grep -q 'home_id_set_mark_replace_on_next_input' "$HOME_ID_SET_C"
grep -q 'home_id_set_save_dirty_current' "$HOME_ID_SET_C"
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

echo "home id set focus replace checks passed"
