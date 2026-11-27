#include "GPKISMVocalSuite.h"
#include "IPlug_include_in_plug_src.h"
#if IPLUG_EDITOR
#include "IControls.h"
#endif
GPKISMVocalSuite::GPKISMVocalSuite(const InstanceInfo& info) : Plugin(info, MakeConfig(kNumParams, 1)) {
  GetParam(kParamInput)->InitDouble("Input", 0.0, -18.0, 18.0, 0.1, "dB");
  GetParam(kDarkness)->InitDouble("Darkness", 25.0, 0.0, 100.0, 0.1, "%");
  GetParam(kPresence)->InitDouble("Presence", 18.0, 0.0, 100.0, 0.1, "%");
  GetParam(kCompression)->InitDouble("Compression", 25.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDrive)->InitDouble("Drive", 3.0, 0.0, 100.0, 0.1, "%");
  GetParam(kHarmonicMix)->InitDouble("Harmonic Mix", 8.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDryWet)->InitDouble("Dry / Wet", 100.0, 0.0, 100.0, 0.1, "%");
  GetParam(kParamOutput)->InitDouble("Output", -3.0, -18.0, 6.0, 0.1, "dB");
  GetParam(kMode)->InitEnum("Performance", 0, {"Clean", "Intense"});
  GetParam(kPreset)->InitEnum("Vocal Character", 0, {"Nobu Natural", "GPKISM 55%", "Kaya Inspired"});
#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() { return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS); };
  mLayoutFunc = [&](IGraphics* g) {
    g->AttachPanelBackground(IColor(255, 12, 10, 18));
    const IRECT b = g->GetBounds().GetPadded(-22.f);
    g->AttachControl(new ITextControl(IRECT(b.L, b.T, b.R, b.T + 48.f),
      "NOBU MORTEM  /  GPKISM VOCAL SUITE", IText(21, IColor(255,235,225,245), "Arial", EAlign::Center)));
    g->AttachControl(new ITextControl(IRECT(b.L, b.T + 48.f, b.R, b.T + 72.f),
      "GOTHIC VISUAL KEI  •  VOCAL CHARACTER PROCESSOR", IText(10, IColor(255,175,145,195), "Arial", EAlign::Center)));
    const float colW = b.W() / 4.f;
    const char* names[] = {"DARKNESS", "PRESENCE", "COMPRESSION", "DRIVE", "HARMONICS", "DRY / WET", "OUTPUT", "INPUT"};
    const int ids[] = {kDarkness,kPresence,kCompression,kDrive,kHarmonicMix,kDryWet,kParamOutput,kParamInput};
    for (int i=0;i<8;i++) {
      const int col=i%4, row=i/4;
      const float x=b.L+col*colW, y=b.T+82.f+row*190.f;
      IRECT label(x,y,x+colW,y+24.f);
      IRECT knob(x+(colW-112.f)*0.5f,y+28.f,x+(colW+112.f)*0.5f,y+140.f);
      g->AttachControl(new ITextControl(label,names[i],IText(12,IColor(255,205,175,225),"Arial",EAlign::Center)));
      g->AttachControl(new IVKnobControl(knob,ids[i],names[i],DEFAULT_STYLE,false,true));
    }
    g->AttachControl(new ITextControl(IRECT(b.L,b.B-78.f,b.L+180.f,b.B-48.f),"VOCAL CHARACTER",IText(11,IColor(255,210,180,225),"Arial",EAlign::Near)));
    g->AttachControl(new IVSwitchControl(IRECT(b.L,b.B-46.f,b.L+430.f,b.B-8.f),kPreset,"NOBU NATURAL / GPKISM 55% / KAYA"));
    g->AttachControl(new IVSwitchControl(IRECT(b.R-180.f,b.B-46.f,b.R,b.B-8.f),kMode,"CLEAN / INTENSE"));
    g->AttachControl(new ITextControl(IRECT(b.R-360.f,b.B-32.f,b.R,b.B-2.f),
      "SUBTLE COLOUR  /  PRESERVE YOUR VOICE",IText(10,IColor(255,160,135,185),"Arial",EAlign::Far)));
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
  const int preset = GetParam(kPreset)->Int();
  if (preset == 1) { p.darkness=0.58f; p.presence=0.38f; p.compression=0.34f; p.drive=0.08f; p.harmonicMix=0.12f; p.inputDb=-2.0f; p.outputDb=-4.0f; }
  else if (preset == 2) { p.darkness=0.22f; p.presence=0.52f; p.compression=0.28f; p.drive=0.04f; p.harmonicMix=0.07f; p.inputDb=-2.0f; p.outputDb=-4.0f; }
  mDSP.setParams(p);
  float frame[2]{}; float* framePtrs[2] = {&frame[0], &frame[1]};
  for (int s=0;s<nFrames;s++) {
    for (int c=0;c<nChans && c<2;c++) frame[c] = (float)inputs[c][s];
    mDSP.process(framePtrs, nChans, 1);
    for (int c=0;c<nChans && c<2;c++) outputs[c][s] = (sample)frame[c];
  }
}
#endif
