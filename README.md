# NOBU MORTEM — DARK AUDIO COLLECTION

Free, lightweight native audio tools for FL Studio on Windows 10/11 x64.

## Collection

- **GPKISM Vocal Suite 0.3.2** — dark, elegant vocal tone shaping, dynamics, controlled harmonic saturation, input/output gain staging and dry/wet blend.
- **Guttural Forge** — planned industrial/electro-metal guttural enhancement, designed to preserve the original vocal identity.
- **Vocal Tuner Pro** — planned pitch detection and correction with user-selected key/scale, correction speed, and natural/creative modes.

## GPKISM Vocal Suite controls

- Darkness, Presence, Compression, Drive and Harmonic Mix.
- Dry / Wet, Input Trim and Output Trim.
- Clean / Intense performance modes.
- **Nobu Natural**, **GPKISM 55%**, **Kaya Inspired**, and **Custom** character settings.
- Selecting a character recalls its actual parameter values so the knob positions, host parameters and DSP remain aligned. Moving a knob changes the character selector to **Custom**.

The character presets are tonal starting points, not voice cloning or a claim of matching another singer's identity.

## Visual identity

Gothic Visual Kei and dark romantic styling: black surfaces, silver text, deep violet accents, separate readable parameter labels, and visible numeric values with units.

## Technical goals

- VST3, 64-bit, Windows x64, FL Studio compatibility.
- Efficient native DSP; no cloud service or neural model running locally.
- Mono/stereo processing, input/output trims and dry/wet control.
- Smoothed parameter changes, controlled saturation and a soft output ceiling near -0.17 dBFS.
- Build validation through GitHub Actions.

## Release status

Version 0.3.2 is the next Windows x64 validation build. CI compilation does not verify the visual layout, preset interaction or final sound quality; those still require a real FL Studio test before calling the release production-ready.
