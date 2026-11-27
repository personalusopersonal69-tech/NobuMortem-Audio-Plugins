#include "GPKISMVocalSuite.h"
#include "IPlug_include_in_plug_src.h"
#if IPLUG_EDITOR
#include "IControls.h"
#endif
GPKISMVocalSuite::GPKISMVocalSuite(const InstanceInfo& info) : Plugin(info, MakeConfig(kNumParams, 1)) {
  GetParam(kParamInput)->InitDouble("Input", 0.0, -18.0, 18.0, 0.1, "dB");
  GetParam(kDarkness)->InitDouble("Darkness", 35.0, 0.0, 100.0, 0.1, "%");
  GetParam(kPresence)->InitDouble("Presence", 25.0, 0.0, 100.0, 0.1, "%");
  GetParam(kCompression)->InitDouble("Compression", 30.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDrive)->InitDouble("Drive", 12.0, 0.0, 100.0, 0.1, "%");
  GetParam(kHarmonicMix)->InitDouble("Harmonic Mix", 18.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDryWet)->InitDouble("Dry / Wet", 100.0, 0.0, 100.0, 0.1, "%");
  GetParam(kParamOutput)->InitDouble("Output", 0.0, -18.0, 6.0, 0.1, "dB");
  GetParam(kMode)->InitEnum("Performance", 0, {"Clean", "Intense"});
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() { return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS); };
  mLayoutFunc = [&](IGraphics* g) {
    g->AttachCornerResizer(EUIResizerMode::Size, true);
    g->AttachPanelBackground(IColor(255, 12, 10, 18));
    const IRECT b = g->GetBounds().GetPadded(-24.f);
    g->AttachControl(new ITextControl(b.GetFromTop(70.f), "NOBU MORTEM  /  GPKISM VOCAL SUITE", IText(22, IColor(255,235,235,245), "Arial", EAlign::Center)));
    const float w = b.W() / 4.f;
    const char* names[] = {"DARKNESS", "PRESENCE", "COMPRESSION", "DRIVE", "HARMONICS", "DRY / WET", "OUTPUT", "INPUT"};
    const int ids[] = {kDarkness,kPresence,kCompression,kDrive,kHarmonicMix,kDryWet,kParamOutput,kParamInput};
    for (int i=0;i<8;i++) {
      const int col=i%4, row=i/4;
      IRECT cell = b.GetFromTop(210.f).GetFromLeft(w).GetTranslated(col*w, row*220.f+90.f);
      g->AttachControl(new ITextControl(cell.GetFromTop(28.f), names[i], IText(12, IColor(255,185,150,220), "Arial", EAlign::Center)));
      g->AttachControl(new IVKnobControl(cell.GetReducedFromTop(32.f).GetCentredInside(116.f,116.f), ids[i]));
    }
    g->AttachControl(new IVSwitchControl(b.GetFromBottom(48.f).GetFromLeft(180.f).GetTranslated(0.f,-8.f), kMode, "CLEAN / INTENSE"));
    g->AttachControl(new ITextControl(b.GetFromBottom(28.f).GetFromRight(330.f), "DARK  /  ELEGANT  /  THEATRICAL", IText(12, IColor(255,160,130,190), "Arial", EAlign::Far)));
  };
#endif
}
#if IPLUG_DSP
void GPKISMVocalSuite::OnReset() { mDSP.prepare(GetSampleRate()); }
void GPKISMVocalSuite::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  const int nChans = NOutChansConnected();
  nobu::dsp::VocalParams p;
  p.inputDb = (float)GetParam(kParamInput)->Value();
  p.darkness = (float)(GetParam(kDarkness)->Value()/100.0);
  p.presence = (float)(GetParam(kPresence)->Value()/100.0);
  p.compression = (float)(GetParam(kCompression)->Value()/100.0);
  p.drive = (float)(GetParam(kDrive)->Value()/100.0);
  p.harmonicMix = (float)(GetParam(kHarmonicMix)->Value()/100.0);
  p.dryWet = (float)(GetParam(kDryWet)->Value()/100.0);
  p.outputDb = (float)GetParam(kParamOutput)->Value();
  p.mode = GetParam(kMode)->Int() == 1 ? nobu::dsp::PerformanceMode::Intense : nobu::dsp::PerformanceMode::Clean;
  mDSP.setParams(p);
  float frame[2]{}; float* framePtrs[2] = {&frame[0], &frame[1]};
  for (int s=0;s<nFrames;s++) {
    for (int c=0;c<nChans && c<2;c++) frame[c] = (float)inputs[c][s];
    mDSP.process(framePtrs, nChans, 1);
    for (int c=0;c<nChans && c<2;c++) outputs[c][s] = (sample)frame[c];
  }
}
#endif
