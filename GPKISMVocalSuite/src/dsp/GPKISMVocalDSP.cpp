#include "GPKISMVocalDSP.h"

namespace nobu::dsp {

float GPKISMVocalDSP::dbToGain(float db) noexcept {
  return std::pow(10.0f, db * 0.05f);
}

float GPKISMVocalDSP::clamp01(float x) noexcept {
  return std::max(0.0f, std::min(1.0f, x));
}

void GPKISMVocalDSP::prepare(double sampleRate) noexcept {
  mSampleRate = std::max(8000.0, sampleRate);
  setParams(VocalParams{});
  reset();
}

void GPKISMVocalDSP::reset() noexcept {
  for (auto& s : mState) s = ChannelState{};
}

void GPKISMVocalDSP::setParams(const VocalParams& p) noexcept {
  const float hp = std::max(25.0f, std::min(250.0f, p.highPassHz));
  const float x = std::exp(-2.0f * 3.14159265358979323846f * hp /
                           static_cast<float>(mSampleRate));
  mHpCoeff = std::max(0.0f, std::min(0.9999f, x));
  mInputGain = dbToGain(std::max(-18.0f, std::min(18.0f, p.inputDb)));
  mOutputGain = dbToGain(std::max(-18.0f, std::min(6.0f, p.outputDb)));
  mCompAmount = clamp01(p.compression);
  mDrive = clamp01(p.drive);
  mHarmonicMix = clamp01(p.harmonicMix);
  mDryWet = clamp01(p.dryWet);
  mDarkness = clamp01(p.darkness);
  mPresence = clamp01(p.presence);
  mModeDrive = (p.mode == PerformanceMode::Intense) ? 1.45f : 1.0f;
}

float GPKISMVocalDSP::processSample(float input, ChannelState& s) noexcept {
  const float dry = input;
  float x = input * mInputGain;

  // Gentle one-pole high-pass filter.
  const float hp = mHpCoeff * (s.hpY + x - s.hpX);
  s.hpX = x;
  s.hpY = hp;
  x = hp;

  // Broad tonal shaping: restrained darkening and presence emphasis.
  // These one-pole components are intentionally inexpensive.
  const float darkCoeff = 0.035f + 0.12f * mDarkness;
  const float low = darkCoeff * x + (1.0f - darkCoeff) * s.hpY;
  const float presence = x - low;
  x = x - 0.12f * mDarkness * low + 0.10f * mPresence * presence;

  // Smooth peak-envelope compressor with a soft-knee style gain reduction.
  const float absX = std::fabs(x);
  const float attack = std::exp(-1.0f / (0.008f * static_cast<float>(mSampleRate)));
  const float release = std::exp(-1.0f / (0.085f * static_cast<float>(mSampleRate)));
  const float coeff = (absX > s.envelope) ? attack : release;
  s.envelope = coeff * s.envelope + (1.0f - coeff) * absX;
  const float threshold = 0.42f;
  const float over = std::max(0.0f, s.envelope - threshold);
  const float reduction = 1.0f / (1.0f + over * (1.0f + 5.0f * mCompAmount));
  x *= reduction;

  // Parallel, bounded harmonic colour. Dry signal remains present to preserve identity.
  const float drive = 1.0f + 7.0f * mDrive * mModeDrive;
  const float saturated = std::tanh(x * drive) / std::max(1.0f, std::tanh(drive));
  const float coloured = x * (1.0f - mHarmonicMix) + saturated * mHarmonicMix;
  x = x * (1.0f - mHarmonicMix) + coloured * mHarmonicMix;

  const float wet = x * mOutputGain;
  return dry * (1.0f - mDryWet) + wet * mDryWet;
}

void GPKISMVocalDSP::process(float* const* channels, int channelCount,
                             int sampleCount) noexcept {
  if (!channels || sampleCount <= 0) return;
  const int count = std::min(2, std::max(0, channelCount));
  for (int ch = 0; ch < count; ++ch) {
    if (!channels[ch]) continue;
    ChannelState& state = mState[ch];
    for (int i = 0; i < sampleCount; ++i) {
      float y = processSample(channels[ch][i], state);
      // Defensive finite-value guard and conservative output ceiling.
      if (!std::isfinite(y)) y = 0.0f;
      channels[ch][i] = std::max(-0.98f, std::min(0.98f, y));
    }
  }
}

} // namespace nobu::dsp
