import { PRESETS, groupPresetsByCategory, getCategoryLabel } from '../presets.js';

/** @typedef {import('../presets.js').VoicePreset} VoicePreset */

/**
 * @param {HTMLElement} root
 * @param {{
 *   onStart: () => void,
 *   onStop: () => void,
 *   onToggleMute: () => void,
 *   onSelectPreset: (id: string) => void,
 *   onEnableHardware: () => void,
 *   onConnectBrooch: () => void,
 * }} handlers
 */
export function renderApp(root, handlers) {
  const groups = groupPresetsByCategory();
  const categoryOrder = ['basic', 'character', 'scene'];

  const presetSections = categoryOrder.map((cat) => {
    const items = groups[cat] ?? [];
    if (items.length === 0) return '';
    return `
      <div class="preset-section">
        <span class="preset-section-label">${getCategoryLabel(cat)}</span>
        <div class="preset-row">
          ${items.map((p) => `
            <button class="preset-chip ${p.id === PRESETS[0].id ? 'selected' : ''}" data-id="${p.id}" style="--chip-color: ${p.color}">
              <span class="chip-emoji">${p.emoji}</span>
              <span>${p.name}</span>
            </button>
          `).join('')}
        </div>
      </div>
    `;
  }).join('');

  root.innerHTML = `
    <header class="header">
      <h1>变声胸针</h1>
      <div class="status-pill">
        <span class="status-dot off" id="status-dot"></span>
        <span id="status-text">未启动</span>
      </div>
    </header>
    <p class="subtitle">
      <span id="brooch-status">胸针未连接</span>
      <span id="battery-status" class="battery-status hidden"></span>
    </p>

    <section class="brooch-stage">
      <div class="brooch" id="brooch" style="--brooch-color: ${PRESETS[0].color}; --brooch-glow: ${PRESETS[0].color}66">
        <span class="brooch-emoji" id="brooch-emoji">${PRESETS[0].emoji}</span>
        <span class="brooch-name" id="brooch-name">${PRESETS[0].name}</span>
        <div class="brooch-pin"></div>
      </div>
      <div class="waveform-wrap">
        <canvas id="waveform" width="320" height="48"></canvas>
      </div>
    </section>

    <div class="preset-scroll" id="preset-scroll">
      ${presetSections}
    </div>

    <div class="controls">
      <button class="btn btn-primary" id="btn-live">开启变声</button>
      <button class="btn btn-secondary" id="btn-mute" disabled>静音</button>
      <button class="btn btn-secondary" id="btn-brooch">连接胸针</button>
      <button class="btn btn-secondary" id="btn-hardware">戒指 / 手杖</button>
    </div>

    <p class="hint">
      胸针按键：短按开关 · 双击切音色 · 长按静音<br>
      戒指倾斜切换 · 手杖敲击静音
    </p>
  `;

  root.querySelector('#btn-live')?.addEventListener('click', () => handlers.onStart());
  root.querySelector('#btn-mute')?.addEventListener('click', () => handlers.onToggleMute());
  root.querySelector('#btn-brooch')?.addEventListener('click', () => handlers.onConnectBrooch());
  root.querySelector('#btn-hardware')?.addEventListener('click', () => handlers.onEnableHardware());

  root.querySelectorAll('.preset-chip').forEach((chip) => {
    chip.addEventListener('click', () => {
      const id = chip.getAttribute('data-id');
      if (id) handlers.onSelectPreset(id);
    });
  });
}

/** @param {VoicePreset} preset */
export function updateBrooch(preset) {
  const brooch = document.getElementById('brooch');
  const emoji = document.getElementById('brooch-emoji');
  const name = document.getElementById('brooch-name');
  if (brooch) {
    brooch.style.setProperty('--brooch-color', preset.color);
    brooch.style.setProperty('--brooch-glow', `${preset.color}66`);
  }
  if (emoji) emoji.textContent = preset.emoji;
  if (name) name.textContent = preset.name;

  document.querySelectorAll('.preset-chip').forEach((chip) => {
    chip.classList.toggle('selected', chip.getAttribute('data-id') === preset.id);
  });

  const selected = document.querySelector('.preset-chip.selected');
  selected?.scrollIntoView({ inline: 'center', block: 'nearest', behavior: 'smooth' });
}

export function setLiveState(live, muted) {
  const dot = document.getElementById('status-dot');
  const text = document.getElementById('status-text');
  const brooch = document.getElementById('brooch');
  const btnLive = document.getElementById('btn-live');
  const btnMute = document.getElementById('btn-mute');

  if (dot) dot.classList.toggle('off', !live);
  if (text) text.textContent = live ? (muted ? '已静音' : '变声中') : '未启动';
  if (brooch) {
    brooch.classList.toggle('active', live && !muted);
    brooch.classList.toggle('muted', muted);
  }
  if (btnLive) btnLive.textContent = live ? '停止变声' : '开启变声';
  if (btnMute) btnMute.disabled = !live;
}

export function setBroochConnection(connected, name, battery) {
  const status = document.getElementById('brooch-status');
  const batteryEl = document.getElementById('battery-status');
  const btn = document.getElementById('btn-brooch');

  if (status) {
    status.textContent = connected ? `胸针已连接 · ${name ?? 'VoiceBrooch'}` : '胸针未连接';
  }
  if (batteryEl) {
    if (connected && battery != null) {
      batteryEl.textContent = `🔋 ${battery}%`;
      batteryEl.classList.remove('hidden');
    } else {
      batteryEl.classList.add('hidden');
    }
  }
  if (btn) btn.textContent = connected ? '断开胸针' : '连接胸针';
}

export function drawWaveform(engine) {
  const canvas = /** @type {HTMLCanvasElement | null} */ (document.getElementById('waveform'));
  if (!canvas) return;
  const ctx = canvas.getContext('2d');
  if (!ctx) return;

  const data = engine.getWaveform();
  const w = canvas.width;
  const h = canvas.height;

  ctx.clearRect(0, 0, w, h);
  ctx.fillStyle = 'rgba(192, 132, 252, 0.08)';
  ctx.fillRect(0, 0, w, h);

  if (!data) return;

  ctx.lineWidth = 2;
  ctx.strokeStyle = '#c084fc';
  ctx.beginPath();
  const slice = w / data.length;
  for (let i = 0; i < data.length; i++) {
    const y = (data[i] / 255) * h;
    const x = i * slice;
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  }
  ctx.stroke();
}
