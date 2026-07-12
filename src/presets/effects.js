/** @typedef {'natural' | 'deep' | 'bright' | 'robot' | 'radio' | 'telephone' | 'megaphone' | 'underwater' | 'hall' | 'vintage' | 'narrator' | 'whisper' | 'monster' | 'alien'} EffectProfile */

/** @typedef {{ pitch: number, distortion: number, distWet: number, filterType: BiquadFilterType, filterFreq: number, filterQ: number, gain: number, reverbWet: number }} EffectParams */

/** @type {Record<EffectProfile, EffectParams>} */
export const EFFECT_PARAMS = {
  natural: { pitch: 0, distortion: 0, distWet: 0, filterType: 'lowpass', filterFreq: 20000, filterQ: 0.5, gain: 1, reverbWet: 0 },
  deep: { pitch: 0, distortion: 0, distWet: 0, filterType: 'lowpass', filterFreq: 900, filterQ: 0.6, gain: 1, reverbWet: 0 },
  bright: { pitch: 0, distortion: 0, distWet: 0, filterType: 'highpass', filterFreq: 180, filterQ: 0.8, gain: 1, reverbWet: 0 },
  robot: { pitch: 0, distortion: 0.65, distWet: 0.45, filterType: 'lowpass', filterFreq: 3200, filterQ: 2, gain: 1, reverbWet: 0 },
  radio: { pitch: 0, distortion: 0.15, distWet: 0.2, filterType: 'bandpass', filterFreq: 1800, filterQ: 1.2, gain: 1, reverbWet: 0 },
  telephone: { pitch: 0, distortion: 0.08, distWet: 0.15, filterType: 'bandpass', filterFreq: 1200, filterQ: 2.5, gain: 0.95, reverbWet: 0 },
  megaphone: { pitch: 0, distortion: 0.35, distWet: 0.3, filterType: 'bandpass', filterFreq: 2400, filterQ: 1.5, gain: 1.1, reverbWet: 0.05 },
  underwater: { pitch: 0, distortion: 0, distWet: 0, filterType: 'lowpass', filterFreq: 650, filterQ: 1.2, gain: 0.85, reverbWet: 0.15 },
  hall: { pitch: 0, distortion: 0, distWet: 0, filterType: 'lowpass', filterFreq: 12000, filterQ: 0.5, gain: 1, reverbWet: 0.45 },
  vintage: { pitch: 0, distortion: 0.25, distWet: 0.25, filterType: 'bandpass', filterFreq: 900, filterQ: 0.9, gain: 0.9, reverbWet: 0.08 },
  narrator: { pitch: 0, distortion: 0, distWet: 0, filterType: 'bandpass', filterFreq: 2200, filterQ: 0.7, gain: 1.05, reverbWet: 0.1 },
  whisper: { pitch: 0, distortion: 0, distWet: 0, filterType: 'highpass', filterFreq: 420, filterQ: 1.1, gain: 0.55, reverbWet: 0.12 },
  monster: { pitch: 0, distortion: 0.85, distWet: 0.55, filterType: 'lowpass', filterFreq: 700, filterQ: 2.2, gain: 1.15, reverbWet: 0.1 },
  alien: { pitch: 0, distortion: 0.4, distWet: 0.35, filterType: 'highpass', filterFreq: 600, filterQ: 1.8, gain: 1, reverbWet: 0.2 }
};
