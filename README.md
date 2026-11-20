# RageForge — FL Studio Bass Synth (prototype 0.1)

A **real desktop instrument plugin project**, written in C++ using JUCE. Targets VST3 and standalone apps on Windows/macOS (not a phone web app). Designed for original dubstep bass stabs, talking bass, Reese tones, and metallic growls.

## What is actually implemented
- 12-voice polyphony and mono/legato with glide
- 2 morphing, band-limited oscillators + an independent sub oscillator
- Frequency/phase modulation; 4x processing for nonlinear synthesis
- Animated resonant filter + dual vowel/formant bandpass filters
- Tempo-synced modulation sourced from the DAW transport
- ADSR, distortion, wet/dry, detune, stereo spread, output control
- 8 editable starting patches, parameter automation and preset/state saving in FL Studio
- Custom plugin editor with rotary controls
- Direct MIDI note handling (no generated rhythm that changes your own MIDI pattern)

## Easiest route: automatic Windows VST3 build on GitHub
Create a **new** GitHub repository for this desktop synth (do not overwrite your existing iPhone Pages app). Upload all contents of this project including `.github/workflows/build-windows.yml`. On the repository’s **Actions** tab, enable workflows if prompted, run **Build RageForge Windows VST3** (or push to `main`), and open the completed run. Download the **RageForge-Windows-VST3** artifact and extract it. The workflow compiles on a Windows build machine; it is not a precompiled plugin in this ZIP. If your project workflow fails, inspect the Actions log and fix the build before installing anything.

## Windows: Build a VST3 for FL Studio
**Requirements:** Visual Studio 2022 or later with **Desktop development with C++**, CMake 3.22+, Git/internet access for JUCE. Build on a Windows PC to get a Windows VST3. JUCE is an external dependency and subject to its own license terms.

Open PowerShell in this project directory:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target RageForge_VST3
```

If your Visual Studio version differs, omit the `-G ... -A x64` portion if CMake discovers your compiler automatically. If you already have JUCE installed locally, append `-DJUCE_DIR=C:/path/to/JUCE` to the configure command instead of downloading it.

The generated VST3 bundle normally lives at `build/RageForge_artefacts/Release/VST3/RageForge.vst3` (exact build tree varies by generator). Copy the complete `.vst3` bundle to `C:\Program Files\Common Files\VST3\` (administrator permission may be required). Open **FL Studio > Options > Manage plugins > Find installed plugins**. Add RageForge from your scanned synths. For a no-install preview, build target `RageForge_Standalone` and run the resulting standalone executable.

## Mac
Install Xcode and CMake, then:
```bash
cmake -S . -B build -G Xcode
cmake --build build --config Release --target RageForge_VST3
```
Place the `.vst3` bundle in `/Library/Audio/Plug-Ins/VST3` or `~/Library/Audio/Plug-Ins/VST3` and rescan in FL Studio.

## How to use
1. Insert the **RageForge VST3 instrument** on an FL Studio Channel Rack channel.
2. Select a starting patch from the top-right menu (Impact Stab, Talking Monster, Cyber Metal, Void Reese, Riddim Chop, Liquid Growl, Laser Shot, Sub Engine).
3. Draw your **own rhythms and notes** in FL Studio's Piano Roll. RageForge follows those MIDI events; it never rearranges them.
4. Turn **FM BITE** and **VOCAL** for dramatic changes; **MOVEMENT** and **LFO RATE** control timbral motion; **SUB LEVEL** supplies independent low end.
5. Export audio using FL Studio's render/export workflow, or consolidate your selected audio clip.

## Testing and limitations
The independent synth DSP unit tests run with `g++ -std=c++17 -O2 tests/dsp_tests.cpp -o dsp_tests && ./dsp_tests` (or an equivalent C++ compiler). These test finite output, note release, variations, stereo/poly operation and clipping ceiling. The **JUCE VST3 itself has not been compiled or listened to in FL Studio** in this environment: there is no JUCE checkout or Windows VST3 build chain here. These are original starting patches, not professionally mastered commercial presets. The design is a foundation for higher fidelity, but cannot yet honestly be called “better than Massive.”

## Next production milestones
Actual VST3 build and FL Studio host validation; preset-by-preset listening comparisons against your reference audio; CPU profiling; anti-alias measurement; true oversampled downsampling stages; mod matrix and LFO curves; modulation visualizer; user patch browser and optional multiband FX.
