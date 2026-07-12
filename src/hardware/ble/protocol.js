/**
 * 变声胸针 BLE 协议 v1
 * 帧格式: SOF(2) + VER(1) + CMD(1) + LEN(1) + PAYLOAD(n) + CRC8(1)
 */

export const PROTOCOL_VERSION = 1;
export const SOF = 0x5642; // 'VB' little-endian on wire: 0x42, 0x56

export const SERVICE_UUID = 'a1b2c001-0000-1000-8000-00805f9b34fb';
export const CHAR_CONTROL_UUID = 'a1b2c002-0000-1000-8000-00805f9b34fb';
export const CHAR_STATUS_UUID = 'a1b2c003-0000-1000-8000-00805f9b34fb';
export const CHAR_EVENT_UUID = 'a1b2c004-0000-1000-8000-00805f9b34fb';
export const CHAR_DEVICE_INFO_UUID = 'a1b2c005-0000-1000-8000-00805f9b34fb';

export const DEVICE_NAME_PREFIX = 'VoiceBrooch';

/** App → Brooch */
export const CMD = {
  SET_PRESET: 0x01,
  SET_LIVE: 0x02,
  SET_MUTE: 0x03,
  SYNC_TIME: 0x04,
  REQUEST_STATUS: 0x05,
  SET_GAIN: 0x06,
  FACTORY_RESET: 0x07,
  PING: 0x08
};

/** Brooch → App */
export const EVT = {
  STATUS: 0x80,
  BUTTON: 0x81,
  PRESET_CHANGED: 0x82,
  AUDIO_LEVEL: 0x83,
  LOW_BATTERY: 0x84,
  PONG: 0x85,
  ERROR: 0x8f
};

export const BUTTON_ACTION = {
  SHORT: 0x01,
  DOUBLE: 0x02,
  LONG: 0x03
};

export const BUTTON_ID = {
  MAIN: 0x01,
  SIDE: 0x02
};

/** @param {number[]} data */
export function crc8(data) {
  let crc = 0;
  for (const b of data) {
    crc ^= b;
    for (let i = 0; i < 8; i++) {
      crc = (crc & 0x80) ? ((crc << 1) ^ 0x07) : (crc << 1);
      crc &= 0xff;
    }
  }
  return crc;
}

/**
 * @param {number} cmd
 * @param {Uint8Array | number[]} [payload]
 */
export function encodeFrame(cmd, payload = []) {
  const body = [PROTOCOL_VERSION, cmd, payload.length, ...payload];
  const crc = crc8(body);
  const frame = new Uint8Array(2 + body.length + 1);
  frame[0] = 0x42;
  frame[1] = 0x56;
  frame.set(body, 2);
  frame[frame.length - 1] = crc;
  return frame;
}

/**
 * @param {DataView | ArrayBuffer | Uint8Array} buffer
 * @returns {{ cmd: number, payload: Uint8Array } | null}
 */
export function decodeFrame(buffer) {
  const view = buffer instanceof DataView
    ? buffer
    : new DataView(buffer instanceof Uint8Array ? buffer.buffer : buffer);

  if (view.byteLength < 6) return null;
  if (view.getUint8(0) !== 0x42 || view.getUint8(1) !== 0x56) return null;

  const ver = view.getUint8(2);
  if (ver !== PROTOCOL_VERSION) return null;

  const cmd = view.getUint8(3);
  const len = view.getUint8(4);
  if (view.byteLength < 5 + len + 1) return null;

  const body = new Uint8Array(len + 3);
  for (let i = 0; i < body.length; i++) body[i] = view.getUint8(2 + i);

  const expectedCrc = view.getUint8(5 + len);
  if (crc8(body) !== expectedCrc) return null;

  const payload = new Uint8Array(len);
  for (let i = 0; i < len; i++) payload[i] = view.getUint8(5 + i);

  return { cmd, payload };
}

/** @param {number} index 0-17 */
export function cmdSetPreset(index) {
  return encodeFrame(CMD.SET_PRESET, [index & 0xff]);
}

/** @param {boolean} enabled */
export function cmdSetLive(enabled) {
  return encodeFrame(CMD.SET_LIVE, [enabled ? 1 : 0]);
}

/** @param {boolean} muted */
export function cmdSetMute(muted) {
  return encodeFrame(CMD.SET_MUTE, [muted ? 1 : 0]);
}

/** @param {number} gain 0-100 */
export function cmdSetGain(gain) {
  return encodeFrame(CMD.SET_GAIN, [Math.max(0, Math.min(100, gain))]);
}

export function cmdRequestStatus() {
  return encodeFrame(CMD.REQUEST_STATUS);
}

export function cmdPing() {
  return encodeFrame(CMD.PING);
}

/**
 * Parse STATUS event payload (6 bytes)
 * @param {Uint8Array} payload
 */
export function parseStatusPayload(payload) {
  if (payload.length < 6) return null;
  return {
    presetIndex: payload[0],
    live: payload[1] === 1,
    muted: payload[2] === 1,
    battery: payload[3],
    gain: payload[4],
    flags: payload[5]
  };
}

/**
 * Parse BUTTON event payload (2 bytes)
 * @param {Uint8Array} payload
 */
export function parseButtonPayload(payload) {
  if (payload.length < 2) return null;
  return { buttonId: payload[0], action: payload[1] };
}

/**
 * @typedef {{
 *   type: 'status',
 *   presetIndex: number,
 *   live: boolean,
 *   muted: boolean,
 *   battery: number,
 *   gain: number,
 *   flags: number
 * } | {
 *   type: 'button',
 *   buttonId: number,
 *   action: number
 * } | {
 *   type: 'preset_changed',
 *   presetIndex: number,
 *   source: number
 * } | {
 *   type: 'audio_level',
 *   level: number
 * } | {
 *   type: 'low_battery',
 *   battery: number
 * } | {
 *   type: 'pong'
 * } | {
 *   type: 'error',
 *   code: number
 * }} BroochEvent
 */

/**
 * @param {{ cmd: number, payload: Uint8Array }} frame
 * @returns {BroochEvent | null}
 */
export function parseEvent(frame) {
  const { cmd, payload } = frame;

  switch (cmd) {
    case EVT.STATUS: {
      const s = parseStatusPayload(payload);
      if (!s) return null;
      return { type: 'status', ...s };
    }
    case EVT.BUTTON: {
      const b = parseButtonPayload(payload);
      if (!b) return null;
      return { type: 'button', ...b };
    }
    case EVT.PRESET_CHANGED:
      return { type: 'preset_changed', presetIndex: payload[0] ?? 0, source: payload[1] ?? 0 };
    case EVT.AUDIO_LEVEL:
      return { type: 'audio_level', level: payload[0] ?? 0 };
    case EVT.LOW_BATTERY:
      return { type: 'low_battery', battery: payload[0] ?? 0 };
    case EVT.PONG:
      return { type: 'pong' };
    case EVT.ERROR:
      return { type: 'error', code: payload[0] ?? 0xff };
    default:
      return null;
  }
}
