# OpenGonio — repo conventions

Free, open-source goniometer, Lissajous / vectorscope and stereo field meter for Audacity and any VST3, AU or LV2 host. Spec: `docs/OpenGonio — Design Spec.md`. Plan: `docs/OpenGonio — Implementation Plan.md`.

## Build and test

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug -j
ctest --test-dir build --output-on-failure
~/ws/bin/pluginval.app/Contents/MacOS/pluginval --strictness-level 5 --validate-in-process --validate ~/Library/Audio/Plug-Ins/VST3/OpenGonio.vst3
```

Release builds: `-DCMAKE_BUILD_TYPE=Release`. On macOS add `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0`.

## Constraints (from the plan's Global Constraints)

- Repo `brady1408/opengonio`, default branch `main`, license AGPL-3.0-or-later.
- Plugin `OpenGonio`; company `Brady Cole`; bundle id `com.bradycole.opengonio`; manufacturer code `Brdy`; plugin code `Ogon`; LV2 URI `https://github.com/brady1408/opengonio`. Semver from `0.1.0`, tags `vX.Y.Z`.
- Formats: `VST3 AU` on macOS, `VST3` on Windows, `VST3 LV2` on Linux. `Standalone` is dev-only and never packaged. No VST2, no AAX.
- `src/dsp/` includes nothing from JUCE and nothing outside the C++ standard library. `tests/` links only `src/dsp/` headers plus Catch2. `ctest` must pass with no audio device and no display.
- `processBlock` never allocates, locks, logs or touches the GUI. Audio out == audio in. Latency 0, tail 0.
- Spec numbers: correlation window 300 ms; status thresholds OK above +0.1 / issue below −0.2 with 500 ms hold (tuned 2026-10-24, see docs/tuning-notes.md); NO SIGNAL below −70 dBFS; peak meters −60..0 dBFS, release 20 dB/s, hold 1.5 s, clip latch 2 s at > −0.1 dBFS; balance `C` within ±0.1 dB; width = 200·S/(M+S).
- Scope mapping: `x = (R − L) / 2`, `y = (L + R) / 2`; auto-gain scale `1/runningPeak` clamped [1, 20].
- Palette: exactly the spec's hex table, in `src/ui/Theme.h`.
- Window default 520 × 360, resizable 400 × 280 .. 2000 × 1400, size persisted in state.
- Commits: `area: what changed`, factual. End every commit message with `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`.
- Visual tuning is A/B'd with Brady watching, never reasoned out.
