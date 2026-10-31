# GARASI AMP 0.1 - Guitar Amp & Effects VST3 for Studio One

A real C++/JUCE audio effect plug-in project, **not** an HTML mockup. Windows 64-bit VST3.

**Status:** the platform-independent DSP engine has been built and its automated tests passed on Linux. The *JUCE plug-in and Windows VST3 binary have not been built here* because this environment doesn't have the JUCE source and a Windows toolchain. To install it into Studio One on Windows, you must first compile it using the provided build script or the GitHub Actions workflow, then test it in Studio One. This is developer v0.1, not a commercially validated amplifier model.

## 1. What works in the source

Audio chain:

`GUITAR INPUT -> INPUT GAIN -> HIGH-PASS -> NOISE GATE -> BOOST -> NONLINEAR AMP (3 MODES) -> BASS/MID/TREBLE/PRESENCE -> CABINET VOICING -> DELAY -> REVERB -> MASTER -> OUTPUT`

- Three amp voicings: CLEAN, CRUNCH, LEAD.
- Noise gate with adjustable threshold and on/off.
- Preamp boost with adjustable intensity and on/off.
- Bass / Mid / Treble / Presence, adjustable gain staging.
- 1x12 OPEN, 2x12 VINTAGE, 4x12 MODERN *filter-based* cabinet responses. **These are not measured impulse responses**.
- Delay with time/feedback/mix and enable/disable.
- Reverb with size/mix and enable/disable.
- Global bypass; live peak meter; four factory presets.
- SAVE/LOAD custom preset as `.xml`, and automatic state recall in Studio One sessions.
- Studio One parameter automation through JUCE AudioProcessorValueTreeState.
- Mono-to-mono and stereo-to-stereo effect buses; supported processing rates 44.1/48/96 kHz tested in the DSP harness.
- C++ DSP allocates buffers only at `prepare()` time; audio sample processing has no locks, file I/O or heap allocation.

**Not yet present:** IR WAV loader / true cabinet convolution, oversampling, standalone app, cabinet mic positioning, oversampled distortion, chorus, tuner, true-peak limiter, official installer .exe, code signing. These belong to follow-on versions.

## 2. Prerequisites (Windows)

1. Windows 10/11 x64.
2. Visual Studio 2026 or 2022 Community **with "Desktop development with C++"** and the MSVC v143 C++ toolset + Windows SDK.
3. Git for Windows.
4. CMake 3.24 or newer (optionally Ninja); enable `cmake` and `git` in `PATH`.
5. Internet for the first build (the CMake script downloads JUCE **8.0.16-security** from GitHub).
6. Studio One with VST3 plug-in support.

Official references:
- JUCE CMake API: https://github.com/juce-framework/JUCE/blob/master/docs/CMake%20API.md
- JUCE licensing: https://github.com/juce-framework/JUCE/blob/master/LICENSE.md
- Studio One scan locations: https://support.presonus.com/hc/en-us/articles/360045544271-How-can-I-get-my-3rd-party-plug-ins-to-show-up-in-Studio-One

## 3. Build + install automatically

Unzip the project, and in the project folder run **`BUILD_WINDOWS.cmd`**. Review the PowerShell script before running it if you like.

Or run from PowerShell:

```powershell
.\build-windows.ps1
```

The script uses CMake with the Visual Studio 2026 or 2022 x64 generator, builds Release, runs the DSP tests, produces:

`dist\GarasiAmp-Windows-x64-VST3.zip`

and installs the `.vst3` bundle to the per-user folder:

`%LOCALAPPDATA%\Programs\Common\VST3`

**Studio One 5.5.1 and newer** automatically scan that per-user VST3 path. If your Studio One is older, install system-wide using an **Administrator PowerShell**:

```powershell
.\build-windows.ps1 -SystemWide
```

The target then becomes:

`C:\Program Files\Common Files\VST3`

Only build/package without copying files into the system:

```powershell
.\build-windows.ps1 -BuildOnly
```

