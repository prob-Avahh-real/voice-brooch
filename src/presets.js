/** @typedef {import('./presets/effects.js').EffectProfile} EffectProfile */

/** @typedef {{ index: number, id: string, name: string, emoji: string, pitch: number, color: string, effect: EffectProfile, category: 'basic' | 'character' | 'scene' }} VoicePreset */

/** @type {VoicePreset[]} */
export const PRESETS = [
  // 基础 6
  { index: 0, id: 'natural', name: '原声', emoji: '🎙️', pitch: 0, color: '#a78bfa', effect: 'natural', category: 'basic' },
  { index: 1, id: 'deep', name: '低沉', emoji: '🎩', pitch: -8, color: '#6366f1', effect: 'deep', category: 'basic' },
  { index: 2, id: 'bright', name: '清脆', emoji: '✨', pitch: 5, color: '#f472b6', effect: 'bright', category: 'basic' },
  { index: 3, id: 'robot', name: '机器人', emoji: '🤖', pitch: -3, color: '#22d3ee', effect: 'robot', category: 'basic' },
  { index: 4, id: 'fairy', name: '精灵', emoji: '🧚', pitch: 9, color: '#34d399', effect: 'bright', category: 'basic' },
  { index: 5, id: 'radio', name: '广播', emoji: '📻', pitch: -2, color: '#fbbf24', effect: 'radio', category: 'basic' },
  // 角色 6
  { index: 6, id: 'uncle', name: '大叔', emoji: '👨', pitch: -6, color: '#78716c', effect: 'deep', category: 'character' },
  { index: 7, id: 'loli', name: '萝莉', emoji: '🎀', pitch: 8, color: '#fb7185', effect: 'bright', category: 'character' },
  { index: 8, id: 'mature', name: '御姐', emoji: '💋', pitch: -2, color: '#e879f9', effect: 'narrator', category: 'character' },
  { index: 9, id: 'child', name: '童声', emoji: '🧒', pitch: 7, color: '#fcd34d', effect: 'bright', category: 'character' },
  { index: 10, id: 'elder', name: '老者', emoji: '👴', pitch: -7, color: '#a8a29e', effect: 'vintage', category: 'character' },
  { index: 11, id: 'monster', name: '怪兽', emoji: '👹', pitch: -11, color: '#dc2626', effect: 'monster', category: 'character' },
  // 场景 6
  { index: 12, id: 'phone', name: '电话', emoji: '☎️', pitch: 0, color: '#38bdf8', effect: 'telephone', category: 'scene' },
  { index: 13, id: 'megaphone', name: '扩音', emoji: '📢', pitch: 2, color: '#f97316', effect: 'megaphone', category: 'scene' },
  { index: 14, id: 'underwater', name: '水下', emoji: '🫧', pitch: -1, color: '#0891b2', effect: 'underwater', category: 'scene' },
  { index: 15, id: 'hall', name: '大厅', emoji: '🏛️', pitch: 0, color: '#818cf8', effect: 'hall', category: 'scene' },
  { index: 16, id: 'whisper', name: '悄悄话', emoji: '🤫', pitch: 3, color: '#c4b5fd', effect: 'whisper', category: 'scene' },
  { index: 17, id: 'alien', name: '外星人', emoji: '👽', pitch: 4, color: '#4ade80', effect: 'alien', category: 'scene' }
];

export const PRESET_COUNT = PRESETS.length;

const CATEGORY_LABELS = {
  basic: '基础',
  character: '角色',
  scene: '场景'
};

export function getPreset(id) {
  return PRESETS.find((p) => p.id === id) ?? PRESETS[0];
}

export function getPresetByIndex(index) {
  return PRESETS.find((p) => p.index === index) ?? PRESETS[0];
}

export function nextPresetId(currentId) {
  const idx = PRESETS.findIndex((p) => p.id === currentId);
  return PRESETS[(idx + 1) % PRESETS.length].id;
}

export function prevPresetId(currentId) {
  const idx = PRESETS.findIndex((p) => p.id === currentId);
  return PRESETS[(idx - 1 + PRESETS.length) % PRESETS.length].id;
}

export function getCategoryLabel(category) {
  return CATEGORY_LABELS[category] ?? category;
}

export function groupPresetsByCategory() {
  /** @type {Record<string, VoicePreset[]>} */
  const groups = {};
  for (const p of PRESETS) {
    if (!groups[p.category]) groups[p.category] = [];
    groups[p.category].push(p);
  }
  return groups;
}
