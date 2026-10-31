# GARASI AMP - Technical design

## Target product
- Platform: Windows x64
- Host: Studio One (audio effect insert)
- Format: Steinberg VST3, built with JUCE 8.0.16-security + CMake + MSVC
- Purpose: realtime DI guitar processing, not a virtual instrument and not MIDI note generation
- UI language: English for standard audio labels; Indonesian README

## Processing graph

```text
IN (Mono/Stereo)
  | Input gain
  v
32Hz HPF
  |
  +--> Envelope tracker --> Smooth gate gain
  |
  v
Stomp pre-gain --> Bright emphasis
  |
  v
Clean / Crunch / Lead waveshaping (nonlinear tanh; v0.1 is 1x)
  |
  v
Bass low shelf --> Mid bell --> Treble high shelf --> Presence
  |
  v
Cab voicing (high pass + colored midrange + dual low pass)
  |
  v
Echo delay send/feedback --> Reverb (4 combs + 2 allpass)
  |
  v
Master gain + output soft guard
  |
OUT (same channel layout as input)
```

## Data flow and realtime safety
- Parameter changes arrive via JUCE APVTS and are captured once each audio block.
- DSP `setSettings` updates filter coefficients without allocating memory.
- Each audio channel has separate filter states/delay/reverb buffers, allocated during `prepare`.
- GUI meter receives only an atomic peak value.
- Preset file reading/writing runs on UI callbacks only; the realtime processing callback never touches disk.
- Audio thread creates no owned GUI objects.

## Intentional prototype limitations
- Saturation algorithm is a simple non-oversampled waveshaper. At higher gain it can alias.
- Cabinet simulation is filter-based and is not a measured guitar speaker impulse response.
- Hard feedback limits, output tanh, and parameter bounds prevent extreme numeric output but are **not** true peak limiters.
- Per-block filter coefficient changes are not smoothed and could click under fast automation.
- Gate/delay/reverb are simple educational algorithms, not modeled versions of specific commercial hardware.

## Testing matrix
- Pure C++: silence, nonzero output with guitar tone, finite amplitude at 44.1/48/96k, bypass bit equality.
- Build: MSVC 2022 CMake Release; run CTest; inspect actual `.vst3` package layout.
- Host: Studio One (mono and stereo audio tracks, monitoring, insert browser, preset load/save, state recall, automation, 10 minute live session).
- Distribution: add VST3 validator, code signing if required by target environment and version control before release.
