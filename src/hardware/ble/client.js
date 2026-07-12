import {
  SERVICE_UUID,
  CHAR_CONTROL_UUID,
  CHAR_STATUS_UUID,
  CHAR_EVENT_UUID,
  CHAR_DEVICE_INFO_UUID,
  DEVICE_NAME_PREFIX,
  decodeFrame,
  parseEvent,
  cmdSetPreset,
  cmdSetLive,
  cmdSetMute,
  cmdRequestStatus,
  cmdPing,
  BUTTON_ACTION
} from './protocol.js';

/**
 * Web Bluetooth 胸针客户端
 * @typedef {(event: import('./protocol.js').BroochEvent) => void} BroochEventHandler
 */

export class BroochBleClient {
  /** @param {BroochEventHandler} onEvent */
  constructor(onEvent) {
    this.onEvent = onEvent;
    /** @type {BluetoothDevice | null} */
    this.device = null;
    /** @type {BluetoothRemoteGATTServer | null} */
    this.server = null;
    /** @type {BluetoothRemoteGATTCharacteristic | null} */
    this.controlChar = null;
    this.pingTimer = null;
    this.connected = false;
  }

  get isSupported() {
    return typeof navigator !== 'undefined' && 'bluetooth' in navigator;
  }

  async connect() {
    if (!this.isSupported) {
      throw new Error('当前浏览器不支持 Web Bluetooth');
    }

    this.device = await navigator.bluetooth.requestDevice({
      filters: [{ namePrefix: DEVICE_NAME_PREFIX }],
      optionalServices: [SERVICE_UUID]
    });

    this.device.addEventListener('gattserverdisconnected', () => this._handleDisconnect());

    this.server = await this.device.gatt.connect();
    const service = await this.server.getPrimaryService(SERVICE_UUID);

    this.controlChar = await service.getCharacteristic(CHAR_CONTROL_UUID);

    const statusChar = await service.getCharacteristic(CHAR_STATUS_UUID);
    await statusChar.startNotifications();
    statusChar.addEventListener('characteristicvaluechanged', (e) => this._onNotify(e));

    const eventChar = await service.getCharacteristic(CHAR_EVENT_UUID);
    await eventChar.startNotifications();
    eventChar.addEventListener('characteristicvaluechanged', (e) => this._onNotify(e));

    try {
      const infoChar = await service.getCharacteristic(CHAR_DEVICE_INFO_UUID);
      const info = await infoChar.readValue();
      this.firmwareVersion = new TextDecoder().decode(info);
    } catch {
      this.firmwareVersion = 'unknown';
    }

    await this.write(cmdRequestStatus());
    this.connected = true;
    this._startPing();
    return this.device.name ?? DEVICE_NAME_PREFIX;
  }

  async disconnect() {
    this._stopPing();
    if (this.device?.gatt?.connected) {
      this.device.gatt.disconnect();
    }
    this._handleDisconnect();
  }

  /** @param {number} index */
  async setPreset(index) {
    await this.write(cmdSetPreset(index));
  }

  /** @param {boolean} live */
  async setLive(live) {
    await this.write(cmdSetLive(live));
  }

  /** @param {boolean} muted */
  async setMute(muted) {
    await this.write(cmdSetMute(muted));
  }

  /** @param {Uint8Array} data */
  async write(data) {
    if (!this.controlChar) throw new Error('胸针未连接');
    await this.controlChar.writeValue(data);
  }

  /** @param {Event} e */
  _onNotify(e) {
    const target = /** @type {BluetoothRemoteGATTCharacteristic} */ (e.target);
    const frame = decodeFrame(target.value);
    if (!frame) return;
    const event = parseEvent(frame);
    if (event) this.onEvent(event);
  }

  _handleDisconnect() {
    this.connected = false;
    this.server = null;
    this.controlChar = null;
    this._stopPing();
    this.onEvent({ type: 'disconnected' });
  }

  _startPing() {
    this._stopPing();
    this.pingTimer = setInterval(() => {
      if (this.connected) this.write(cmdPing()).catch(() => {});
    }, 30000);
  }

  _stopPing() {
    if (this.pingTimer) {
      clearInterval(this.pingTimer);
      this.pingTimer = null;
    }
  }
}

/** 将胸针按键事件映射为应用动作 */
export function mapButtonToAction(action) {
  switch (action) {
    case BUTTON_ACTION.SHORT: return 'toggle-live';
    case BUTTON_ACTION.DOUBLE: return 'next-preset';
    case BUTTON_ACTION.LONG: return 'toggle-mute';
    default: return null;
  }
}
