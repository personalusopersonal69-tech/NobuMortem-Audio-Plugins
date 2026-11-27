#include "GPKISMVocalSuite.h"
#include "IPlug_include_in_plug_src.h"
#if IPLUG_EDITOR
#include "IControls.h"
#endif

GPKISMVocalSuite::GPKISMVocalSuite(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1))
{
  GetParam(kParamInput)->InitDouble("Input Trim", -6.0, -18.0, 6.0, 0.1, "dB");
  GetParam(kDarkness)->InitDouble("Darkness", 25.0, 0.0, 100.0, 0.1, "%");
  GetParam(kPresence)->InitDouble("Presence", 18.0, 0.0, 100.0, 0.1, "%");
  GetParam(kCompression)->InitDouble("Compression", 22.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDrive)->InitDouble("Drive", 0.0, 0.0, 100.0, 0.1, "%");
  GetParam(kHarmonicMix)->InitDouble("Harmonic Mix", 0.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDryWet)->InitDouble("Dry / Wet", 100.0, 0.0, 100.0, 0.1, "%");
  GetParam(kParamOutput)->InitDouble("Output Trim", -6.0, -18.0, 6.0, 0.1, "dB");
  GetParam(kMode)->InitEnum("Performance", 0, {"Clean", "Intense"});
  GetParam(kPreset)->InitEnum("Vocal Character", 0, {"Nobu Natural", "GPKISM 55%", "Kaya Inspired"});

#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() { return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT)); };
  mLayoutFunc = [&](IGraphics* g) {
    if (g->NControls() > 0)
      return;

    g->AttachPanelBackground(IColor(255, 12, 10, 18));
    g->EnableTooltips(true);
    const IRECT b = g->GetBounds().GetPadded(18.f);

    const IText titleText(22.f, IColor(255, 242, 232, 250), "Arial", EAlign::Center);
    const IText subtitleText(10.f, IColor(255, 190, 155, 215), "Arial", EAlign::Center);
    g->AttachControl(new ITextControl(IRECT(b.L, b.T, b.R, b.T + 34.f),
      "NOBU MORTEM  /  GPKISM VOCAL SUITE", titleText));
    g->AttachControl(new ITextControl(IRECT(b.L, b.T + 34.f, b.R, b.T + 56.f),
      "GOTHIC VISUAL KEI  •  VOCAL CHARACTER PROCESSOR", subtitleText));

    const IVStyle knobStyle {
      true, true,
      {
        IColor(255, 25, 20, 32),
        IColor(255, 220, 195, 235),
        IColor(255, 135, 75, 165),
        IColor(255, 70, 55, 82),
        IColor(255, 190, 115, 225),
        IColor(180, 0, 0, 0),
        IColor(255, 80, 45, 100),
        IColor(255, 165, 125, 190),
        IColor(255, 235, 215, 245)
      },
      IText(13.f, IColor(255, 225, 205, 238), "Arial", EAlign::Center)
    };

    const char* names[] = {
      "DARKNESS", "PRESENCE", "COMPRESSION", "DRIVE",
      "HARMONIC MIX", "DRY / WET", "OUTPUT TRIM", "INPUT TRIM"
    };
    const int ids[] = {
      kDarkness, kPresence, kCompression, kDrive,
      kHarmonicMix, kDryWet, kParamOutput, kParamInput
    };

    const float colW = b.W() / 4.f;
    for (int i = 0; i < 8; ++i) {
      const int col = i % 4;
      const int row = i / 4;
      const float x = b.L + col * colW;
      const float y = b.T + 72.f + row * 184.f;
      const IRECT knobBounds(x + 8.f, y, x + colW - 8.f, y + 164.f);
      g->AttachControl(new IVKnobControl(knobBounds, ids[i], names[i], knobStyle, false, false));
    }

    const float bottomY = b.B - 66.f;
    g->AttachControl(new ITextControl(
      IRECT(b.L, bottomY - 20.f, b.L + 210.f, bottomY),
      "VOCAL CHARACTER PRESET",
      IText(11.f, IColor(255, 220, 190, 235), "Arial", EAlign::Near)));
    g->AttachControl(new IVSwitchControl(
      IRECT(b.L, bottomY, b.L + 390.f, b.B - 2.f), kPreset,
      "NOBU NATURAL / GPKISM 55% / KAYA", knobStyle));
    g->AttachControl(new IVSwitchControl(
      IRECT(b.R - 190.f, bottomY, b.R, b.B - 2.f), kMode,
      "CLEAN / INTENSE", knobStyle));
  };
#endif
}

#if IPLUG_DSP
void GPKISMVocalSuite::OnReset()
{
  mDSP.prepare(GetSampleRate());
}

void GPKISMVocalSuite::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const int nChans = std::min(2, NOutChansConnected());
  nobu::dsp::VocalParams p;

  p.inputDb = static_cast<float>(GetParam(kParamInput)->Value());
  p.darkness = static_cast<float>(GetParam(kDarkness)->Value() / 100.0);
  p.presence = static_cast<float>(GetParam(kPresence)->Value() / 100.0);
  p.compression = static_cast<float>(GetParam(kCompression)->Value() / 100.0);
  p.drive = static_cast<float>(GetParam(kDrive)->Value() / 100.0);
  p.harmonicMix = static_cast<float>(GetParam(kHarmonicMix)->Value() / 100.0);
  p.dryWet = static_cast<float>(GetParam(kDryWet)->Value() / 100.0);
  p.outputDb = static_cast<float>(GetParam(kParamOutput)->Value());
  p.mode = GetParam(kMode)->Int() == 1
    ? nobu::dsp::PerformanceMode::Intense
    : nobu::dsp::PerformanceMode::Clean;

  const int preset = GetParam(kPreset)->Int();
  if (preset == 1) {
    p.darkness = std::min(1.0f, 0.30f + p.darkness * 0.55f);
    p.presence = std::min(1.0f, 0.12f + p.presence * 0.55f);
    p.compression = std::min(1.0f, 0.28f + p.compression * 0.55f);
    p.drive = std::min(1.0f, 0.06f + p.drive * 0.30f);
    p.harmonicMix = std::min(1.0f, 0.08f + p.harmonicMix * 0.32f);
    p.outputDb -= 1.0f;
  }
  else if (preset == 2) {
    p.darkness = std::min(1.0f, 0.08f + p.darkness * 0.45f);
    p.presence = std::min(1.0f, 0.30f + p.presence * 0.60f);
    p.compression = std::min(1.0f, 0.18f + p.compression * 0.50f);
    p.drive = std::min(1.0f, 0.03f + p.drive * 0.25f);
    p.harmonicMix = std::min(1.0f, 0.04f + p.harmonicMix * 0.25f);
    p.outputDb -= 0.5f;
  }

  mDSP.setParams(p);

  float frame[2]{};
  float* framePtrs[2] = { &frame[0], &frame[1] };
  for (int s = 0; s < nFrames; ++s) {
    for (int c = 0; c < nChans; ++c)
      frame[c] = static_cast<float>(inputs[c][s]);

    mDSP.process(framePtrs, nChans, 1);

    for (int c = 0; c < nChans; ++c)
      outputs[c][s] = static_cast<sample>(frame[c]);
  }
}
#endif
