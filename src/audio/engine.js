import * as Tone from 'tone';
import { EFFECT_PARAMS } from '../presets/effects.js';

export class VoiceEngine {
  constructor() {
    /** @type {Tone.UserMedia | null} */
    this.mic = null;
    /** @type {Tone.PitchShift | null} */
    this.pitchShift = null;
    /** @type {Tone.Distortion | null} */
    this.distortion = null;
    /** @type {Tone.Filter | null} */
    this.filter = null;
    /** @type {Tone.Reverb | null} */
    this.reverb = null;
    /** @type {Tone.Analyser | null} */
    this.analyser = null;
    /** @type {Tone.Gain | null} */
    this.gain = null;
    this.active = false;
    this.muted = false;
    this.lastOutputGain = 1;
  }

  async start() {
    if (this.active) return;

    await Tone.start();

    this.mic = new Tone.UserMedia();
    this.pitchShift = new Tone.PitchShift({ pitch: 0, windowSize: 0.1 });
    this.distortion = new Tone.Distortion({ distortion: 0, wet: 0 });
    this.filter = new Tone.Filter({ type: 'lowpass', frequency: 20000, Q: 0.5 });
    this.reverb = new Tone.Reverb({ decay: 2.5, wet: 0 });
    this.analyser = new Tone.Analyser('waveform', 256);
    this.gain = new Tone.Gain(1);

    this.mic.connect(this.pitchShift);
    this.pitchShift.connect(this.distortion);
    this.distortion.connect(this.filter);
    this.filter.connect(this.reverb);
    this.reverb.connect(this.analyser);
    this.analyser.connect(this.gain);
    this.gain.toDestination();

    await this.reverb.generate();
    await this.mic.open();
    this.active = true;
  }

  stop() {
    if (!this.active) return;
    this.mic?.close();
    this.mic?.dispose();
    this.pitchShift?.dispose();
    this.distortion?.dispose();
    this.filter?.dispose();
    this.reverb?.dispose();
    this.analyser?.dispose();
    this.gain?.dispose();
    this.mic = null;
    this.pitchShift = null;
    this.distortion = null;
    this.filter = null;
    this.reverb = null;
    this.analyser = null;
    this.gain = null;
    this.active = false;
  }

  /** @param {import('../presets.js').VoicePreset} preset */
  applyPreset(preset) {
    if (!this.pitchShift || !this.distortion || !this.filter || !this.reverb || !this.gain) return;

    const fx = EFFECT_PARAMS[preset.effect] ?? EFFECT_PARAMS.natural;

    this.pitchShift.pitch = preset.pitch + fx.pitch;
    this.distortion.distortion = fx.distortion;
    this.distortion.wet.value = fx.distWet;
    this.filter.type = fx.filterType;
    this.filter.frequency.value = fx.filterFreq;
    this.filter.Q.value = fx.filterQ;
    this.reverb.wet.value = fx.reverbWet;
    this.lastOutputGain = fx.gain;
    this.gain.gain.value = this.muted ? 0 : fx.gain;
  }

  setMuted(muted) {
    this.muted = muted;
    if (this.gain) {
      this.gain.gain.rampTo(muted ? 0 : this.lastOutputGain, 0.05);
    }
  }

  /** @returns {Uint8Array | null} */
  getWaveform() {
    if (!this.analyser) return null;
    return this.analyser.getValue();
  }

  getLevel() {
    const data = this.getWaveform();
    if (!data || data.length === 0) return 0;
    let sum = 0;
    for (let i = 0; i < data.length; i++) {
      const v = (data[i] - 128) / 128;
      sum += v * v;
    }
    return Math.sqrt(sum / data.length);
  }
}
