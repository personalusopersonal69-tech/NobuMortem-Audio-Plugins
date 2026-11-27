#pragma once
#include "IPlug_include_in_plug_hdr.h"
#include "src/dsp/GPKISMVocalDSP.h"
using namespace iplug;
using namespace igraphics;
enum EParams { kParamInput = 0, kDarkness, kPresence, kCompression, kDrive, kHarmonicMix, kDryWet, kParamOutput, kMode, kPreset, kNumParams };
class GPKISMVocalSuite final : public Plugin {
public:
  GPKISMVocalSuite(const InstanceInfo& info);
#if IPLUG_EDITOR
  bool OnHostRequestingSupportedViewConfiguration(int width, int height) override { return true; }
#endif
#if IPLUG_DSP
  void OnReset() override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
private:
  nobu::dsp::GPKISMVocalDSP mDSP;
};
