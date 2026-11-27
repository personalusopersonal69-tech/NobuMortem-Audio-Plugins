#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace nobu::dsp {

enum class PerformanceMode : int { Clean = 0, Intense = 1 };

struct VocalParams {
  float inputDb = 0.0f;
  float outputDb = 0.0f;
  float highPassHz = 75.0f;
  float darkness = 0.20f;
  float presence = 0.18f;
  float compression = 0.22f;
  float drive = 0.0f;
  float harmonicMix = 0.0f;
  float dryWet = 1.0f;
  PerformanceMode mode = PerformanceMode::Clean;
};

class GPKISMVocalDSP {
public:
  void prepare(double sampleRate) noexcept;
  void reset() noexcept;
  void setParams(const VocalParams& params) noexcept;
  void process(float* const* channels, int channelCount, int sampleCount) noexcept;

private:
  struct ChannelState {
    float hpX = 0.0f;
    float hpY = 0.0f;
    float toneY = 0.0f;
    float envelope = 0.0f;
    float compressorGain = 1.0f;
  };

  static float dbToGain(float db) noexcept;
  static float clamp01(float x) noexcept;
  void smoothParameters() noexcept;
  float processSample(float input, ChannelState& state) noexcept;

  double mSampleRate = 44100.0;
  float mHpCoeff = 0.99f;
  float mToneCoeff = 0.03f;
  float mAttackCoeff = 0.998f;
  float mReleaseCoeff = 0.9998f;
  float mSmoothCoeff = 0.996f;

  // Target values are updated by setParams; current values glide smoothly
  // toward them once per sample frame, keeping stereo channels phase-aligned.
  float mInputGain = 1.0f;
  float mOutputGain = 1.0f;
  float mCompAmount = 0.22f;
  float mDrive = 0.0f;
  float mHarmonicMix = 0.0f;
  float mDryWet = 1.0f;
  float mDarkness = 0.20f;
  float mPresence = 0.18f;
  float mModeDrive = 1.0f;

  float mCurrentInputGain = 1.0f;
  float mCurrentOutputGain = 1.0f;
  float mCurrentCompAmount = 0.22f;
  float mCurrentDrive = 0.0f;
  float mCurrentHarmonicMix = 0.0f;
  float mCurrentDryWet = 1.0f;
  float mCurrentDarkness = 0.20f;
  float mCurrentPresence = 0.18f;
  float mCurrentModeDrive = 1.0f;

  ChannelState mState[2]{};
};

} // namespace nobu::dsp
