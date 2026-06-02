#!/bin/sh
set -eu

lang_h="app_cu_datin/system/layout/language.h"
lang_c="app_cu_datin/system/layout/language.c"
layout_base="app_cu_datin/system/layout/layout_base.c"
layout_system_set="app_cu_datin/system/layout/layout_system_set.c"
layout_calling="app_cu_datin/system/layout/layout_calling.c"
logo_resource="app_cu_datin/system/ui/r/img/taba_icon.png"
expected_logo_sha256="55fa360582c0a4b97b976e7edad70ad0ed45ce8b4cdb290be116e8cb484f6051"

rg -q 'STR_LOGO_TABA' "$lang_h"
rg -q 'STR_LOGO_ELECTRONICS' "$lang_h"

rg -q '\{"TABA", "تابا"\}' "$lang_c"
rg -q '\{"Electronics", "الکترونیک"\}' "$lang_c"

rg -q 'font_str\(STR_LOGO_TABA\)' "$layout_base"
rg -q 'font_str\(STR_LOGO_ELECTRONICS\)' "$layout_base"

rg -q 'text_init\(&taba_text, &taba_text_pos, 36\)' "$layout_base"
rg -q 'text_init\(&electronics_text, &electronics_text_pos, 20\)' "$layout_base"
rg -q 'language_get\(\) == language_persian' "$layout_base"
rg -Fq 'position taba_text_pos = {{120, 219}, {112, 44}}' "$layout_base"
rg -Fq 'position electronics_text_pos = {{210, 226}, {150, 30}}' "$layout_base"
rg -Fq 'taba_text_pos = (position){{236, 219}, {86, 44}}' "$layout_base"
rg -Fq 'electronics_text_pos = (position){{146, 226}, {126, 30}}' "$layout_base"

rg -q 'static void system_set_logo_refresh\(void\)' "$layout_system_set"
rg -Fq 'position pos = {{0, 193}, {480, 79}};' "$layout_system_set"
rg -q 'gui_erase\(&pos, 0x00\);' "$layout_system_set"
rg -q 'taba_btn_display\(\);' "$layout_system_set"
rg -q 'font_file_reload\(\);' "$layout_system_set"
rg -q 'SystemSetClass.widget_show.font\(\);' "$layout_system_set"
rg -q 'system_set_logo_refresh\(\);' "$layout_system_set"

rg -q 'static void calling_text_area_clear\(void\)' "$layout_calling"
rg -q 'calling_text_area_clear\(\);' "$layout_calling"
rg -q 'sprintf\(string, "%s NO.%d \.\.\.", \(char \*\)font_str\(STR_CALLING_USER\), number\);' "$layout_calling"
rg -q 'text_display\(&no_answer, font_str\(STR_CALLING_NO_ANSWER\)\);' "$layout_calling"

actual_logo_sha256="$(sha256sum "$logo_resource" | awk '{print $1}')"
[ "$actual_logo_sha256" = "$expected_logo_sha256" ]

echo "bottom logo language checks passed"
