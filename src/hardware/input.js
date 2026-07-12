/**
 * 智能戒指 / 智能手杖输入
 * Ring: DeviceOrientationEvent — tilt 切换预设, shake 开关
 * Cane: DeviceMotionEvent — tap 静音, swing 下一预设
 */

/** @typedef {'ring-tilt-left' | 'ring-tilt-right' | 'ring-shake' | 'cane-tap' | 'cane-swing'} HardwareAction */

export class SmartInput {
  /** @param {(action: HardwareAction) => void} onAction */
  constructor(onAction) {
    this.onAction = onAction;
    this.enabled = false;
    this.ringBase = null;
    this.lastRingFrame = 0;
    this.lastCaneFrame = 0;
    this.lastBeta = 0;
    this.lastGamma = 0;
    this._onOrientation = this._onOrientation.bind(this);
    this._onMotion = this._onMotion.bind(this);
  }

  async enable() {
    if (this.enabled) return;

    if (typeof DeviceOrientationEvent !== 'undefined' &&
        typeof DeviceOrientationEvent.requestPermission === 'function') {
      const perm = await DeviceOrientationEvent.requestPermission();
      if (perm !== 'granted') return;
    }

    if (typeof DeviceMotionEvent !== 'undefined' &&
        typeof DeviceMotionEvent.requestPermission === 'function') {
      const perm = await DeviceMotionEvent.requestPermission();
      if (perm !== 'granted') {
        // ring may still work
      }
    }

    window.addEventListener('deviceorientation', this._onOrientation);
    window.addEventListener('devicemotion', this._onMotion);
    this.enabled = true;
  }

  disable() {
    window.removeEventListener('deviceorientation', this._onOrientation);
    window.removeEventListener('devicemotion', this._onMotion);
    this.enabled = false;
    this.ringBase = null;
  }

  _onOrientation(e) {
    const now = performance.now();
    if (now - this.lastRingFrame < 1000 / 8) return;
    this.lastRingFrame = now;

    const beta = e.beta ?? 0;
    const gamma = e.gamma ?? 0;

    if (this.ringBase === null) {
      this.ringBase = { beta, gamma };
      this.lastBeta = beta;
      this.lastGamma = gamma;
      return;
    }

    const dBeta = beta - this.lastBeta;
    const dGamma = gamma - this.lastGamma;
    this.lastBeta = beta;
    this.lastGamma = gamma;

    const tiltFromBase = gamma - this.ringBase.gamma;

    if (Math.abs(tiltFromBase) > 5) {
      this.onAction(tiltFromBase > 0 ? 'ring-tilt-right' : 'ring-tilt-left');
      this.ringBase.gamma = gamma;
    }

    const shake = Math.abs(dBeta) + Math.abs(dGamma);
    if (shake > 20) {
      this.onAction('ring-shake');
    }
  }

  _onMotion(e) {
    const now = performance.now();
    if (now - this.lastCaneFrame < 1000 / 6) return;
    this.lastCaneFrame = now;

    const acc = e.accelerationIncludingGravity;
    if (!acc) return;

    const ax = acc.x ?? 0;
    const ay = acc.y ?? 0;
    const az = acc.z ?? 0;
    const mag = Math.sqrt(ax * ax + ay * ay + az * az);
    const spike = Math.abs(mag - 9.8);

    if (spike > 8) {
      this.onAction('cane-tap');
      return;
    }

    const swing = Math.abs(ax) + Math.abs(ay);
    if (swing > 3) {
      this.onAction('cane-swing');
    }
  }
}