## 4. Build manually

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel 4
ctest --test-dir build --config Release --output-on-failure
```

JUCE writes the plugin bundle into the VST3 subfolder of `build\GarasiAmp_artefacts\Release` (bundle filename depends on JUCE product naming). Copy the *entire* `.vst3` bundle folder, not just the internal `.dll`.

Advanced: to use your own JUCE checkout instead of downloading:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DFETCHCONTENT_SOURCE_DIR_JUCE="C:/dev/JUCE"
```

## 5. Build via GitHub Actions (no local Visual Studio required)

1. Create an empty GitHub repository under your account (private is okay).
2. Upload the contents of this project, including `.github/workflows/windows-vst3.yml`.
3. In GitHub, open **Actions > Windows VST3 Build > Run workflow**.
4. After the workflow passes, download the `GarasiAmp-Windows-x64-VST3` artifact.
5. Unzip that artifact, then unzip `GarasiAmp-Windows-x64-VST3.zip` if necessary.
6. Copy the `.vst3` folder to one of the installation directories above; restart Studio One.

*This workflow has been supplied but not run yet. Do not claim the downloaded VST3 is validated until the build and Studio One tests pass.*

## 6. Studio One setup and verification

1. Plug guitar into an audio interface's **Instrument/Hi-Z** input, not a line input if possible.
2. In Studio One, create a **mono audio track** whose input is the guitar interface channel.
3. Enable input monitoring (speaker icon), and enable the interface's low-latency audio driver, normally ASIO on Windows.
4. In **Options > Locations > VST Plug-ins**, check *Scan at Startup*. Restart Studio One. If blocked, reset the plugin blocklist and rescan.
5. Search the Effects browser for **Garasi Amp**, insert it on the guitar track.
6. Choose `GLASS CLEAN` first. Try other presets with output volume down; high-drive tones can be significantly louder.
7. Save the song; reopen it and check parameter restoration. Save/load `.xml` presets; test toggles and DAW automation.

**Recommended manual QA:** monitor input/output, verify mono and stereo signal, dry bypass, no audible glitches with parameter automation, CPU load, sample rates 44.1/48/96 kHz, Studio One plug-in scan, crash-free repeated close/open, preset persistence, long reverb tail, no runaway feedback.

## 7. Architecture

| File | Responsibility |
| ---- | -------------- |
| `Source/AmpEngine.h/.cpp` | Framework-free, realtime DSP implementation |
| `Source/PluginProcessor.h/.cpp` | JUCE VST3 wrapper, parameters and host state serialization |
| `Source/PluginEditor.h/.cpp` | Dark amp-styled GUI, parameter attachments, factory presets, save/load |
| `CMakeLists.txt` | Reproducible JUCE/CMake project |
| `tests/dsp_tests.cpp` | Cross-platform audio sanity tests |
| `build-windows.ps1` | One-command Windows build/test/installation |
| `.github/workflows/windows-vst3.yml` | Cloud Windows binary build and downloadable package |

## 8. Next engineering milestones

| Version | Milestone | Acceptance criterion |
| -- | -- | -- |
| 0.1 | Build VST3 and run it in Studio One | Loads in FX browser, audio heard, tests pass, session recalls |
| 0.2 | IR cabinet loader (`.wav`) and true convolution | User IR persists/reloads correctly; safe if missing on another PC |
| 0.3 | 4x/8x oversampling and parameter smoothing | Measured reduction in alias products and click-free automations |
| 0.4 | Gain staging, presets, A/B, tuner, tempo-sync delay | Musical playability and reproducible preset recall |
| 1.0 | Full QA, packaging/signing and distribution | VST3 plugin validation + Studio One regression suite on Windows |

## 9. License note

Project code in this folder is supplied as an example for private development, and does not bundle JUCE source code. JUCE has AGPLv3/open-source and commercial licensing conditions. Before distributing a proprietary or commercial product, check the applicable JUCE license, third-party assets/IR licenses, and any required plugin certification.


### Note for Visual Studio 2026
The Windows build script autodetects Visual Studio 2026 and uses the `Visual Studio 18 2026` CMake generator. This requires a CMake build that supports that generator (CMake 4.2+). The script will show actionable messages if a compatible CMake executable or Git is missing.
