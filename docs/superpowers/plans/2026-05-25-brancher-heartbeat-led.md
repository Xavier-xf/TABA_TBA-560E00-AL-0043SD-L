# Brancher Heartbeat LED Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a scalable hall-machine heartbeat so branchers blink their power LED when upstream communication is disconnected.

**Architecture:** Add one broadcast protocol command shared by hall machine and brancher. The hall machine sends `CMD_HEARTBEAT` periodically from the existing intercom UART task; each brancher refreshes a local last-seen timestamp and drives `POWER_LED` from `cpu_count` without sending any reply.

**Tech Stack:** Embedded C, Anyka Linux APP, MG82F6D64 brancher MCU code, shell static regression test, existing APP build and SDK packaging scripts.

---

### Task 1: Static Regression Test

**Files:**
- Create: `tests/test_brancher_heartbeat.sh`

- [x] **Step 1: Add failing test**

Check that both sides define the same `CMD_HEARTBEAT`, the APP sends it, the brancher handles it without `send_can_cmd_encode()`, and LED blinking does not use `TIMER0`.

- [x] **Step 2: Verify RED**

Run: `bash tests/test_brancher_heartbeat.sh`
Expected before implementation: FAIL with missing `CMD_HEARTBEAT`.

### Task 2: Hall-Machine Heartbeat Sender

**Files:**
- Modify: `app_cu_datin/system/src/intercom.h`
- Modify: `app_cu_datin/system/src/intercom.c`

- [x] **Step 1: Add protocol constant**

Define `CMD_HEARTBEAT 0xBA` and `INTERCOM_HEARTBEAT_INTERVAL_MS 1000`.

- [x] **Step 2: Send heartbeat periodically**

Add `intercom_heartbeat_check()` and call it from `intercom_event_detect()` before receive processing.

### Task 3: Brancher Heartbeat Receiver And LED State

**Files:**
- Modify: `switch/code/include/kevin_function.h`
- Modify: `switch/code/msg_event.c`

- [x] **Step 1: Add matching command**

Define `CMD_HEARTBEAT 0xBA` in the brancher protocol header.

- [x] **Step 2: Handle heartbeat without reply**

Add `case CMD_HEARTBEAT:` to refresh local heartbeat state only.

- [x] **Step 3: Add LED offline state machine**

Use `cpu_count` to enter blink mode after `5000 ms` without heartbeat and toggle `POWER_LED` every `500 ms`.

### Task 4: Verification And Packaging

**Files:**
- Generated: `app_cu_datin/ANYKA37E.BIN`
- Generated: `AK37E_SDK_V1.03/upgrade/platform/app.sqsh4`
- Generated: `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`

- [x] **Step 1: Run static test**

Run: `bash tests/test_brancher_heartbeat.sh`
Expected: PASS.

- [x] **Step 2: Build hall-machine APP**

Run: `cd app_cu_datin && ./autobuild.sh -all-sdk`
Expected: exit 0; known sandbox `mkfs.jffs2` messages may appear during SDK packaging.

- [x] **Step 3: Make APP-only package**

Run: `cd AK37E_SDK_V1.03/upgrade && upgrade_bin_version=$(date +%Y%m%d%H%M%S) ./partition_image.sh app_resource`
Expected: `HALL_MACHINEOS` contains only `app.sqsh4`.
