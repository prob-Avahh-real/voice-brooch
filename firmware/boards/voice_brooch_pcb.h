#ifndef BOARD_PCB_H
#define BOARD_PCB_H

/*
 * Voice Brooch PCB v1 pin reference (matches boards/voice_brooch.overlay)
 *
 * Function         nRF52840 pin    Notes
 * ---------------- --------------  ---------------------------
 * Main button      P0.11           Active low, internal pull-up
 * Status LED       P0.13           Active high
 * Charge detect    P0.12           High when USB charging
 * MEMS mic ADC     P0.03 / AIN1    12-bit SAADC channel 1
 * Battery ADC      P0.04 / AIN2    Via 2:1 divider, channel 2
 */

#define VB_PCB_BUTTON_PIN   11
#define VB_PCB_LED_PIN      13
#define VB_PCB_CHARGE_PIN   12
#define VB_PCB_MIC_AIN      NRF_SAADC_AIN1
#define VB_PCB_BATT_AIN     NRF_SAADC_AIN2

#endif
