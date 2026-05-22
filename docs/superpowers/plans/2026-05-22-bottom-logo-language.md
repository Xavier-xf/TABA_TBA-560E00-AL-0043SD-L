# Bottom Logo Language Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Draw the bottom TABA logo text from the language table so it switches between English and Persian.

**Architecture:** Keep the pure PNG as the background image rendered by `taba_btn_display()`. Add two language strings for the larger `TABA` text and the smaller `Electronics` text, then draw them over `ROM_R_IMG_TABA_ICON_PNG` using the existing `text` API and `font_str()`.

**Tech Stack:** Embedded C, existing `ui_api` text/icon rendering, shell static regression test.

---

### Task 1: Add Regression Test

**Files:**
- Create: `tests/test_bottom_logo_language.sh`

- [ ] **Step 1: Write the failing test**

Create a shell test that checks:
- `language.h` defines `STR_LOGO_TABA` and `STR_LOGO_ELECTRONICS`.
- `language.c` maps them to `TABA` / `تابا` and `Electronics` / `الکترونیک`.
- `layout_base.c` draws both strings via `font_str()`.
- TABA uses a larger font than Electronics.
- `app_cu_datin/system/ui/r/img/taba_icon.png` matches the new pure logo image from `logonew.png`.

- [ ] **Step 2: Run test to verify it fails**

Run: `sh tests/test_bottom_logo_language.sh`
Expected: FAIL because the logo strings and drawing code do not exist yet.

### Task 2: Implement Dynamic Bottom Logo Text

**Files:**
- Modify: `app_cu_datin/system/layout/language.h`
- Modify: `app_cu_datin/system/layout/language.c`
- Modify: `app_cu_datin/system/layout/layout_base.c`

- [ ] **Step 1: Add language IDs**

Add `STR_LOGO_TABA` and `STR_LOGO_ELECTRONICS` near `STR_TOTAL`.

- [ ] **Step 2: Add translations**

Add:
- `{"TABA", "تابا"}`
- `{"Electronics", "الکترونیک"}`

- [ ] **Step 3: Draw text over the pure logo**

In `taba_btn_display()`, after `icon_display(&star)`, draw:
- Large yellow TABA text, centered around the previous logo word position.
- Smaller yellow Electronics text to the right, matching the old image layout.

- [ ] **Step 4: Run static regression test**

Run: `sh tests/test_bottom_logo_language.sh`
Expected: PASS.

### Task 3: Build and Package

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
Run: `head -c 160 AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS | strings`
Expected: size around APP-only package size and header includes `# File Partion: app.sqsh4`.
