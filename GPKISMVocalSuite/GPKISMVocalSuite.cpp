#include "GPKISMVocalSuite.h"
#include "IPlug_include_in_plug_src.h"
#if IPLUG_EDITOR
#include "IControls.h"
#endif

GPKISMVocalSuite::GPKISMVocalSuite(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1))
{
  GetParam(kParamInput)->InitDouble("Input Trim", 0.0, -18.0, 6.0, 0.1, "dB");
  GetParam(kDarkness)->InitDouble("Darkness", 20.0, 0.0, 100.0, 0.1, "%");
  GetParam(kPresence)->InitDouble("Presence", 18.0, 0.0, 100.0, 0.1, "%");
  GetParam(kCompression)->InitDouble("Compression", 22.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDrive)->InitDouble("Drive", 0.0, 0.0, 100.0, 0.1, "%");
  GetParam(kHarmonicMix)->InitDouble("Harmonic Mix", 0.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDryWet)->InitDouble("Dry / Wet", 100.0, 0.0, 100.0, 0.1, "%");
  GetParam(kParamOutput)->InitDouble("Output Trim", 0.0, -18.0, 6.0, 0.1, "dB");
  GetParam(kMode)->InitEnum("Performance", 0, {"Clean", "Intense"});
  GetParam(kPreset)->InitEnum("Vocal Character", 0,
    {"Nobu Natural", "GPKISM 55%", "Kaya Inspired", "Custom"});

#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS,
                        GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* g) {
    if (g->NControls() > 0)
      return;

    const IColor bg(255, 9, 7, 14);
    const IColor card(255, 20, 16, 29);
    const IColor bright(255, 246, 239, 250);
    const IColor muted(255, 184, 160, 205);
    const IColor violet(255, 177, 112, 214);
    const IColor accent(255, 115, 65, 145);

    g->AttachPanelBackground(bg);
    g->EnableTooltips(true);
    const IRECT b = g->GetBounds().GetPadded(24.f);

    g->AttachControl(new ITextControl(
      IRECT(b.L, b.T, b.R, b.T + 37.f),
      "NOBU MORTEM  /  GPKISM VOCAL SUITE",
      IText(24.f, bright, "Arial", EAlign::Center)));

    g->AttachControl(new ITextControl(
      IRECT(b.L, b.T + 38.f, b.R, b.T + 58.f),
      "GOTHIC VISUAL KEI  •  VOCAL CHARACTER PROCESSOR",
      IText(11.f, muted, "Arial", EAlign::Center)));

    g->AttachControl(new ITextControl(
      IRECT(b.L, b.T + 60.f, b.R, b.T + 78.f),
      "PRESETS RECALL THE CONTROLS BELOW  •  ADJUST ANY KNOB TO CREATE A CUSTOM SETTING",
      IText(9.f, accent, "Arial", EAlign::Center)));

    const IVColorSpec colors {
      IColor(255, 17, 13, 24),    // background
      IColor(255, 226, 211, 238), // foreground
      IColor(255, 150, 91, 188),  // pressed
      IColor(255, 71, 52, 88),    // frame
      IColor(255, 197, 133, 225), // highlight
      IColor(155, 0, 0, 0),       // shadow
      IColor(255, 91, 54, 117),
      IColor(255, 169, 128, 194),
      IColor(255, 245, 229, 250)
    };

    const IVStyle knobStyle(false, true, colors,
      IText(12.f, bright, "Arial", EAlign::Center),
      IText(13.f, bright, "Arial", EAlign::Center),
      true, false, true, false, 0.16f, 1.f, 2.f, 0.78f);

    const char* names[] = {
      "DARKNESS", "PRESENCE", "COMPRESSION", "DRIVE",
      "HARMONIC MIX", "DRY / WET", "OUTPUT TRIM", "INPUT TRIM"
    };
    const char* tips[] = {
      "Darkens the low-mid body of the vocal. Start subtly.",
      "Adds upper-mid definition and vocal intelligibility.",
      "Controls vocal dynamics. Higher values apply stronger compression.",
      "Adds controlled saturation and density.",
      "Blends additional harmonic coloration into the processed voice.",
      "Blends the processed signal with the original signal.",
      "Sets the final output level after the Dry / Wet blend.",
      "Sets the input level before processing."
    };
    const int ids[] = {
      kDarkness, kPresence, kCompression, kDrive,
      kHarmonicMix, kDryWet, kParamOutput, kParamInput
    };

    const float top = b.T + 88.f;
    const float bottom = b.B - 112.f;
    const float cellW = b.W() / 4.f;
    const float rowH = (bottom - top) / 2.f;

    for (int i = 0; i < 8; ++i) {
      const int col = i % 4;
      const int row = i / 4;
      const float x = b.L + col * cellW;
      const float y = top + row * rowH;
      const IRECT cell(x + 5.f, y + 4.f, x + cellW - 5.f, y + rowH - 5.f);

      g->AttachControl(new IPanelControl(cell, card));

      g->AttachControl(new ITextControl(
        IRECT(cell.L + 5.f, cell.T + 8.f, cell.R - 5.f, cell.T + 29.f),
        names[i], IText(12.f, muted, "Arial", EAlign::Center)));

      const IRECT knobBounds(cell.L + 12.f, cell.T + 29.f,
                             cell.R - 12.f, cell.B - 8.f);
      auto* knob = new IVKnobControl(knobBounds, ids[i], "", knobStyle,
                                     false, false);
      knob->SetTooltip(tips[i]);
      g->AttachControl(knob);
    }

    const float footerY = b.B - 94.f;
    const float halfW = (b.W() - 20.f) * 0.5f;

    g->AttachControl(new ITextControl(
      IRECT(b.L, footerY, b.L + halfW, footerY + 18.f),
      "VOCAL CHARACTER",
      IText(10.f, violet, "Arial", EAlign::Center)));

    g->AttachControl(new IVSwitchControl(
      IRECT(b.L, footerY + 20.f, b.L + halfW, b.B - 2.f),
      kPreset, "", knobStyle.WithShowValue(true)));

    g->AttachControl(new ITextControl(
      IRECT(b.L + halfW + 20.f, footerY, b.R, footerY + 18.f),
      "PERFORMANCE MODE",
      IText(10.f, violet, "Arial", EAlign::Center)));

    g->AttachControl(new IVSwitchControl(
      IRECT(b.L + halfW + 20.f, footerY + 20.f, b.R, b.B - 2.f),
      kMode, "", knobStyle.WithShowValue(true)));
  };
