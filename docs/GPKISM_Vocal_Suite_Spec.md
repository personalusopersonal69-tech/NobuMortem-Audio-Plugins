# GPKISM / KAYA Vocal Suite — Engineering Specification

Version: 0.1.0 (design baseline)
Status: specification only; not a compiled plugin.

## Product target
A lightweight native 64-bit VST3 vocal effect for FL Studio on Windows. Intended for a medium-register voice recorded in both clean and raspy styles. The sonic direction is dark, elegant, theatrical gothic Visual Kei, using broad stylistic references only. It must not claim to clone or reproduce a named singer's voice.

## Core principles
1. Preserve the user's recognizable voice; no extreme pitch shifting by default.
2. Natural processing first: moderate gain staging, smooth dynamics, controlled harmonics.
3. No neural network or cloud inference in the audio path.
4. All time-varying parameters must be automatable by the host.
5. Mono and stereo operation; sample rates 44.1/48/88.2/96 kHz where supported.
6. No clipping under normal operation; output trim and peak protection.
7. Low CPU use and no memory allocation, file I/O, or locks in the audio callback.

## Initial parameter model
- Input: Input Trim (-18 to +18 dB), Gate/Expander (off to gentle), HPF (40–140 Hz).
- Character: Darkness (0–100%), Presence (0–100%), Elegance (0–100%), Voice Character (0–100%).
- Formant: Formant Shift (-2 to +2 semitones equivalent as a perceptual control; bypass default). Implementation must use a suitable formant-preserving algorithm, not simply pitch-shift the whole signal.
- Harmonics: Drive (0–30%), Mix (0–100%), Tone (dark to bright).
- Dynamics: Compressor amount, attack, release, threshold, makeup; de-esser amount.
- Space: Reverb mix/size/damping and tempo-synced delay mix/time/feedback.
- Output: Dry/Wet (0–100%), Output Trim (-18 to +6 dB), soft peak protection.
- Performance Mode: Clean / Intense, with host-automatable crossfade or parameter morph.

## Processing order (initial proposal)
Input trim and safety -> high-pass / corrective EQ -> gentle dynamics -> optional formant stage -> parallel harmonic colour -> de-esser -> ambience sends -> dry/wet -> output protection.
The exact order may change after listening tests.

## Clean mode
Lower harmonic drive, gentler compression, more transient preservation, subtle ambience, clear consonants.

## Intense mode
More parallel harmonic density and firmer dynamics, while retaining dry vocal articulation and avoiding hard clipping. Mode changes must be smoothed to prevent clicks.

## Preset: Signature Voice
Starting preset, not a universal setting:
- medium-register male vocal;
- clean/raspy capable;
- dark but intelligible;
- moderate compression and harmonic mix;
- formant stage initially subtle and user-adjustable;
- reverb/delay conservative by default.
Exact values require test recordings and listening; do not hard-code assumptions about the user's microphone or vocal range.

## UI direction
Black matte surfaces, restrained silver borders, deep violet highlights, legible labels, compact meters and knobs. Gothic/Visual Kei mood without reducing readability. Avoid large animations and heavy graphics.

## Validation checklist
- Builds as x64 VST3.
- Loads and scans in FL Studio.
- Processes silence, low-level input, loud input, mono and stereo.
- No denormals, clicks on mode changes, or unexpected NaN/Inf.
- Host automation and preset save/restore work.
- CPU and latency measured at 44.1 and 48 kHz.
- Listening tests with the user's clean and raspy recordings before calling the preset finished.

## Deferred features
- Automatic clean/raspy classification: only after a reliable, low-CPU detector is validated. The first version should expose manual Clean/Intense modes and optional envelope-based automation.
- Automatic key-aware pitch correction belongs in Vocal Tuner Pro, not this plugin.
- Exact singer voice imitation is out of scope.
