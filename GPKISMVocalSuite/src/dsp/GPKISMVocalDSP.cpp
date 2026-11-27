#include "GPKISMVocalDSP.h"

namespace nobu::dsp {

float GPKISMVocalDSP::dbToGain(float db) noexcept
{
  return std::pow(10.0f, db * 0.05f);
}

float GPKISMVocalDSP::clamp01(float x) noexcept
{
  return std::max(0.0f, std::min(1.0f, x));
}

void GPKISMVocalDSP::prepare(double sampleRate) noexcept
{
  mSampleRate = std::max(8000.0, sampleRate);
  reset();
  setParams(VocalParams{});
}

void GPKISMVocalDSP::reset() noexcept
{
  for (auto& state : mState)
    state = ChannelState{};
}

void GPKISMVocalDSP::setParams(const VocalParams& p) noexcept
{
  const float sr = static_cast<float>(mSampleRate);
  const float hp = std::max(35.0f, std::min(180.0f, p.highPassHz));

  mHpCoeff = std::exp(-2.0f * 3.14159265358979323846f * hp / sr);
  mToneCoeff = 1.0f - std::exp(-2.0f * 3.14159265358979323846f * 260.0f / sr);
  mAttackCoeff = std::exp(-1.0f / (0.012f * sr));
  mReleaseCoeff = std::exp(-1.0f / (0.110f * sr));

  mInputGain = dbToGain(std::max(-18.0f, std::min(18.0f, p.inputDb)));
  mOutputGain = dbToGain(std::max(-18.0f, std::min(6.0f, p.outputDb)));
  mCompAmount = clamp01(p.compression);
  mDrive = clamp01(p.drive);
  mHarmonicMix = std::min(0.30f, clamp01(p.harmonicMix));
  mDryWet = clamp01(p.dryWet);
  mDarkness = clamp01(p.darkness);
  mPresence = clamp01(p.presence);
  mModeDrive = (p.mode == PerformanceMode::Intense) ? 1.15f : 1.0f;
}

float GPKISMVocalDSP::processSample(float input, ChannelState& state) noexcept
{
  const float dry = input;
  float x = input * mInputGain;

  // Low-cost high-pass filter to reduce rumble while retaining vocal body.
  const float hp = mHpCoeff * (state.hpY + x - state.hpX);
  state.hpX = x;
  state.hpY = hp;
  x = hp;

  // Broad tonal contour: controlled low-mid reduction and upper-mid detail.
  state.toneY += mToneCoeff * (x - state.toneY);
  const float lowMid = state.toneY;
  const float detail = x - lowMid;
  x += (-0.10f * mDarkness * lowMid) + (0.18f * mPresence * detail);

  // Smoothed envelope compressor. Coefficients are precomputed per block.
  const float level = std::fabs(x);
  const float coeff = level > state.envelope ? mAttackCoeff : mReleaseCoeff;
  state.envelope = coeff * state.envelope + (1.0f - coeff) * level;

  const float threshold = 0.62f - 0.18f * mCompAmount;
  const float over = std::max(0.0f, state.envelope - threshold);
  const float ratio = 1.0f + 2.5f * mCompAmount;
  const float targetGain = over > 0.0f
    ? (threshold + over / ratio) / std::max(state.envelope, 1.0e-6f)
    : 1.0f;
  x *= targetGain;

  // Parallel soft saturation adds restrained harmonics without hard clipping.
  const float drive = 1.0f + 1.8f * mDrive * mModeDrive;
  const float saturated = std::tanh(x * drive) / drive;
  const float colour = std::min(0.30f, mHarmonicMix * (0.35f + 0.65f * mDrive));
  x = x * (1.0f - colour) + saturated * colour;

  const float wet = x * mOutputGain;
  const float mixed = dry * (1.0f - mDryWet) + wet * mDryWet;

  // Smooth safety ceiling; catches exceptional peaks without hard truncation.
  float safe = mixed;
  const float magnitude = std::fabs(safe);
  if (magnitude > 0.97f)
    safe = std::copysign(0.97f + 0.025f * std::tanh((magnitude - 0.97f) / 0.025f), safe);

  return std::isfinite(safe) ? safe : 0.0f;
}

void GPKISMVocalDSP::process(float* const* channels, int channelCount, int sampleCount) noexcept
{
  if (!channels || sampleCount <= 0)
    return;

  const int count = std::min(2, std::max(0, channelCount));
  for (int ch = 0; ch < count; ++ch) {
    if (!channels[ch])
      continue;

    ChannelState& state = mState[ch];
    for (int i = 0; i < sampleCount; ++i)
      channels[ch][i] = processSample(channels[ch][i], state);
  }
}

} // namespace nobu::dsp
