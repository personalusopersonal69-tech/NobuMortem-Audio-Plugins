#include "GPKISMVocalDSP.h"

namespace nobu::dsp {
float GPKISMVocalDSP::dbToGain(float db) noexcept { return std::pow(10.0f, db * 0.05f); }
float GPKISMVocalDSP::clamp01(float x) noexcept { return std::max(0.0f, std::min(1.0f, x)); }

void GPKISMVocalDSP::prepare(double sampleRate) noexcept {
  mSampleRate = std::max(8000.0, sampleRate);
  setParams(VocalParams{});
  reset();
}
void GPKISMVocalDSP::reset() noexcept {
  for (auto& s : mState) s = ChannelState{};
}
void GPKISMVocalDSP::setParams(const VocalParams& p) noexcept {
  const float hp = std::max(35.0f, std::min(180.0f, p.highPassHz));
  mHpCoeff = std::exp(-2.0f * 3.14159265358979323846f * hp / static_cast<float>(mSampleRate));
  mInputGain = dbToGain(std::max(-12.0f, std::min(12.0f, p.inputDb)));
  mOutputGain = dbToGain(std::max(-18.0f, std::min(3.0f, p.outputDb)));
  mCompAmount = clamp01(p.compression);
  mDrive = clamp01(p.drive);
  mHarmonicMix = std::min(0.30f, clamp01(p.harmonicMix));
  mDryWet = clamp01(p.dryWet);
  mDarkness = clamp01(p.darkness);
  mPresence = clamp01(p.presence);
  mModeDrive = (p.mode == PerformanceMode::Intense) ? 1.15f : 1.0f;
}
float GPKISMVocalDSP::processSample(float input, ChannelState& s) noexcept {
  const float dry = input;
  float x = input * mInputGain;

  // Gentle high-pass: remove rumble without thinning the voice.
  const float hp = mHpCoeff * (s.hpY + x - s.hpX);
  s.hpX = x; s.hpY = hp; x = hp;

  // Broad, low-cost tonal contour. Darkness softens low-mid weight;
  // Presence adds a restrained upper-mid detail component.
  const float toneCoeff = 1.0f - std::exp(-2.0f * 3.14159265f * 260.0f / static_cast<float>(mSampleRate));
  s.toneY += toneCoeff * (x - s.toneY);
  const float lowMid = s.toneY;
  const float detail = x - lowMid;
  x += (-0.10f * mDarkness * lowMid) + (0.18f * mPresence * detail);

  // Soft-knee style downward compression with smoothed envelope.
  const float absX = std::fabs(x);
  const float attack = std::exp(-1.0f / (0.012f * static_cast<float>(mSampleRate)));
  const float release = std::exp(-1.0f / (0.110f * static_cast<float>(mSampleRate)));
  const float coeff = absX > s.envelope ? attack : release;
  s.envelope = coeff * s.envelope + (1.0f - coeff) * absX;
  const float threshold = 0.62f - 0.18f * mCompAmount;
  const float over = std::max(0.0f, s.envelope - threshold);
  const float ratio = 1.0f + 2.5f * mCompAmount;
  const float targetGain = over > 0.0f ? (threshold + over / ratio) / std::max(s.envelope, 1.0e-6f) : 1.0f;
  x *= targetGain;

  // Subtle parallel harmonic colour; no aggressive clipping by default.
  const float drive = 1.0f + 1.8f * mDrive * mModeDrive;
  const float saturated = std::tanh(x * drive) / drive;
  const float colour = std::min(0.30f, mHarmonicMix * (0.35f + 0.65f * mDrive));
  x = x * (1.0f - colour) + saturated * colour;

  const float wet = x * mOutputGain;
  float mixed = dry * (1.0f - mDryWet) + wet * mDryWet;
  // Transparent safety ceiling, only catches exceptional peaks.
  if (std::fabs(mixed) > 0.97f)
    mixed = std::copysign(0.97f + 0.025f * std::tanh((std::fabs(mixed) - 0.97f) / 0.025f), mixed);
  return std::isfinite(mixed) ? mixed : 0.0f;
}
void GPKISMVocalDSP::process(float* const* channels, int channelCount, int sampleCount) noexcept {
  if (!channels || sampleCount <= 0) return;
  const int count = std::min(2, std::max(0, channelCount));
  for (int ch = 0; ch < count; ++ch) {
    if (!channels[ch]) continue;
    ChannelState& state = mState[ch];
    for (int i = 0; i < sampleCount; ++i) channels[ch][i] = processSample(channels[ch][i], state);
  }
}
} // namespace nobu::dsp
