# 变声胸针参考固件

ESP32 与 nRF52 共用 `shared/` 协议层，实现与 App 端 `src/hardware/ble/protocol.js` 一致的 BLE GATT 服务。

## 目录

```
firmware/
├── shared/              # 跨平台：协议、按键、应用状态机
├── esp32/               # ESP-IDF + NimBLE (+ platformio.ini)
├── nrf52/               # nRF Connect SDK (Zephyr)
├── boards/              # 胸针 PCB devicetree overlay
├── test/                # 协议单元测试 (JS + C)
├── platformio.ini       # native 测试 + nRF52840 一键编译
├── Makefile             # 快捷命令
└── build-all.sh         # 全目标构建脚本
```

## 硬件引脚（胸针 PCB v1）

| 功能 | ESP32 | nRF52840 (overlay) |
|------|-------|---------------------|
| 按键 (低有效) | GPIO0 | P0.11 (`sw0`) |
| MEMS 麦克风 ADC | GPIO34 | P0.03 / AIN1 |
| 电量 ADC | GPIO35 | P0.04 / AIN2 |
| 状态 LED | GPIO2 | P0.13 (`led0`) |
| 充电检测 | GPIO36 | P0.12 (`charge0`) |

设备树 overlay：[`boards/voice_brooch.overlay`](boards/voice_brooch.overlay)

## 协议单元测试

```bash
cd firmware

# JS（与 App protocol.js 交叉验证）
node --test test/js/protocol.test.mjs

# C 原生测试（PlatformIO）
pio run -e native_test
.pio/build/native_test/program

# 或使用 gcc 直接编译
make test-native

# 全部测试
make test
```

黄金向量见 [`test/vectors/protocol_vectors.json`](test/vectors/protocol_vectors.json)。

## PlatformIO 一键编译

```bash
cd firmware

# 协议测试
pio run -e native_test

# nRF52840 + 胸针 PCB overlay
pio run -e nrf52840_brooch
pio run -t upload -e nrf52840_brooch

# ESP32（在 esp32 子目录）
cd esp32 && pio run -e esp32_brooch

# 全部（需安装 PlatformIO + ESP-IDF/Zephyr 工具链）
./build-all.sh
# 或
make all
```

### 环境说明

| PlatformIO Env | 目标 |
|----------------|------|
| `native_test` | Host 协议/按键单元测试 |
| `nrf52840_brooch` | nRF52840 DK + PCB overlay |
| `nrf52840_dk` | nRF52840 DK 无 overlay |
| `esp32_brooch` | ESP32-WROOM (`firmware/esp32/`) |
| `esp32_brooch_s3` | ESP32-S3 |
| `esp32_brooch_c3` | ESP32-C3 |

## ESP-IDF 原生编译

```bash
cd firmware/esp32
idf.py set-target esp32
idf.py build flash monitor
```

## nRF Connect SDK (west)

```bash
cd firmware/nrf52
west build -b nrf52840dk_nrf52840 -- -DEXTRA_DTC_OVERLAY_FILE=../boards/voice_brooch.overlay
west flash
```

## 按键映射

- **短按** → 开关变声 (live toggle)
- **双击** → 下一音色 (0–17 循环)
- **长按** → 静音切换

## 广播名

`VoiceBrooch-XXXX`（XXXX = MAC 后两字节十六进制）

## 与 App 联调

1. 烧录固件 → 广播名 `VoiceBrooch-*`
2. App 点「连接胸针」→ 选择设备
3. 订阅 Status / Event 通知
4. App 发送 REQUEST_STATUS → 固件回复 STATUS
5. 按键 / App 切音色应双向同步
