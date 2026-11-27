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
  const float sr = static_cast<float>(mSampleRate);

  mHpCoeff = std::exp(-2.0f * 3.14159265358979323846f * 75.0f / sr);
  mToneCoeff = 1.0f - std::exp(-2.0f * 3.14159265358979323846f * 240.0f / sr);
  mAttackCoeff = std::exp(-1.0f / (0.010f * sr));
  mReleaseCoeff = std::exp(-1.0f / (0.140f * sr));
  mSmoothCoeff = std::exp(-1.0f / (0.020f * sr));
  reset();
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
  mToneCoeff = 1.0f - std::exp(-2.0f * 3.14159265358979323846f * 240.0f / sr);

  mInputGain = dbToGain(std::max(-18.0f, std::min(6.0f, p.inputDb)));
  mOutputGain = dbToGain(std::max(-18.0f, std::min(6.0f, p.outputDb)));
  mCompAmount = clamp01(p.compression);
  mDrive = clamp01(p.drive);
  mHarmonicMix = clamp01(p.harmonicMix);
  mDryWet = clamp01(p.dryWet);
  mDarkness = clamp01(p.darkness);
  mPresence = clamp01(p.presence);
  mModeDrive = (p.mode == PerformanceMode::Intense) ? 1.10f : 1.0f;
}

void GPKISMVocalDSP::smoothParameters() noexcept
{
  const float glide = 1.0f - mSmoothCoeff;
  mCurrentInputGain += (mInputGain - mCurrentInputGain) * glide;
  mCurrentOutputGain += (mOutputGain - mCurrentOutputGain) * glide;
  mCurrentCompAmount += (mCompAmount - mCurrentCompAmount) * glide;
  mCurrentDrive += (mDrive - mCurrentDrive) * glide;
  mCurrentHarmonicMix += (mHarmonicMix - mCurrentHarmonicMix) * glide;
  mCurrentDryWet += (mDryWet - mCurrentDryWet) * glide;
  mCurrentDarkness += (mDarkness - mCurrentDarkness) * glide;
  mCurrentPresence += (mPresence - mCurrentPresence) * glide;
  mCurrentModeDrive += (mModeDrive - mCurrentModeDrive) * glide;
}

float GPKISMVocalDSP::processSample(float input, ChannelState& state) noexcept
{
  const float dry = input * mCurrentInputGain;
  float x = dry;

  // First-order high-pass for rumble control.
  const float hp = mHpCoeff * (state.hpY + x - state.hpX);
  state.hpX = x;
  state.hpY = hp;
  x = hp;

  // Broad low-mid body reduction and upper-mid presence.
  state.toneY += mToneCoeff * (x - state.toneY);
  const float lowBody = state.toneY;
  const float upperDetail = x - lowBody;
  x += (-0.24f * mCurrentDarkness * lowBody)
     + (0.34f * mCurrentPresence * upperDetail);

  // Smooth envelope detector and smoothed gain reduction.
  const float level = std::fabs(x);
  const float detectorCoeff = level > state.envelope ? mAttackCoeff : mReleaseCoeff;
  state.envelope = detectorCoeff * state.envelope + (1.0f - detectorCoeff) * level;
  const float threshold = 0.72f - 0.34f * mCurrentCompAmount;
  const float over = std::max(0.0f, state.envelope - threshold);
  const float ratio = 1.0f + 4.0f * mCurrentCompAmount;
  const float targetGain = over > 0.0f
    ? (threshold + over / ratio) / std::max(state.envelope, 1.0e-6f)
    : 1.0f;
  const float gainCoeff = targetGain < state.compressorGain ? mAttackCoeff : mReleaseCoeff;
  state.compressorGain = gainCoeff * state.compressorGain
                       + (1.0f - gainCoeff) * targetGain;
  x *= state.compressorGain;

  // Controlled saturation; Harmonic Mix controls the amount blended in.
  const float driveFactor = 1.0f + 1.25f * mCurrentDrive * mCurrentModeDrive;
  const float saturated = std::tanh(x * driveFactor) / driveFactor;
  const float harmonicBlend = 0.40f * mCurrentHarmonicMix;
  const float driveBlend = 0.22f * mCurrentDrive;
  const float blend = std::min(0.48f, harmonicBlend + driveBlend);
  x = x * (1.0f - blend) + saturated * blend;

  const float wet = x;
  const float mixed = (dry * (1.0f - mCurrentDryWet) + wet * mCurrentDryWet)
                    * mCurrentOutputGain;

  // Smooth soft ceiling: approaches 0.98 full scale (about -0.17 dBFS).
  const float magnitude = std::fabs(mixed);
  float safe = mixed;
  if (magnitude > 0.92f)
    safe = std::copysign(0.92f + 0.06f * std::tanh((magnitude - 0.92f) / 0.06f), mixed);

  return std::isfinite(safe) ? safe : 0.0f;
}

void GPKISMVocalDSP::process(float* const* channels, int channelCount, int sampleCount) noexcept
{
  if (!channels || sampleCount <= 0)
    return;

  const int count = std::min(2, std::max(0, channelCount));
  for (int i = 0; i < sampleCount; ++i) {
    smoothParameters();
    for (int ch = 0; ch < count; ++ch) {
      if (!channels[ch])
        continue;
      channels[ch][i] = processSample(channels[ch][i], mState[ch]);
    }
  }
}

} // namespace nobu::dsp
