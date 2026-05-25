# Brancher Heartbeat LED Design

## Goal
When the hall machine and a brancher lose CAN/UART-bus communication, the brancher should change its normally steady power LED to blinking. The hall machine must support many branchers without creating response traffic from every brancher.

## Selected Approach
Use方案A：主站广播心跳，分支器只接收不回复。

The hall machine sends a low-frequency broadcast heartbeat frame. Every brancher accepts this frame as proof that the upstream communication line is alive. Branchers do not send ACK or heartbeat responses, so bus traffic stays constant even with dozens or hundreds of branchers.

## Protocol
- Add `CMD_HEARTBEAT = 0xBA` to both protocol headers.
- Heartbeat carries no data bytes.
- Hall machine sends heartbeat every `1000 ms`.
- Brancher ignores heartbeat at the business-command level and only refreshes local online time.

## Brancher LED Behavior
- Normal online state: `POWER_LED = LOW_LEVEL`, matching the previous steady-on initialization.
- Offline threshold: if no heartbeat is received for `5000 ms`, enter offline indication.
- Offline indication: toggle `POWER_LED` every `500 ms`.
- Recovery: when the next heartbeat is received, immediately restore steady-on LED.

## Timer Choice
Do not reuse the existing `led_blink()` function because it uses `TIMER0`. `TIMER0` is already used by unlock debounce in `switch/code/msg_event.c`, and that debounce path calls `kill_timer(TIMER0)`. The brancher LED state machine therefore uses `cpu_count` instead of allocating a timer slot.

## Validation
- Static regression script checks protocol consistency, no brancher heartbeat reply, and no `TIMER0` LED blink usage.
- Hall-machine APP build verifies the Linux-side changes compile.
- Brancher MCU still needs Keil build and hardware validation.
