#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
namespace nobu::dsp {
enum class PerformanceMode : int { Clean = 0, Intense = 1 };
struct VocalParams {
 float inputDb=0.0f, outputDb=0.0f, highPassHz=75.0f;
 float darkness=0.35f, presence=0.25f, compression=0.30f;
 float drive=0.12f, harmonicMix=0.18f, dryWet=1.0f;
 PerformanceMode mode=PerformanceMode::Clean;
};
class GPKISMVocalDSP {
public:
 void prepare(double sampleRate) noexcept;
 void reset() noexcept;
 void setParams(const VocalParams& params) noexcept;
 void process(float* const* channels,int channelCount,int sampleCount) noexcept;
private:
 struct ChannelState { float hpX=0.0f,hpY=0.0f,toneY=0.0f,envelope=0.0f; };
 static float dbToGain(float db) noexcept;
 static float clamp01(float x) noexcept;
 float processSample(float input,ChannelState& state) noexcept;
 double mSampleRate=44100.0;
 float mHpCoeff=0.99f,mToneCoeff=0.03f,mAttackCoeff=0.998f,mReleaseCoeff=0.9998f;
 float mInputGain=1.0f,mOutputGain=1.0f,mCompAmount=0.30f,mDrive=0.12f;
 float mHarmonicMix=0.18f,mDryWet=1.0f,mDarkness=0.35f,mPresence=0.25f,mModeDrive=1.0f;
 ChannelState mState[2]{};
};
} // namespace nobu::dsp
