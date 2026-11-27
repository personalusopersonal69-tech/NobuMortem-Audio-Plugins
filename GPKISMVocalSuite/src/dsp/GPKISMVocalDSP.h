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
  float darkness = 0.35f;
  float presence = 0.25f;
  float compression = 0.30f; // 0..1
  float drive = 0.12f;       // 0..1
  float harmonicMix = 0.18f; // 0..1
  float dryWet = 1.0f;       // 0..1
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
    float envelope = 0.0f;
  };

  static float dbToGain(float db) noexcept;
  static float clamp01(float x) noexcept;
  float processSample(float input, ChannelState& state) noexcept;

  double mSampleRate = 44100.0;
  float mHpCoeff = 0.99f;
  float mInputGain = 1.0f;
  float mOutputGain = 1.0f;
  float mCompAmount = 0.30f;
  float mDrive = 0.12f;
  float mHarmonicMix = 0.18f;
  float mDryWet = 1.0f;
  float mDarkness = 0.35f;
  float mPresence = 0.25f;
  float mModeDrive = 1.0f;
  ChannelState mState[2]{};
};

} // namespace nobu::dsp
