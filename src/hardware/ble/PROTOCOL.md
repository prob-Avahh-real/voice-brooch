# 变声胸针 BLE 硬件协议 v1

## 概述

胸针端负责：MEMS 麦克风采集、物理按键、电量/LED 指示、与手机同步音色状态。  
手机端负责：实时变声处理、UI、戒指/手杖扩展输入。

- 传输：Bluetooth LE 4.2+
- 字节序：小端 (Little-Endian)
- 单帧最大载荷：32 字节
- 预设索引：0–17，与 `src/presets.js` 数组顺序一致

## GATT 服务

| 名称 | UUID | 属性 |
|------|------|------|
| VoiceBrooch Service | `a1b2c001-0000-1000-8000-00805f9b34fb` | Primary |
| Control | `a1b2c002-...` | Write, Write Without Response |
| Status | `a1b2c003-...` | Read, Notify |
| Event | `a1b2c004-...` | Notify |
| Device Info | `a1b2c005-...` | Read |

广播名：`VoiceBrooch-XXXX`（XXXX = 设备后 4 位 MAC）

## 帧格式

```
┌──────┬─────┬─────┬─────┬─────────┬──────┐
│ 0x42 │0x56 │ VER │ CMD │ PAYLOAD │ CRC8 │
│ SOF  │ SOF │  1  │ u8  │ LEN=u8  │      │
└──────┴─────┴─────┴─────┴─────────┴──────┘
```

- **SOF**: 固定 `0x42 0x56` ('VB')
- **VER**: 协议版本，当前 `0x01`
- **CRC8**: 多项式 `0x07`，覆盖 VER+CMD+LEN+PAYLOAD

## 命令 (App → Brooch)

| CMD | 名称 | Payload | 说明 |
|-----|------|---------|------|
| `0x01` | SET_PRESET | `[index: u8]` | 切换音色 0–17 |
| `0x02` | SET_LIVE | `[0\|1]` | 开关变声 |
| `0x03` | SET_MUTE | `[0\|1]` | 静音 |
| `0x04` | SYNC_TIME | `[unix: u32]` | 同步时间戳 |
| `0x05` | REQUEST_STATUS | — | 请求完整状态 |
| `0x06` | SET_GAIN | `[0–100]` | 输入增益 |
| `0x07` | FACTORY_RESET | — | 恢复出厂 |
| `0x08` | PING | — | 心跳 |

## 事件 (Brooch → App)

| EVT | 名称 | Payload | 说明 |
|-----|------|---------|------|
| `0x80` | STATUS | 见下表 | 连接后主动推送 |
| `0x81` | BUTTON | `[id, action]` | 物理按键 |
| `0x82` | PRESET_CHANGED | `[index, source]` | 胸针本地切换 |
| `0x83` | AUDIO_LEVEL | `[peak: u8]` | 电平 0–255，20Hz |
| `0x84` | LOW_BATTERY | `[percent: u8]` | 低电量告警 |
| `0x85` | PONG | — | 心跳响应 |
| `0x8f` | ERROR | `[code: u8]` | 错误码 |

### STATUS Payload (6 bytes)

| 偏移 | 字段 | 类型 | 说明 |
|------|------|------|------|
| 0 | preset_index | u8 | 当前音色 |
| 1 | live | u8 | 0=关 1=开 |
| 2 | muted | u8 | 0=否 1=是 |
| 3 | battery | u8 | 电量 % |
| 4 | gain | u8 | 增益 0–100 |
| 5 | flags | u8 | bit0=充电 bit1=麦克风故障 |

### BUTTON Action

| 值 | 动作 | 默认映射 |
|----|------|----------|
| `0x01` | SHORT | 开关变声 |
| `0x02` | DOUBLE | 下一音色 |
| `0x03` | LONG | 静音 |

## 连接流程

```
App                          Brooch
 │                              │
 │────── Scan / Connect ───────►│
 │◄───── Subscribe Notify ──────│
 │────── REQUEST_STATUS ───────►│
 │◄───── STATUS (notify) ───────│
 │────── SET_PRESET / LIVE ────►│
 │◄───── PRESET_CHANGED ────────│ (若本地按键)
 │────── PING (每 30s) ────────►│
 │◄───── PONG ──────────────────│
```

## 固件参考 (ESP32 / nRF52)

完整参考固件见 [`firmware/`](../firmware/README.md)：

- **ESP32**：ESP-IDF v5 + NimBLE，`firmware/esp32/`
- **nRF52**：nRF Connect SDK (Zephyr)，`firmware/nrf52/`
- **共享层**：`firmware/shared/`（协议编解码、按键、应用状态机）

```c
// 按键短按 → 切换 live，notify STATUS
// 按键双击 → preset_index = (index + 1) % 18，notify PRESET_CHANGED
// 按键长按 → toggle mute，notify STATUS
// 麦克风 ADC 峰值 → 每 50ms notify AUDIO_LEVEL
```

## 错误码

| Code | 含义 |
|------|------|
| `0x01` | 无效预设索引 |
| `0x02` | 麦克风未就绪 |
| `0x03` | CRC 校验失败 |
| `0x04` | 不支持的命令 |

## 版本历史

- **v1** (2026-07): 初始协议，18 种音色，按键/状态/电平通知
