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
  mToneCoeff = 1.0f - std::exp(-2.0f * 3.14159265358979323846f * 240.0f / sr);
  mAttackCoeff = std::exp(-1.0f / (0.010f * sr));
  mReleaseCoeff = std::exp(-1.0f / (0.140f * sr));

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

float GPKISMVocalDSP::processSample(float input, ChannelState& state) noexcept
{
  const float dry = input;
  float x = input * mInputGain;

  // Gentle rumble control; the original dry path remains available via Dry/Wet.
  const float hp = mHpCoeff * (state.hpY + x - state.hpX);
  state.hpX = x;
  state.hpY = hp;
  x = hp;

  // Broad low-mid body and upper-mid presence controls with audible but restrained ranges.
  state.toneY += mToneCoeff * (x - state.toneY);
  const float lowBody = state.toneY;
  const float upperDetail = x - lowBody;
  x += (-0.24f * mDarkness * lowBody) + (0.34f * mPresence * upperDetail);

  // Smooth vocal compression. The threshold and ratio move continuously with the knob.
  const float level = std::fabs(x);
  const float coeff = level > state.envelope ? mAttackCoeff : mReleaseCoeff;
  state.envelope = coeff * state.envelope + (1.0f - coeff) * level;
  const float threshold = 0.72f - 0.34f * mCompAmount;
  const float over = std::max(0.0f, state.envelope - threshold);
  const float ratio = 1.0f + 4.0f * mCompAmount;
  const float targetGain = over > 0.0f
    ? (threshold + over / ratio) / std::max(state.envelope, 1.0e-6f)
    : 1.0f;
  x *= targetGain;

  // Drive controls saturation intensity; Harmonic Mix controls how much is blended in.
  const float driveFactor = 1.0f + 1.25f * mDrive * mModeDrive;
  const float saturated = std::tanh(x * driveFactor) / driveFactor;
  const float harmonicBlend = 0.40f * mHarmonicMix;
  const float driveBlend = 0.22f * mDrive;
  const float blend = std::min(0.48f, harmonicBlend + driveBlend);
  x = x * (1.0f - blend) + saturated * blend;

  const float wet = x * mOutputGain;
  const float mixed = dry * (1.0f - mDryWet) + wet * mDryWet;

  // Transparent safety ceiling: only engages on peaks above -0.7 dBFS.
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
  for (int ch = 0; ch < count; ++ch) {
    if (!channels[ch])
      continue;

    ChannelState& state = mState[ch];
    for (int i = 0; i < sampleCount; ++i)
      channels[ch][i] = processSample(channels[ch][i], state);
  }
}

} // namespace nobu::dsp
