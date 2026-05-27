# Intercom Heartbeat Deferral Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Temporarily defer hall-machine heartbeat frames during intercom business communication to verify whether room-number reads stop failing.

**Architecture:** Keep the existing heartbeat protocol and brancher firmware unchanged. Wrap the hall-machine public `send_cmd` callback so business sends mark recent bus activity, mark activity again after any valid received frame, and skip heartbeat transmission while the recent-activity window is active.

**Tech Stack:** Embedded C, Anyka Linux APP, shell static regression tests, existing APP build and SDK packaging scripts.

---

### Task 1: Add Regression Test

**Files:**
- Create: `tests/test_intercom_heartbeat_deferral.sh`
- Modify: `tests/test_brancher_heartbeat.sh`

- [x] **Step 1: Write failing test**

Require `INTERCOM_HEARTBEAT_DEFER_MS`, a business-send wrapper, bus-activity marking, and heartbeat bypass of the public callback.

- [x] **Step 2: Verify RED**

Run: `bash tests/test_intercom_heartbeat_deferral.sh`
Expected before implementation: FAIL because heartbeat deferral does not exist.

### Task 2: Add APP-Side Heartbeat Deferral

**Files:**
- Modify: `app_cu_datin/system/src/intercom.h`
- Modify: `app_cu_datin/system/src/intercom.c`

- [x] **Step 1: Add defer window**

Define `INTERCOM_HEARTBEAT_DEFER_MS 500`.

- [x] **Step 2: Mark business communication**

Add `intercom_mark_bus_activity()` and call it from the business send wrapper and after valid receives.

- [x] **Step 3: Defer heartbeat**

In `intercom_heartbeat_check()`, skip heartbeat and move the next send time forward while the bus was active within the defer window.

### Task 3: Verify And Package

**Files:**
- Generated: `app_cu_datin/ANYKA37E.BIN`
- Generated: `AK37E_SDK_V1.03/upgrade/platform/app.sqsh4`
- Generated: `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`

- [ ] **Step 1: Run static tests**

Run: `bash tests/test_intercom_heartbeat_deferral.sh`
Run: `bash tests/test_brancher_heartbeat.sh`
Expected: both PASS.

- [ ] **Step 2: Build APP**

Run: `cd app_cu_datin && ./autobuild.sh -all-sdk`
Expected: exit 0; known sandbox `mkfs.jffs2` messages may appear during SDK packaging.

- [ ] **Step 3: Make APP-only package**

Run: `cd AK37E_SDK_V1.03/upgrade && upgrade_bin_version=$(date +%Y%m%d%H%M%S) ./partition_image.sh app_resource`
Expected: `HALL_MACHINEOS` contains only `app.sqsh4`.
