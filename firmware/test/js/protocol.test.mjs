import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import test from 'node:test';

import {
  CMD,
  EVT,
  crc8,
  encodeFrame,
  decodeFrame,
  cmdSetPreset,
  cmdRequestStatus,
  cmdPing,
  parseStatusPayload,
  parseEvent
} from '../../../src/hardware/ble/protocol.js';

const root = dirname(fileURLToPath(import.meta.url));
const vectors = JSON.parse(
  readFileSync(join(root, '../vectors/protocol_vectors.json'), 'utf8')
);

function hexToBytes(hex) {
  const out = new Uint8Array(hex.length / 2);
  for (let i = 0; i < out.length; i++) {
    out[i] = parseInt(hex.slice(i * 2, i * 2 + 2), 16);
  }
  return out;
}

function bytesToHex(bytes) {
  return [...bytes].map((b) => b.toString(16).padStart(2, '0')).join('');
}

test('crc8 matches firmware vectors', () => {
  for (const sample of vectors.crc8_samples) {
    assert.equal(crc8(sample.data), sample.crc, JSON.stringify(sample.data));
  }
});

test('encodeFrame matches golden hex vectors', () => {
  for (const frame of vectors.frames) {
    const encoded = encodeFrame(frame.cmd, frame.payload);
    assert.equal(bytesToHex(encoded), frame.hex, frame.name);
  }
});

test('decodeFrame round-trips all vectors', () => {
  for (const frame of vectors.frames) {
    const bytes = hexToBytes(frame.hex);
    const decoded = decodeFrame(bytes);
    assert.ok(decoded, frame.name);
    assert.equal(decoded.cmd, frame.cmd);
    assert.deepEqual([...decoded.payload], frame.payload);
  }
});

test('cmd helpers produce valid frames', () => {
  const preset = cmdSetPreset(17);
  assert.equal(decodeFrame(preset)?.cmd, CMD.SET_PRESET);
  assert.equal(decodeFrame(preset)?.payload[0], 17);

  const status = cmdRequestStatus();
  assert.equal(decodeFrame(status)?.cmd, CMD.REQUEST_STATUS);

  const pong = decodeFrame(cmdPing());
  assert.equal(pong?.cmd, CMD.PING);
});

test('parseStatusPayload and parseEvent', () => {
  const statusPl = new Uint8Array([3, 1, 0, 85, 80, 0]);
  const status = parseStatusPayload(statusPl);
  assert.equal(status.presetIndex, 3);
  assert.equal(status.live, true);
  assert.equal(status.battery, 85);
  assert.equal(status.gain, 80);

  const evt = parseEvent({ cmd: EVT.STATUS, payload: statusPl });
  assert.equal(evt.type, 'status');
  assert.equal(evt.muted, false);
});

test('decodeFrame rejects bad crc', () => {
  const bytes = hexToBytes(vectors.frames[0].hex);
  bytes[bytes.length - 1] ^= 0xff;
  assert.equal(decodeFrame(bytes), null);
});
