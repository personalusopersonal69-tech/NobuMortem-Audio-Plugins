#include "GPKISMVocalSuite.h"
#include "IPlug_include_in_plug_src.h"
#if IPLUG_EDITOR
#include "IControls.h"
#endif

GPKISMVocalSuite::GPKISMVocalSuite(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, 1))
{
  // Neutral gain staging is the safest starting point for a vocal insert.
  GetParam(kParamInput)->InitDouble("Input Trim", 0.0, -18.0, 6.0, 0.1, "dB");
  GetParam(kDarkness)->InitDouble("Darkness", 25.0, 0.0, 100.0, 0.1, "%");
  GetParam(kPresence)->InitDouble("Presence", 18.0, 0.0, 100.0, 0.1, "%");
  GetParam(kCompression)->InitDouble("Compression", 22.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDrive)->InitDouble("Drive", 0.0, 0.0, 100.0, 0.1, "%");
  GetParam(kHarmonicMix)->InitDouble("Harmonic Mix", 0.0, 0.0, 100.0, 0.1, "%");
  GetParam(kDryWet)->InitDouble("Dry / Wet", 100.0, 0.0, 100.0, 0.1, "%");
  GetParam(kParamOutput)->InitDouble("Output Trim", 0.0, -18.0, 6.0, 0.1, "dB");
  GetParam(kMode)->InitEnum("Performance", 0, {"Clean", "Intense"});
  GetParam(kPreset)->InitEnum("Vocal Character", 0, {"Nobu Natural", "GPKISM 55%", "Kaya Inspired"});

#if IPLUG_EDITOR
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS,
                        GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };

  mLayoutFunc = [&](IGraphics* g) {
    if (g->NControls() > 0)
      return;

    const IColor bg(255, 10, 8, 15);
    const IColor panel(255, 20, 16, 28);
    const IColor bright(255, 245, 235, 250);
    const IColor muted(255, 190, 165, 210);
    const IColor violet(255, 170, 105, 205);

    g->AttachPanelBackground(bg);
    g->EnableTooltips(true);
    const IRECT b = g->GetBounds().GetPadded(22.f);

    g->AttachControl(new ITextControl(
      IRECT(b.L, b.T, b.R, b.T + 38.f),
      "NOBU MORTEM  /  GPKISM VOCAL SUITE",
      IText(23.f, bright, "Arial", EAlign::Center)));

    g->AttachControl(new ITextControl(
      IRECT(b.L, b.T + 38.f, b.R, b.T + 60.f),
      "GOTHIC VISUAL KEI  •  VOCAL CHARACTER PROCESSOR",
      IText(11.f, muted, "Arial", EAlign::Center)));

    const IVColorSpec knobColors {
      IColor(255, 24, 19, 31),
      IColor(255, 225, 205, 238),
      IColor(255, 145, 78, 180),
      IColor(255, 68, 52, 82),
      IColor(255, 195, 125, 225),
      IColor(170, 0, 0, 0),
      IColor(255, 92, 48, 112),
      IColor(255, 175, 135, 198),
      IColor(255, 245, 225, 250)
    };

    const IText valueText(12.f, bright, "Arial", EAlign::Center);
    const IVStyle knobStyle(false, true, knobColors,
                            IText(12.f, bright, "Arial", EAlign::Center),
                            valueText, false, true, true, true, 0.18f,
                            1.f, 2.f, 0.78f);

    const char* names[] = {
      "DARKNESS", "PRESENCE", "COMPRESSION", "DRIVE",
      "HARMONIC MIX", "DRY / WET", "OUTPUT TRIM", "INPUT TRIM"
    };
    const char* tips[] = {
      "Reduce low-mid body for a darker vocal tone.",
      "Add upper-mid definition and vocal intelligibility.",
      "Control vocal dynamics with smooth compression.",
      "Add controlled saturation and density.",
      "Blend additional harmonic coloration.",
      "Blend the processed signal with the original.",
      "Set the final output level.",
      "Set the signal level entering the processor."
    };
    const int ids[] = {
      kDarkness, kPresence, kCompression, kDrive,
      kHarmonicMix, kDryWet, kParamOutput, kParamInput
    };

    const float top = b.T + 76.f;
    const float bottom = b.B - 112.f;
    const float cellW = b.W() / 4.f;
    const float rowH = (bottom - top) / 2.f;

    for (int i = 0; i < 8; ++i) {
      const int col = i % 4;
      const int row = i / 4;
      const float x = b.L + col * cellW;
      const float y = top + row * rowH;
      const IRECT cell(x + 5.f, y + 4.f, x + cellW - 5.f, y + rowH - 5.f);

      g->AttachControl(new IPanelControl(cell, panel));

      // Dedicated labels are intentionally separate from the knobs so they
      // remain readable even if a host scales or clips vector-control labels.
      g->AttachControl(new ITextControl(
        IRECT(cell.L + 4.f, cell.T + 8.f, cell.R - 4.f, cell.T + 30.f),
        names[i], IText(12.f, muted, "Arial", EAlign::Center)));

      const IRECT knobBounds(cell.L + 14.f, cell.T + 30.f,
                             cell.R - 14.f, cell.B - 8.f);
      g->AttachControl(new IVKnobControl(knobBounds, ids[i], "", knobStyle,
                                         false, false))->SetTooltip(tips[i]);
    }

    const float footerY = b.B - 91.f;
    const float halfW = (b.W() - 18.f) * 0.5f;

    g->AttachControl(new ITextControl(
      IRECT(b.L, footerY, b.L + halfW, footerY + 19.f),
      "VOICE CHARACTER  •  SELECT A BASE TONE",
      IText(10.f, violet, "Arial", EAlign::Center)));

    g->AttachControl(new IVSwitchControl(
      IRECT(b.L, footerY + 20.f, b.L + halfW, b.B - 2.f),
      kPreset, "", knobStyle.WithShowValue(true)));

    g->AttachControl(new ITextControl(
      IRECT(b.L + halfW + 18.f, footerY, b.R, footerY + 19.f),
      "PERFORMANCE",
      IText(10.f, violet, "Arial", EAlign::Center)));

    g->AttachControl(new IVSwitchControl(
      IRECT(b.L + halfW + 18.f, footerY + 20.f, b.R, b.B - 2.f),
      kMode, "", knobStyle.WithShowValue(true)));

    g->AttachControl(new ITextControl(
      IRECT(b.L, b.B - 1.f, b.R, b.B + 15.f),
      "Choose a character, then fine-tune it with the controls above.",
      IText(9.f, muted, "Arial", EAlign::Center)));
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

  // The selected character is a tonal starting point; the eight controls
  // remain continuous user adjustments layered on top of that character.
  const int preset = GetParam(kPreset)->Int();
  if (preset == 1) {
    p.darkness = std::min(1.0f, 0.30f + p.darkness * 0.55f);
    p.presence = std::min(1.0f, 0.12f + p.presence * 0.55f);
    p.compression = std::min(1.0f, 0.28f + p.compression * 0.55f);
    p.drive = std::min(1.0f, 0.06f + p.drive * 0.30f);
    p.harmonicMix = std::min(1.0f, 0.08f + p.harmonicMix * 0.32f);
  }
  else if (preset == 2) {
    p.darkness = std::min(1.0f, 0.08f + p.darkness * 0.45f);
    p.presence = std::min(1.0f, 0.30f + p.presence * 0.60f);
    p.compression = std::min(1.0f, 0.18f + p.compression * 0.50f);
    p.drive = std::min(1.0f, 0.03f + p.drive * 0.25f);
    p.harmonicMix = std::min(1.0f, 0.04f + p.harmonicMix * 0.25f);
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
