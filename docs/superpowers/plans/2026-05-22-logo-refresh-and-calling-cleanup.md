# Logo Refresh And Calling Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the bottom logo refresh immediately on language change, tighten Persian logo spacing, and clear the calling text area cleanly for four-digit English room numbers.

**Architecture:** Keep the current layout structure and resource usage. Fix the issues by updating coordinates in `taba_btn_display()`, redrawing the bottom logo from the language-setting handlers, and introducing a shared clear-area helper in `layout_calling.c`.

**Tech Stack:** Embedded C, existing UI text/icon APIs, shell static regression test, existing SDK build/package scripts.

---

### Task 1: Extend Regression Test

**Files:**
- Modify: `tests/test_bottom_logo_language.sh`

- [ ] **Step 1: Write the failing test**

Extend the shell test to require:
- tighter Persian logo coordinates;
- language toggle handlers to clear bottom area and call `taba_btn_display()`;
- a shared calling text clear helper used by ring/guard/no-answer rendering.

- [ ] **Step 2: Run test to verify it fails**

Run: `sh tests/test_bottom_logo_language.sh`
Expected: FAIL because the current code does not yet contain the new coordinates, redraw path, or shared clear helper.

### Task 2: Fix Bottom Logo Layout And Live Refresh

**Files:**
- Modify: `app_cu_datin/system/layout/layout_base.c`
- Modify: `app_cu_datin/system/layout/layout_system_set.c`

- [ ] **Step 1: Tighten Persian logo layout**

Update the Persian branch in `taba_btn_display()` to use closer coordinates for `تابا` and `الکترونیک`.

- [ ] **Step 2: Redraw logo after language toggle**

In both `system_set_key_up_up()` and `system_set_key_down_up()` language-layer branches:
- keep `language_set(...)`;
- keep `font_file_reload()`;
- keep `SystemSetClass.widget_show.font()`;
- add bottom-area erase and `taba_btn_display()`.

- [ ] **Step 3: Run test**

Run: `sh tests/test_bottom_logo_language.sh`
Expected: still FAIL because calling-area cleanup is not implemented yet.

### Task 3: Fix Calling Text Residue

**Files:**
- Modify: `app_cu_datin/system/layout/layout_calling.c`

- [ ] **Step 1: Add shared clear helper**

Add one helper that erases a unified calling text rectangle.

- [ ] **Step 2: Use helper in all three display paths**

Call the helper before:
- `calling_ring_font_display()`
- `calling_guard_font_display()`
- `no_answer_font_display()`

- [ ] **Step 3: Run test to verify it passes**

Run: `sh tests/test_bottom_logo_language.sh`
Expected: PASS.

### Task 4: Build And Package

**Files:**
- Generated: `app_cu_datin/ANYKA37E.BIN`
- Generated: `AK37E_SDK_V1.03/upgrade/platform/app.sqsh4`
- Generated: `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`

- [ ] **Step 1: Build APP**

Run: `cd app_cu_datin && ./autobuild.sh -all-sdk`
Expected: exit 0.

- [ ] **Step 2: Make APP-only package**

Run: `cd AK37E_SDK_V1.03/upgrade && upgrade_bin_version=$(date +%Y%m%d%H%M%S) ./partition_image.sh app_resource`
Expected: `HALL_MACHINEOS` header contains only `app.sqsh4`.

- [ ] **Step 3: Verify package**

Run: `stat -c '%n %s bytes' AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS AK37E_SDK_V1.03/upgrade/platform/app.sqsh4`
Run: `head -c 180 AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS | strings`
Expected: APP-only size and header with `# File Parttion: app.sqsh4`.
