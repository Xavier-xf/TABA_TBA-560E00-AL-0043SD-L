# Intercom Heartbeat Busy Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将短期“心跳避让业务通信”验证改为正式实现，并同步修正分支器在线判定逻辑。

**Architecture:** 大楼机侧以显式 `bus busy` 窗口管理心跳抑制，使用“最近业务活动 + 恢复延时 + 最大忙超时”三段式控制；分支器侧把“主站在线刷新”从心跳专用扩展为任意有效主站命令。

**Tech Stack:** Embedded C, Anyka Linux APP, brancher MCU C, shell 静态回归脚本，现有 APP 打包脚本。

---

### Task 1: 收紧回归测试

**Files:**
- Modify: `tests/test_intercom_heartbeat_deferral.sh`
- Modify: `tests/test_brancher_heartbeat.sh`

- [ ] **Step 1: 先写新的静态断言**

要求覆盖：
- APP 侧存在恢复延时和忙超时两个常量；
- 心跳检查中存在 busy 超时释放；
- 分支器成功解析任意有效主站命令后刷新在线状态；
- `CMD_HEARTBEAT` 分支仍然不回包。

- [ ] **Step 2: 运行静态测试确认先失败**

Run:
- `bash tests/test_intercom_heartbeat_deferral.sh`
- `bash tests/test_brancher_heartbeat.sh`

Expected:
- 至少一项因新断言缺失而 FAIL。

### Task 2: 实现大楼机 busy-window

**Files:**
- Modify: `app_cu_datin/system/src/intercom.h`
- Modify: `app_cu_datin/system/src/intercom.c`

- [ ] **Step 1: 增加正式常量**
- [ ] **Step 2: 引入 busy 状态、最近活动时间和超时释放逻辑**
- [ ] **Step 3: 将接收处理顺序调整为“先收后判心跳”，减少已到达业务帧前插入心跳的概率**

### Task 3: 实现分支器在线刷新扩展

**Files:**
- Modify: `switch/code/msg_event.c`

- [ ] **Step 1: 将在线刷新抽象为通用函数**
- [ ] **Step 2: 在成功解析任意有效主站命令后刷新在线时间**
- [ ] **Step 3: 保持 `CMD_HEARTBEAT` 不回复总线**

### Task 4: 验证与交付

**Files:**
- Modify: `findings.md`
- Modify: `progress.md`
- Modify: `task_plan.md`
- Modify: `README.md`
- Generated: `AK37E_SDK_V1.03/upgrade/HALL_MACHINEOS`

- [ ] **Step 1: 运行静态测试**
- [ ] **Step 2: 编译 APP**
- [ ] **Step 3: 生成 APP-only 升级包**
- [ ] **Step 4: 更新仓库记录**
