let Haptics = null;
let StatusBar = null;
let SplashScreen = null;
let isNative = false;

export async function initNative() {
  try {
    const cap = await import('@capacitor/core');
    if (!cap.Capacitor.isNativePlatform()) return;

    isNative = true;
    try { Haptics = (await import('@capacitor/haptics')).Haptics; } catch { /* web */ }
    try { StatusBar = (await import('@capacitor/status-bar')).StatusBar; } catch { /* web */ }
    try { SplashScreen = (await import('@capacitor/splash-screen')).SplashScreen; } catch { /* web */ }

    if (StatusBar) {
      await StatusBar.setStyle({ style: 'DARK' });
      await StatusBar.setBackgroundColor({ color: '#1a1028' });
    }
    if (SplashScreen) await SplashScreen.hide();
  } catch {
    /* browser */
  }
}

export async function hapticLight() {
  if (!isNative || !Haptics) return;
  try { await Haptics.impact({ style: 'LIGHT' }); } catch { /* silent */ }
}

export async function hapticSuccess() {
  if (!isNative || !Haptics) return;
  try { await Haptics.notification({ type: 'SUCCESS' }); } catch { /* silent */ }
}

export function getIsNative() {
  return isNative;
}