#endif
}

#if IPLUG_EDITOR
void GPKISMVocalSuite::OnParamChangeUI(int paramIdx, EParamSource source)
{
  // Recall a character only when the user or host changes the selector.
  // State/preset restoration must retain any previously customized knob values.
  if (source != kUI && source != kHost)
    return;

  if (paramIdx == kPreset) {
    const int selected = GetParam(kPreset)->Int();
    if (selected >= 0 && selected <= 2)
      mPendingPreset.store(selected);
  }
  else if (paramIdx >= kDarkness && paramIdx <= kParamOutput) {
    mPendingPreset.store(3); // Custom
  }
}

void GPKISMVocalSuite::OnIdle()
{
  const int pending = mPendingPreset.exchange(-1);
  if (pending < 0)
    return;

  if (pending == 3) {
    SetParameterValue(kPreset, GetParam(kPreset)->ToNormalized(3));
    if (GetUI())
      GetUI()->SetAllControlsDirty();
    return;
  }

  // Each character is a real parameter recall. The knobs, host automation,
  // saved project state and DSP all use the same values (no hidden offsets).
  static const double presetValues[3][8] = {
    // Input, Darkness, Presence, Compression, Drive, Harmonics, Dry/Wet, Output
    { 0.0, 20.0, 18.0, 22.0,  0.0,  0.0, 100.0, 0.0 }, // Nobu Natural
    { 0.0, 42.0, 32.0, 50.0, 18.0, 24.0, 100.0, 0.0 }, // GPKISM 55%
    { 0.0,  8.0, 52.0, 38.0,  8.0, 12.0, 100.0, 0.0 }  // Kaya Inspired
  };
  static const int parameterIds[8] = {
    kParamInput, kDarkness, kPresence, kCompression,
    kDrive, kHarmonicMix, kDryWet, kParamOutput
  };

  for (int i = 0; i < 8; ++i) {
    const int paramIdx = parameterIds[i];
    SetParameterValue(paramIdx, GetParam(paramIdx)->ToNormalized(presetValues[pending][i]));
  }

  if (GetUI())
    GetUI()->SetAllControlsDirty();
}
#endif

#if IPLUG_DSP
void GPKISMVocalSuite::OnReset()
{
  mDSP.prepare(GetSampleRate());
}

void GPKISMVocalSuite::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const int nChans = std::min(2, NOutChansConnected());
  if (nChans <= 0)
    return;

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
