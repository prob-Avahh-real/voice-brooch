# 变声胸针参考固件

ESP32 与 nRF52 共用 `shared/` 协议层，实现与 App 端 `src/hardware/ble/protocol.js` 一致的 BLE GATT 服务。

## 目录

```
firmware/
├── shared/          # 跨平台：协议、按键、应用状态机
├── esp32/           # ESP-IDF + NimBLE
└── nrf52/           # nRF Connect SDK (Zephyr)
```

## 硬件引脚（参考）

| 功能 | ESP32 | nRF52840 |
|------|-------|----------|
| 按键 (低有效) | GPIO0 | P0.11 |
| MEMS 麦克风 ADC | GPIO34 (ADC1_CH6) | P0.03 (AIN1) |
| 电量 ADC | GPIO35 (ADC1_CH7) | P0.04 (AIN2) |
| 状态 LED | GPIO2 | P0.13 |
| 充电检测 | GPIO36 | P0.12 |

## 按键映射

- **短按** → 开关变声 (live toggle)
- **双击** → 下一音色 (0–17 循环)
- **长按** → 静音切换

## ESP32 编译

```bash
cd firmware/esp32
idf.py set-target esp32
idf.py build flash monitor
```

依赖：ESP-IDF v5.x，启用 NimBLE。

## nRF52 编译

```bash
cd firmware/nrf52
west build -b nrf52840dk_nrf52840
west flash
```

依赖：nRF Connect SDK v2.x，Zephyr Bluetooth。

## 广播名

`VoiceBrooch-XXXX`（XXXX = MAC 后两字节十六进制）

## 与 App 联调

1. 手机 App 点「连接胸针」
2. 选择 `VoiceBrooch-*` 设备
3. 订阅 Status / Event 通知
4. App 发送 REQUEST_STATUS → 固件回复 STATUS
5. 按键 / App 切音色应双向同步
