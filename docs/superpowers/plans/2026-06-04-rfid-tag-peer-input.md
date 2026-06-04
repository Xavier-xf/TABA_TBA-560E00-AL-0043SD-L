# RFID TAG Peer Input Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make RFID `TAG:` input a peer of `UNIT:`, let TAG confirmation resolve the saved card's room automatically, and prevent TAG text from covering the focus arrow.

**Architecture:** Keep the existing card-management screen and card-data format. Add a small TAG confirmation state in `layout_card_manage.c`; `TAG_FOCUS` accepts numbers and swipe-fill in the main layer, then confirmation globally looks up the saved card, fills `UNIT`, refreshes the room card count in `SAVE`, and switches `ERASE` to single-card mode. Whole-room erase remains driven by confirmed `UNIT`; TAG-driven erase is driven by the resolved saved card index, not by re-validating the UNIT field.

**Tech Stack:** Embedded C UI code, existing shell static regression tests, existing APP build and SDK packaging flow.

---

### Task 1: Static Regression Tests

**Files:**
- Modify: `tests/test_card_prompt_interaction.sh`

- [ ] Add checks that TAG display width no longer reaches the arrow at `x=430`.
- [ ] Add checks that TAG numeric input and `*` delete do not require `CARD_MANAGE_MAIN_LAYER_CONFIRM`.
- [ ] Add checks that `card_manage_tag_confirmed` is cleared on TAG changes and required by `ERASE:`.
- [ ] Run `bash tests/test_card_prompt_interaction.sh` and verify it fails before implementation.

### Task 2: RFID TAG Interaction

**Files:**
- Modify: `app_cu_datin/system/layout/layout_card_manage.c`

- [ ] Add `card_manage_tag_confirmed`.
- [ ] Clear confirmation whenever TAG is typed, deleted, cleared, or set from a swipe.
- [x] On `TAG_FOCUS` confirm, globally validate the TAG against saved card data, fill the card's room into `UNIT`, refresh the room card count in `SAVE`, and set `card_manage_tag_confirmed = true`.
- [x] Update `ERASE_FOCUS` so TAG mode deletes by the resolved saved card's room base; only whole-room mode re-validates the UNIT field.
- [ ] Shrink TAG display and result-clear areas to avoid the focus arrow.
- [ ] Update `card_manage_update_tag_fill_request()` to follow `TAG_FOCUS` rather than the confirm layer.

### Task 2.1: TAG Lookup Correction

**Files:**
- Modify: `app_cu_datin/system/layout/layout_card_manage.c`
- Modify: `tests/test_card_prompt_interaction.sh`

- [x] Add `card_manage_find_saved_card_by_tag()` for global saved-card lookup using the display TAG value.
- [x] Add `card_manage_find_saved_card_by_raw_id()` so swipe-fill only succeeds for already-saved cards; unknown cards show `Card number error`.
- [x] Add `card_manage_set_unit_by_home_id()` so TAG confirmation populates the `UNIT` box using the saved card's room.
- [x] Widen `ERASE ROOM` drawing area to prevent truncation.

### Task 3: Verification And Packaging

**Files:**
- No source edits expected beyond Task 2.

- [x] Run `bash tests/test_card_prompt_interaction.sh`.
- [x] Run `bash tests/test_home_id_set_focus_replace.sh`.
- [x] Run `bash tests/test_intercom_heartbeat_deferral.sh`.
- [x] Run `bash tests/test_brancher_heartbeat.sh`.
- [x] Run `git -c core.whitespace=cr-at-eol diff --check -- ...` for touched files.
- [x] Run `cd app_cu_datin && make`.
- [x] Run `cd app_cu_datin && ./autobuild.sh -all-sdk`.
- [x] Rebuild SDK `config.jffs2/data.jffs2/tuya.jffs2` if sandbox blocks `mkfs.jffs2`.
- [x] Run APP-only package with existing `partition_image.sh app_resource`.
