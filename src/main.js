import { VoiceEngine } from './audio/engine.js';
import { SmartInput } from './hardware/input.js';
import { BroochBleClient, mapButtonToAction } from './hardware/ble/client.js';
import { initNative, hapticLight, hapticSuccess } from './mobile/bridge.js';
import { getPreset, getPresetByIndex, nextPresetId, prevPresetId } from './presets.js';
import { renderApp, updateBrooch, setLiveState, setBroochConnection, drawWaveform } from './ui/app.js';

const engine = new VoiceEngine();
let currentPresetId = 'natural';
let hardwareEnabled = false;
let broochConnected = false;
let animId = 0;

/** @type {SmartInput | null} */
let smartInput = null;
/** @type {BroochBleClient | null} */
let broochClient = null;

function selectPreset(id, syncBle = true) {
  currentPresetId = id;
  const preset = getPreset(id);
  engine.applyPreset(preset);
  updateBrooch(preset);
  hapticLight();

  if (syncBle && broochClient?.connected) {
    broochClient.setPreset(preset.index).catch(() => {});
  }
}

async function toggleLive() {
  if (engine.active) {
    engine.stop();
    cancelAnimationFrame(animId);
    setLiveState(false, false);
    if (broochClient?.connected) broochClient.setLive(false).catch(() => {});
    hapticLight();
    return;
  }

  try {
    await engine.start();
    engine.applyPreset(getPreset(currentPresetId));
    setLiveState(true, false);
    if (broochClient?.connected) broochClient.setLive(true).catch(() => {});
    hapticSuccess();
    loopWaveform();
  } catch (err) {
    console.error(err);
    alert('无法访问麦克风，请检查权限设置');
  }
}

function toggleMute() {
  if (!engine.active) return;
  engine.setMuted(!engine.muted);
  setLiveState(true, engine.muted);
  if (broochClient?.connected) broochClient.setMute(engine.muted).catch(() => {});
  hapticLight();
}

async function enableHardware() {
  if (!smartInput) {
    smartInput = new SmartInput(handleHardwareAction);
  }
  await smartInput.enable();
  hardwareEnabled = true;
  const btn = document.getElementById('btn-hardware');
  if (btn) btn.textContent = '戒指 / 手杖已连接';
  hapticSuccess();
}

async function toggleBrooch() {
  if (!broochClient) {
    broochClient = new BroochBleClient(handleBroochEvent);
  }

  if (broochConnected) {
    await broochClient.disconnect();
    return;
  }

  try {
    const name = await broochClient.connect();
    broochConnected = true;
    setBroochConnection(true, name);
    hapticSuccess();
  } catch (err) {
    console.error(err);
    if (err instanceof Error && !err.message.includes('cancelled')) {
      alert(err.message || '胸针连接失败');
    }
  }
}

/** @param {import('./hardware/input.js').HardwareAction} action */
function handleHardwareAction(action) {
  if (!engine.active && action !== 'ring-shake' && action !== 'cane-tap') return;

  switch (action) {
    case 'ring-tilt-left':
      selectPreset(prevPresetId(currentPresetId));
      break;
    case 'ring-tilt-right':
      selectPreset(nextPresetId(currentPresetId));
      break;
    case 'ring-shake':
      toggleLive();
      break;
    case 'cane-tap':
      if (engine.active) toggleMute();
      else toggleLive();
      break;
    case 'cane-swing':
      selectPreset(nextPresetId(currentPresetId));
      break;
  }
}

/** @param {import('./hardware/ble/protocol.js').BroochEvent | { type: 'disconnected' }} event */
function handleBroochEvent(event) {
  if (event.type === 'disconnected') {
    broochConnected = false;
    setBroochConnection(false);
    return;
  }

  switch (event.type) {
    case 'status':
      selectPreset(getPresetByIndex(event.presetIndex).id, false);
      if (event.live !== engine.active) {
        if (event.live) toggleLive();
        else if (engine.active) toggleLive();
      }
      if (engine.active && event.muted !== engine.muted) toggleMute();
      setBroochConnection(true, broochClient?.device?.name, event.battery);
      break;
    case 'preset_changed':
      selectPreset(getPresetByIndex(event.presetIndex).id, false);
      break;
    case 'button': {
      const action = mapButtonToAction(event.action);
      if (action === 'toggle-live') toggleLive();
      else if (action === 'next-preset') selectPreset(nextPresetId(currentPresetId));
      else if (action === 'toggle-mute') toggleMute();
      break;
    }
    case 'low_battery':
      setBroochConnection(true, broochClient?.device?.name, event.battery);
      break;
  }
}

function loopWaveform() {
  drawWaveform(engine);
  animId = requestAnimationFrame(loopWaveform);
}

async function main() {
  await initNative();

  const root = document.getElementById('app');
  if (!root) return;

  renderApp(root, {
    onStart: toggleLive,
    onStop: toggleLive,
    onToggleMute: toggleMute,
    onSelectPreset: (id) => selectPreset(id),
    onEnableHardware: enableHardware,
    onConnectBrooch: toggleBrooch
  });

  updateBrooch(getPreset(currentPresetId));
}

main();
