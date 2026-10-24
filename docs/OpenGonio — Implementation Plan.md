---
tags:
  - project
  - opengonio
  - audio-plugin
  - plan
created: '2026-10-23'
status: >-
  executing 2026-10-23/24 (native): Tasks 1-13 done and pushed to
  github.com/brady1408/opengonio (A/B session done 2026-10-24, constants in
  docs/tuning-notes.md). Remaining: Task 14 (Windows hand test; needs SSH host
  details), Task 15 (tag v0.1.0)
---
# OpenGonio Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A free, open-source goniometer plugin (VST3/AU/LV2) that shows the stereo field, phase correlation, L/R peak levels and a MONO OK / PHASE ISSUE light, loads as a realtime effect in Audacity 4, and ships as version 0.1.0 on GitHub Releases for macOS, Windows and Linux.

**Architecture:** The audio thread does one thing: copy each block's L/R samples into a lock-free single-producer/single-consumer ring (`SampleFifo`) and pass audio through untouched. The editor's 60 fps timer drains the ring, feeds a JUCE-free analysis core (`StereoAnalyzer`: correlation, energies, peak meters, status light with hysteresis) and the scope's persistence image, then repaints. All DSP is header-only plain C++ with Catch2 unit tests that never link JUCE; the JUCE layer is thin and verified with pluginval plus hand tests in Audacity.

**Tech Stack:** C++20, CMake ≥ 3.22, JUCE 9.0.4 (FetchContent, AGPLv3), Catch2 v3.8.1 (FetchContent, tests only), pluginval v1.0.4 (CI and local), GitHub Actions. macOS: Xcode CLT + Homebrew CMake. Windows: Visual Studio 2022 Build Tools. Linux: GCC 12+.

**Spec:** `personal/projects/OpenGonio — Design Spec.md` (this vault). Read it first; this plan argues from it. Task 1 copies the spec, this plan and the mockup into the repo under `docs/`.

## Global Constraints

- Repo `~/ws/brady1408/opengonio`, GitHub `brady1408/opengonio`, public, default branch `main`. License `AGPL-3.0-or-later` (LICENSE file = full AGPLv3 text).
- Plugin name `OpenGonio`; company `Brady Cole`; bundle id `com.bradycole.opengonio`; manufacturer code `Brdy`; plugin code `Ogon`; LV2 URI `https://github.com/brady1408/opengonio`. Version starts at `0.1.0`, semver, git tags `v0.1.0`.
- Formats: `VST3 AU` on macOS, `VST3` on Windows, `VST3 LV2` on Linux. `Standalone` is added on every platform for development only and is never packaged. No VST2, no AAX.
- Platforms: macOS 11.0+ universal (`CMAKE_OSX_ARCHITECTURES="arm64;x86_64"`, `CMAKE_OSX_DEPLOYMENT_TARGET=11.0`), Windows 10+ x64, Linux x64.
- `src/dsp/` includes nothing from JUCE and nothing outside the C++ standard library. `tests/` links only `src/dsp/` headers plus Catch2. `ctest` must pass with no audio device and no display.
- Audio thread (`processBlock`) never allocates, locks, logs or calls into JUCE GUI. Audio out == audio in, bit-identical. `getLatencySamples()` returns 0. `getTailLengthSeconds()` returns 0.
- Readings and constants pinned by the spec: correlation window 300 ms; status thresholds +0.3 / −0.3 with 500 ms hold; NO SIGNAL below −70 dBFS on both channels; peak meters −60..0 dBFS, release 20 dB/s, peak hold 1.5 s, clip latch 2 s at > −0.1 dBFS; balance shows `C` within ±0.1 dB; width 0 % for mono, 100 % for uncorrelated equal-level noise, 200 % for pure side.
- Scope mapping (fixed scale): `x = (R − L) / 2`, `y = (L + R) / 2`; ±1 spans the box. Auto-gain param `autoGain` (bool, default off) scales by `1 / runningPeak`, clamped to [1, 20].
- Palette: exactly the hex values in the spec's "Palette" table, as constants in `src/ui/Theme.h`. Font: JUCE's default monospaced font for 0.1.0 (my call; a bundled font is 0.2.0 polish).
- Window: default 520 × 360, resizable, limits 400 × 280 to 2000 × 1400; size persisted in plugin state.
- Commits: subject `area: what changed`, factual. End every commit message with `Co-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>`.
- Ko-fi URL: placeholder `https://ko-fi.com/bradycole` in README; **Brady confirms the real URL before Task 13 merges.**

## Review Focus

Behaviours the spec implies but no task's tests pin by default. Each has a test in the owning task.

1. **Sample rate or block size changes mid-session (host switches 44.1 → 96 kHz) keep timings in seconds, not samples.** All time constants recompute in `prepare()`. → Task 6 (`prepare` at 48 kHz then 96 kHz; status hold still 500 ms).
2. **A denormal-level or NaN sample does not poison the correlation or meters.** NaN in → readings stay finite. → Task 3 (`push(NAN, 0.f)` leaves `value()` finite and in [−1, 1]).
3. **A block larger than the FIFO's free space neither overruns memory nor blocks the audio thread.** → Task 2 (push 2× capacity; `size()` ≤ capacity; later `pop` returns the first-written samples).
4. **A full-scale square wave (|x| = 1.0 exactly) shows 0.0 dBFS and trips clip; a −0.2 dBFS sine does not.** → Task 4.
5. **A mono bus layout (1 in / 1 out) loads and shows MONO SOURCE instead of crashing on a missing channel 1.** → Task 7 (pluginval with `--validate-in-process` exercises mono layout) and Task 10 (readings with `monoSource == true` render the tag).

---

## File Structure

```
opengonio/
  CMakeLists.txt                     project, options, FetchContent JUCE + Catch2, juce_add_plugin, add_subdirectory(tests)
  LICENSE                            AGPLv3 text
  README.md                          tagline, screenshots, install per OS incl. Gatekeeper/SmartScreen notes, Ko-fi, build-from-source
  CLAUDE.md                          repo conventions (constraints above), build/test commands
  .gitignore                         build/, .superpowers/, *.user, .DS_Store
  .clang-format                      LLVM base, 4-space indent, 100 cols
  docs/OpenGonio — Design Spec.md    copy of the vault spec
  docs/OpenGonio — Implementation Plan.md  copy of this plan
  docs/layout-mockup.html            the approved mockup
  src/dsp/SampleFifo.h               SPSC ring of stereo frames
  src/dsp/CorrelationMeter.h         EWMA phase correlation
  src/dsp/EnergyTracker.h            EWMA RMS of L, R, M, S + width/balance helpers
  src/dsp/PeakMeter.h                peak, release, hold, clip latch
  src/dsp/StatusLight.h              state machine with hysteresis
  src/dsp/StereoAnalyzer.h           facade: prepare/process/readings
  src/PluginProcessor.h/.cpp         buses, processBlock → fifo, APVTS, state
  src/PluginEditor.h/.cpp            layout, timer, drains fifo, owns components
  src/ui/Theme.h                     palette + font helper
  src/ui/ScopeComponent.h/.cpp       goniometer with persistence image, grid, labels, AUTO tag
  src/ui/LevelMeter.h/.cpp           one horizontal L or R bar with hold tick + dB readout
  src/ui/StatusPill.h/.cpp           MONO OK / CAUTION / PHASE ISSUE / NO SIGNAL / MONO SOURCE
  src/ui/ReadoutPill.h/.cpp          label + value pill (Correlation, Width, Balance)
  src/ui/CorrelationBar.h/.cpp       gradient bar with needle and labels
  tests/CMakeLists.txt               Catch2 target opengonio_tests, catch_discover_tests
  tests/test_SampleFifo.cpp
  tests/test_CorrelationMeter.cpp
  tests/test_EnergyTracker.cpp
  tests/test_PeakMeter.cpp
  tests/test_StatusLight.cpp
  tests/test_StereoAnalyzer.cpp
  packaging/macos/build-pkg.sh       pkgbuild + productbuild → OpenGonio-<ver>-macOS.pkg
  packaging/windows/opengonio.iss    Inno Setup → OpenGonio-<ver>-Windows-Setup.exe
  packaging/linux/install.sh         copies VST3 + LV2 to ~/.vst3 and ~/.lv2
  .github/workflows/build.yml        matrix build, ctest, pluginval, artifacts, release on tag
```

---

### Task 1: Repository scaffold and a pass-through plugin that loads in Audacity 4

**Files:**
- Create: `CMakeLists.txt`, `LICENSE`, `README.md` (stub), `CLAUDE.md`, `.gitignore`, `.clang-format`, `docs/*`, `src/PluginProcessor.h`, `src/PluginProcessor.cpp`, `src/PluginEditor.h`, `src/PluginEditor.cpp`

**Interfaces:**
- Produces: CMake target `OpenGonio` (via `juce_add_plugin`), build dir convention `build/` (Debug) with `COPY_PLUGIN_AFTER_BUILD TRUE`; `class OpenGonioProcessor : public juce::AudioProcessor`; `class OpenGonioEditor : public juce::AudioProcessorEditor`.

- [ ] **Step 1: Install CMake and create the repo**

```bash
brew install cmake
mkdir -p ~/ws/brady1408/opengonio && cd ~/ws/brady1408/opengonio && git init -b main
```

- [ ] **Step 2: Copy the spec, this plan and the mockup into `docs/`** from the vault paths in the header; write `LICENSE` (full AGPLv3 text from https://www.gnu.org/licenses/agpl-3.0.txt), `.gitignore`, `.clang-format`, and `CLAUDE.md` containing the Global Constraints section verbatim plus the build/test commands from Steps 4 and 6.

- [ ] **Step 3: Write `CMakeLists.txt`**

`cmake_minimum_required(VERSION 3.22)`, `project(OpenGonio VERSION 0.1.0 LANGUAGES C CXX)`, `CMAKE_CXX_STANDARD 20`. FetchContent JUCE from `https://github.com/juce-framework/JUCE.git` tag `9.0.4`. Option `OPENGONIO_BUILD_TESTS` default ON. `juce_add_plugin(OpenGonio ...)` with the identifiers from Global Constraints, `FORMATS` chosen per platform (`if(APPLE) VST3 AU Standalone; elseif(WIN32) VST3 Standalone; else() VST3 LV2 Standalone`), `IS_SYNTH FALSE`, `NEEDS_MIDI_INPUT FALSE`, `NEEDS_MIDI_OUTPUT FALSE`, `IS_MIDI_EFFECT FALSE`, `EDITOR_WANTS_KEYBOARD_FOCUS FALSE`, `COPY_PLUGIN_AFTER_BUILD TRUE`, `VST3_CATEGORIES "Fx" "Analyzer"`, `AU_MAIN_TYPE kAudioUnitType_Effect`, `LV2URI https://github.com/brady1408/opengonio`, `PRODUCT_NAME "OpenGonio"`. `juce_generate_juce_header(OpenGonio)`. Sources: the four files in `src/`. Compile definitions: `JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_DISPLAY_SPLASH_SCREEN=0`. Link `juce::juce_audio_utils juce::juce_audio_processors juce::juce_gui_basics` plus `juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags`. `if(OPENGONIO_BUILD_TESTS) enable_testing(); add_subdirectory(tests) endif()` (the `tests/` dir arrives in Task 2; guard with `if(EXISTS)` until then).

- [ ] **Step 4: Write the minimal processor and editor**

`OpenGonioProcessor`: constructor with `BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)`; `isBusesLayoutSupported` accepts stereo→stereo and mono→mono only; `processBlock` does nothing (pass-through); `getLatencySamples()` 0; `getTailLengthSeconds()` 0.0; `hasEditor()` true; `createEditor()` returns `new OpenGonioEditor(*this)`; `getStateInformation` / `setStateInformation` empty for now; `getName()` returns `JucePlugin_Name`. `OpenGonioEditor`: `setSize(520, 360)`, `paint` fills `#0d1117` and draws the text "OpenGonio" centered in `#8b949e`.

- [ ] **Step 5: Configure and build Debug**

Run: `cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build --config Debug -j`
Expected: builds `OpenGonio.vst3`, `OpenGonio.component`, `OpenGonio.app` under `build/OpenGonio_artefacts/Debug/`, and copies the VST3 and AU into `~/Library/Audio/Plug-Ins/VST3` and `~/Library/Audio/Plug-Ins/Components`.

- [ ] **Step 6: Run pluginval locally**

Download `pluginval_macOS.zip` from https://github.com/Tracktion/pluginval/releases/tag/v1.0.4 into `~/ws/bin/` (outside the repo), unzip.
Run: `~/ws/bin/pluginval.app/Contents/MacOS/pluginval --strictness-level 5 --validate-in-process --validate ~/Library/Audio/Plug-Ins/VST3/OpenGonio.vst3`
Expected: output ends with `ALL TESTS PASSED`. Repeat for `~/Library/Audio/Plug-Ins/Components/OpenGonio.component`.

- [ ] **Step 7: Hand check in Audacity 4** (Brady or Claude on the Mac)

Open Audacity 4.0.1, import any stereo file, open the track's Effects panel, Add effect → OpenGonio (if missing: Plugin Manager → Rescan). Expected: the editor opens showing "OpenGonio" on a dark background; playback is unchanged. Note in the commit message whether a rescan was needed.

- [ ] **Step 8: Commit**

```bash
git add -A && git commit -m "scaffold: JUCE 9.0.4 pass-through plugin that loads in Audacity 4"
```

Then create the public GitHub repo and push: `gh repo create brady1408/opengonio --public --source=. --description "Free, open-source goniometer, Lissajous / vectorscope and stereo field meter for Audacity and any VST3, AU or LV2 host" --push`.

---

### Task 2: `SampleFifo` (lock-free stereo ring)

**Files:**
- Create: `src/dsp/SampleFifo.h`, `tests/CMakeLists.txt`, `tests/test_SampleFifo.cpp`
- Modify: `CMakeLists.txt` (remove the `if(EXISTS)` guard)

**Interfaces:**
- Produces: `namespace opengonio::dsp { class SampleFifo { public: explicit SampleFifo(int capacityFrames /*rounded up to power of two*/); int capacity() const; int size() const; int push(const float* l, const float* r, int n); /*returns frames written; drops the tail on overflow*/ int pop(float* l, float* r, int maxFrames); /*returns frames read*/ void clearFromConsumer(); }; }`
- Single producer (`push`, audio thread), single consumer (`pop`, `clearFromConsumer`, GUI thread). `std::atomic<int>` read/write indices with acquire/release; no locks, no allocation after construction.

- [ ] **Step 1: Write `tests/CMakeLists.txt`**

FetchContent Catch2 from `https://github.com/catchorg/Catch2.git` tag `v3.8.1`; `add_executable(opengonio_tests test_SampleFifo.cpp)`; `target_include_directories(... ${CMAKE_SOURCE_DIR}/src)`; `target_link_libraries(opengonio_tests PRIVATE Catch2::Catch2WithMain)`; `include(Catch)`; `catch_discover_tests(opengonio_tests)`.

- [ ] **Step 2: Write the failing tests**

```cpp
TEST_CASE("fifo round-trips frames in order") {
    opengonio::dsp::SampleFifo f(8);
    float l[3] = {1, 2, 3}, r[3] = {-1, -2, -3};
    REQUIRE(f.push(l, r, 3) == 3);
    REQUIRE(f.size() == 3);
    float ol[8], orr[8];
    REQUIRE(f.pop(ol, orr, 8) == 3);
    REQUIRE(ol[2] == 3.f); REQUIRE(orr[0] == -1.f);
    REQUIRE(f.size() == 0);
}
TEST_CASE("fifo drops the tail on overflow and never exceeds capacity") {
    opengonio::dsp::SampleFifo f(8);              // capacity 8
    float l[16], r[16]; for (int i = 0; i < 16; ++i) l[i] = r[i] = (float) i;
    int written = f.push(l, r, 16);
    REQUIRE(written <= f.capacity()); REQUIRE(f.size() <= f.capacity());
    float ol[16], orr[16]; int got = f.pop(ol, orr, 16);
    REQUIRE(got == written); REQUIRE(ol[0] == 0.f);
}
TEST_CASE("capacity rounds up to a power of two") {
    REQUIRE(opengonio::dsp::SampleFifo(100).capacity() == 128);
}
TEST_CASE("clearFromConsumer empties the ring") {
    opengonio::dsp::SampleFifo f(8); float x[4] = {1,1,1,1};
    f.push(x, x, 4); f.clearFromConsumer(); REQUIRE(f.size() == 0);
}
```

- [ ] **Step 3: Run tests to verify they fail**

Run: `cmake -B build -DCMAKE_BUILD_TYPE=Debug && cmake --build build --target opengonio_tests -j && ctest --test-dir build --output-on-failure`
Expected: build FAILS (header missing).

- [ ] **Step 4: Implement `SampleFifo` in `src/dsp/SampleFifo.h`** per the Interfaces block. Storage: two `std::vector<float>` sized to capacity; indices are monotonically increasing counters masked by `capacity − 1`.

- [ ] **Step 5: Run tests to verify they pass**

Run: `cmake --build build --target opengonio_tests -j && ctest --test-dir build --output-on-failure`
Expected: `100% tests passed`.

- [ ] **Step 6: Commit**

```bash
git add src/dsp/SampleFifo.h tests CMakeLists.txt && git commit -m "dsp: lock-free stereo SampleFifo with Catch2 test target"
```

---

### Task 3: `CorrelationMeter`

**Files:**
- Create: `src/dsp/CorrelationMeter.h`, `tests/test_CorrelationMeter.cpp`
- Modify: `tests/CMakeLists.txt` (add source)

**Interfaces:**
- Produces: `class CorrelationMeter { public: void prepare(double sampleRate, double windowMs = 300.0); void push(float l, float r); float value() const; /*[-1,1]; 0 when both energies < 1e-12*/ void reset(); };`
- EWMA with coefficient `a = exp(-1 / (windowMs * 0.001 * sampleRate))` applied to `sumLR`, `sumLL`, `sumRR`. Non-finite inputs are treated as 0.

- [ ] **Step 1: Write the failing tests**

```cpp
static void feedSine(opengonio::dsp::CorrelationMeter& m, float gainR, int n = 48000) {
    for (int i = 0; i < n; ++i) { float s = std::sin(2 * M_PI * 440 * i / 48000.0); m.push(s, gainR * s); }
}
TEST_CASE("identical channels read +1") { CorrelationMeter m; m.prepare(48000); feedSine(m, 1.f);  REQUIRE(m.value() == Approx(1.f).margin(0.01)); }
TEST_CASE("inverted channels read -1") { CorrelationMeter m; m.prepare(48000); feedSine(m, -1.f); REQUIRE(m.value() == Approx(-1.f).margin(0.01)); }
TEST_CASE("independent noise reads near 0") {
    CorrelationMeter m; m.prepare(48000); std::mt19937 g(1); std::uniform_real_distribution<float> d(-1, 1);
    for (int i = 0; i < 96000; ++i) m.push(d(g), d(g));
    REQUIRE(std::abs(m.value()) < 0.1f);
}
TEST_CASE("silence reads 0 and NaN does not poison it") {
    CorrelationMeter m; m.prepare(48000); REQUIRE(m.value() == 0.f);
    m.push(NAN, 0.f); feedSine(m, 1.f); REQUIRE(std::isfinite(m.value())); REQUIRE(m.value() == Approx(1.f).margin(0.01));
}
TEST_CASE("window is about 300 ms: value crosses 0 within 300 ms after a polarity flip") {
    CorrelationMeter m; m.prepare(48000); feedSine(m, 1.f);
    int samples = 0; while (m.value() > 0.f && samples < 48000) { float s = std::sin(2 * M_PI * 440 * samples / 48000.0); m.push(s, -s); ++samples; }
    REQUIRE(samples < 48000 * 0.3);
}
```

- [ ] **Step 2: Run tests to verify they fail** — Run the ctest command from Task 2 Step 5. Expected: build FAILS.

- [ ] **Step 3: Implement `CorrelationMeter`** per the Interfaces block.

- [ ] **Step 4: Run tests to verify they pass** — Expected: `100% tests passed`.

- [ ] **Step 5: Commit** — `git commit -m "dsp: EWMA phase correlation meter"`

---

### Task 4: `EnergyTracker` with width and balance

**Files:**
- Create: `src/dsp/EnergyTracker.h`, `tests/test_EnergyTracker.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces: `class EnergyTracker { public: void prepare(double sampleRate, double windowMs = 300.0); void push(float l, float r); float rmsL() const; float rmsR() const; float rmsM() const; float rmsS() const; void reset(); };` where `m = (l + r) / 2`, `s = (l − r) / 2`, each RMS is `sqrt(EWMA(x²))`.
- Free functions: `float widthPercent(float rmsM, float rmsS); /* 100 * 2 * S / (M + S); 0 when M + S < 1e-9 */` and `float balanceDb(float rmsL, float rmsR); /* 20*log10(L/R), clamped to [-60, 60]; 0 when either < 1e-9 */` and `std::string formatBalance(float balanceDb); /* "C" within ±0.1, else "L 0.4 dB" / "R 1.2 dB" with one decimal */`.

- [ ] **Step 1: Write the failing tests**

```cpp
TEST_CASE("mono gives width 0 and balance C") {
    EnergyTracker e; e.prepare(48000); for (int i = 0; i < 48000; ++i) { float s = std::sin(2*M_PI*440*i/48000.0); e.push(s, s); }
    REQUIRE(widthPercent(e.rmsM(), e.rmsS()) == Approx(0.f).margin(0.5));
    REQUIRE(formatBalance(balanceDb(e.rmsL(), e.rmsR())) == "C");
}
TEST_CASE("pure side gives width 200") {
    EnergyTracker e; e.prepare(48000); for (int i = 0; i < 48000; ++i) { float s = std::sin(2*M_PI*440*i/48000.0); e.push(s, -s); }
    REQUIRE(widthPercent(e.rmsM(), e.rmsS()) == Approx(200.f).margin(1));
}
TEST_CASE("uncorrelated equal-level noise gives width about 100") {
    EnergyTracker e; e.prepare(48000); std::mt19937 g(2); std::uniform_real_distribution<float> d(-1,1);
    for (int i = 0; i < 96000; ++i) e.push(d(g), d(g));
    REQUIRE(widthPercent(e.rmsM(), e.rmsS()) == Approx(100.f).margin(10));
}
TEST_CASE("right channel 6 dB louder reads R 6.0 dB") {
    EnergyTracker e; e.prepare(48000); for (int i = 0; i < 48000; ++i) { float s = std::sin(2*M_PI*440*i/48000.0); e.push(0.5f*s, s); }
    REQUIRE(formatBalance(balanceDb(e.rmsL(), e.rmsR())) == "R 6.0 dB");
}
TEST_CASE("formatBalance edges") {
    REQUIRE(formatBalance(0.09f) == "C"); REQUIRE(formatBalance(0.11f) == "L 0.1 dB"); REQUIRE(formatBalance(-1.24f) == "R 1.2 dB");
}
```

- [ ] **Step 2: Run tests to verify they fail.** — [ ] **Step 3: Implement.** — [ ] **Step 4: Run tests to verify they pass.** — [ ] **Step 5: Commit** `git commit -m "dsp: L/R/M/S energy tracker with width and balance"`

---

### Task 5: `PeakMeter`

**Files:**
- Create: `src/dsp/PeakMeter.h`, `tests/test_PeakMeter.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces: `class PeakMeter { public: void prepare(double sampleRate); void push(float x); float levelDb() const; /* current meter, floor -60 */ float holdDb() const; /* highest in last 1.5 s, floor -60 */ bool clipped() const; /* true for 2 s after |x| > 0.98855 (= -0.1 dBFS) */ void reset(); };`
- Ballistics: instant attack; release 20 dB/s; hold 1.5 s then follows level; clip latch 2 s. All timers are sample counters derived from `prepare`.

- [ ] **Step 1: Write the failing tests**

```cpp
TEST_CASE("full-scale square reads 0 dBFS and clips; -0.2 dBFS sine does not clip") {
    PeakMeter m; m.prepare(48000); for (int i = 0; i < 4800; ++i) m.push(i % 2 ? 1.f : -1.f);
    REQUIRE(m.levelDb() == Approx(0.f).margin(0.01)); REQUIRE(m.clipped());
    PeakMeter n; n.prepare(48000); float g = std::pow(10.f, -0.2f/20.f); for (int i = 0; i < 48000; ++i) n.push(g*std::sin(2*M_PI*1000*i/48000.0));
    REQUIRE_FALSE(n.clipped()); REQUIRE(n.levelDb() == Approx(-0.2f).margin(0.05));
}
TEST_CASE("release is 20 dB per second") {
    PeakMeter m; m.prepare(48000); m.push(1.f); for (int i = 0; i < 48000; ++i) m.push(0.f);
    REQUIRE(m.levelDb() == Approx(-20.f).margin(0.5));
}
TEST_CASE("hold keeps the peak for 1.5 s then drops") {
    PeakMeter m; m.prepare(48000); m.push(1.f);
    for (int i = 0; i < 48000; ++i) m.push(0.f);  REQUIRE(m.holdDb() == Approx(0.f).margin(0.01));
    for (int i = 0; i < 48000; ++i) m.push(0.f);  REQUIRE(m.holdDb() < -0.5f);
}
TEST_CASE("clip latch clears after 2 s") {
    PeakMeter m; m.prepare(48000); m.push(1.f); for (int i = 0; i < 48000 * 2.1; ++i) m.push(0.f); REQUIRE_FALSE(m.clipped());
}
TEST_CASE("silence floors at -60") { PeakMeter m; m.prepare(48000); REQUIRE(m.levelDb() == -60.f); REQUIRE(m.holdDb() == -60.f); }
```

- [ ] **Step 2: Run tests to verify they fail.** — [ ] **Step 3: Implement.** — [ ] **Step 4: Run tests to verify they pass.** — [ ] **Step 5: Commit** `git commit -m "dsp: peak meter with release, hold and clip latch"`

---

### Task 6: `StatusLight` and the `StereoAnalyzer` facade

**Files:**
- Create: `src/dsp/StatusLight.h`, `src/dsp/StereoAnalyzer.h`, `tests/test_StatusLight.cpp`, `tests/test_StereoAnalyzer.cpp`
- Modify: `tests/CMakeLists.txt`

**Interfaces:**
- Produces: `enum class Status { NoSignal, Ok, Caution, Issue };` `class StatusLight { public: void prepare(double sampleRate, double holdMs = 500.0); void push(float correlation, float peakDbL, float peakDbR); /* once per sample */ Status status() const; void reset(); };` Rules: candidate = `NoSignal` if both peaks < −70, else `Ok` if corr > 0.3, `Issue` if corr < −0.3, else `Caution`. The displayed status changes only after the candidate has been stable for `holdMs`. Initial status `NoSignal`.
- Produces: `struct Readings { float correlation; float widthPercent; float balanceDb; float levelDbL, levelDbR, holdDbL, holdDbR; bool clipL, clipR; Status status; bool monoSource; };` `class StereoAnalyzer { public: void prepare(double sampleRate); void setMonoSource(bool); void process(const float* l, const float* r, int n); Readings readings() const; void reset(); };` composing one `CorrelationMeter`, one `EnergyTracker`, two `PeakMeter`, one `StatusLight`. When `monoSource` is true, `process` is still called with `r == l` by the caller; `readings().monoSource` echoes the flag.

- [ ] **Step 1: Write the failing tests**

```cpp
// test_StatusLight.cpp
TEST_CASE("starts NoSignal; silence stays NoSignal") { StatusLight s; s.prepare(48000); for (int i=0;i<48000;++i) s.push(0.f,-80.f,-80.f); REQUIRE(s.status()==Status::NoSignal); }
TEST_CASE("Ok after 500 ms of correlation above 0.3, not before") {
    StatusLight s; s.prepare(48000); for (int i=0;i<48000*0.4;++i) s.push(0.9f,-6.f,-6.f); REQUIRE(s.status()==Status::NoSignal);
    for (int i=0;i<48000*0.2;++i) s.push(0.9f,-6.f,-6.f); REQUIRE(s.status()==Status::Ok);
}
TEST_CASE("Issue below -0.3, Caution between") {
    StatusLight s; s.prepare(48000); for (int i=0;i<48000;++i) s.push(-0.8f,-6.f,-6.f); REQUIRE(s.status()==Status::Issue);
    for (int i=0;i<48000;++i) s.push(0.f,-6.f,-6.f); REQUIRE(s.status()==Status::Caution);
}
TEST_CASE("a 100 ms blip does not flip the state") {
    StatusLight s; s.prepare(48000); for (int i=0;i<48000;++i) s.push(0.9f,-6.f,-6.f);
    for (int i=0;i<4800;++i) s.push(-0.9f,-6.f,-6.f); REQUIRE(s.status()==Status::Ok);
}
// test_StereoAnalyzer.cpp
TEST_CASE("inverted stereo reads Issue, correlation -1, width 200 within a second") {
    StereoAnalyzer a; a.prepare(48000); std::vector<float> l(48000), r(48000);
    for (int i=0;i<48000;++i){ l[i]=0.5f*std::sin(2*M_PI*440*i/48000.0); r[i]=-l[i]; }
    a.process(l.data(), r.data(), 48000); auto R = a.readings();
    REQUIRE(R.status==Status::Issue); REQUIRE(R.correlation==Approx(-1.f).margin(0.01)); REQUIRE(R.widthPercent==Approx(200.f).margin(1));
}
TEST_CASE("timings survive a sample-rate change: hold is 500 ms at 96 kHz too") {
    StereoAnalyzer a; a.prepare(48000); a.prepare(96000); std::vector<float> l(96000*0.4), r(l.size());
    for (size_t i=0;i<l.size();++i){ l[i]=std::sin(2*M_PI*440*i/96000.0); r[i]=l[i]; }
    a.process(l.data(), r.data(), (int)l.size()); REQUIRE(a.readings().status==Status::NoSignal);
    a.process(l.data(), r.data(), (int)l.size()); REQUIRE(a.readings().status==Status::Ok);
}
TEST_CASE("monoSource flag is echoed") { StereoAnalyzer a; a.prepare(48000); a.setMonoSource(true); REQUIRE(a.readings().monoSource); }
```

- [ ] **Step 2: Run tests to verify they fail.** — [ ] **Step 3: Implement both headers.** — [ ] **Step 4: Run tests to verify they pass.** — [ ] **Step 5: Commit** `git commit -m "dsp: status light with hysteresis and StereoAnalyzer facade"`

---

### Task 7: Processor wiring: FIFO, parameters, state

**Files:**
- Modify: `src/PluginProcessor.h`, `src/PluginProcessor.cpp`

**Interfaces:**
- Consumes: `SampleFifo` (Task 2).
- Produces: `OpenGonioProcessor` members: `juce::AudioProcessorValueTreeState apvts;` with one `AudioParameterBool` id `"autoGain"`, name `"Auto Gain"`, default `false`; `opengonio::dsp::SampleFifo& fifo();` (capacity = 2 × sample rate frames, re-created in `prepareToPlay` only if the capacity would change; initial 96000); `double currentSampleRate() const;` `bool isMonoInput() const;` (true when the main input bus has 1 channel); `juce::Point<int> editorSize() const; void setEditorSize(juce::Point<int>);` stored as properties `"editorW"`, `"editorH"` on `apvts.state`, default 520 × 360.
- `processBlock`: read L (channel 0) and R (channel 1, or channel 0 again when mono) from the input, `fifo().push(l, r, numSamples)`, nothing else. `getStateInformation` writes `apvts.copyState()` as XML; `setStateInformation` restores it.

- [ ] **Step 1: Implement the Interfaces block** in the processor. `prepareToPlay(sampleRate, maxBlock)` stores the rate; buffer re-creation there is allowed (not in `processBlock`).

- [ ] **Step 2: Build and run pluginval at strictness 5 on VST3 and AU** (commands from Task 1 Step 6).
Expected: `ALL TESTS PASSED` for both, including the state-restore and mono-layout sections pluginval prints.

- [ ] **Step 3: Run the unit tests** (`ctest`) — Expected: still `100% tests passed`.

- [ ] **Step 4: Commit** `git commit -m "processor: feed SampleFifo from processBlock, autoGain param, editor size in state"`

---

### Task 8: `Theme` and `ScopeComponent`

**Files:**
- Create: `src/ui/Theme.h`, `src/ui/ScopeComponent.h`, `src/ui/ScopeComponent.cpp`
- Modify: `CMakeLists.txt` (add sources), `src/PluginEditor.*` (temporarily show only the scope, full-window, to see it)

**Interfaces:**
- Produces: `namespace opengonio::theme { inline const juce::Colour windowBg{0xff0d1117}, scopeBg{0xff000000}, grid{0xff21262d}, panelBg{0xff161b22}, panelBorder{0xff30363d}, textMuted{0xff8b949e}, textPrimary{0xffe6edf3}, green{0xff3fb950}, greenHot{0xff7ee787}, amber{0xffd29922}, red{0xfff85149}, okBg{0xff12331f}, okBorder{0xff238636}, issueBg{0xff3d1d1d}, issueBorder{0xffda3633}, white{0xffffffff}; juce::Font mono(float height); }`
- Produces: `class ScopeComponent : public juce::Component { public: void pushFrames(const float* l, const float* r, int n); /* GUI thread; appends points to the persistence image */ void setAutoGain(bool); void decay(); /* called once per timer tick before pushFrames */ void paint(juce::Graphics&) override; void resized() override; };`
- Rendering: a `juce::Image` (ARGB, square, re-created in `resized()`) holds the persistence layer. `decay()` fills it with `scopeBg.withAlpha(kDecayAlpha)`; `kDecayAlpha` starts at `0.18f` (so a trace fades over ~300 ms at 60 fps; tuned in Task 11). `pushFrames` maps each frame to `(x, y)` per Global Constraints, applies auto-gain if enabled (running peak with release 0.5 s, scale clamped [1, 20]), and draws 1-px line segments between consecutive points in `green` with the last 64 points overdrawn in `greenHot`. `paint` draws `scopeBg`, the grid (axes, both diagonals, circle r = 0.7 of half-size) in `grid`, labels `M L R S` in `textMuted`, the persistence image, and the `AUTO` tag (bottom-right, `textMuted`) when enabled.

- [ ] **Step 1: Implement `Theme.h` and `ScopeComponent`** per the Interfaces block.

- [ ] **Step 2: Wire a temporary editor**: a `juce::Timer` at 60 Hz drains `fifo()` into a local `std::vector<float>` pair (reserve 8192), calls `scope.decay()` then `scope.pushFrames(...)`, and `repaint()`s. The editor shows only the scope filling the window.

- [ ] **Step 3: Build and look at it in the Standalone app and in Audacity 4** with a stereo music file. Expected: a glowing cloud; a mono file draws a vertical line; Audacity's Invert effect on one channel turns it horizontal. Take a screenshot into `docs/screenshots/scope-first-light.png` for the README.

- [ ] **Step 4: Commit** `git commit -m "ui: theme and goniometer scope with persistence"`

---

### Task 9: Meter, pills and correlation bar components

**Files:**
- Create: `src/ui/LevelMeter.h/.cpp`, `src/ui/StatusPill.h/.cpp`, `src/ui/ReadoutPill.h/.cpp`, `src/ui/CorrelationBar.h/.cpp`
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces: `class LevelMeter : public juce::Component { public: explicit LevelMeter(juce::String label /* "L" or "R" */); void setLevels(float levelDb, float holdDb, bool clipped); };` draws label and `"-6.2 dB"` readout above a `panelBg` track with `panelBorder`, a fill from −60 to `levelDb` (green to −6, amber to −1, red above, as a horizontal gradient), a 2-px `white` tick at `holdDb`, and the fill end turned `red` while `clipped`. Readout prints `"-inf"` below −59.9.
- Produces: `class StatusPill : public juce::Component { public: void setStatus(opengonio::dsp::Status, bool monoSource); };` text and colours: `MONO OK` (okBg/okBorder/green), `CAUTION` (panelBg/amber border/amber text), `PHASE ISSUE` (issueBg/issueBorder/red), `NO SIGNAL` (panelBg/panelBorder/textMuted), and `MONO SOURCE` (panelBg/panelBorder/textMuted) overriding all when `monoSource`.
- Produces: `class ReadoutPill : public juce::Component { public: explicit ReadoutPill(juce::String label); void setValue(juce::String); void setValueColour(juce::Colour); };` label left in `textMuted`, value right in `textPrimary` (or the override), `panelBg`/`panelBorder`, 8-px corners.
- Produces: `class CorrelationBar : public juce::Component { public: void setValue(float correlation); };` horizontal gradient `red → amber (35 %) → green (70 %) → green`, 6-px corners, 3-px `white` needle at `(v + 1) / 2`, labels `-1`, `phase correlation`, `+1` beneath in `textMuted`.

- [ ] **Step 1: Implement the four components.**

- [ ] **Step 2: Build; temporarily add all four under the scope in the editor with fixed test values** (`setLevels(-6.2f, -4.f, false)`, `setStatus(Status::Ok, false)`, `setValue("+0.72")`, `setValue(0.72f)`) and compare against `docs/layout-mockup.html` side by side. Expected: same colours and proportions as the mockup's right column and bottom bar.

- [ ] **Step 3: Commit** `git commit -m "ui: level meters, status pill, readout pills, correlation bar"`

---

### Task 10: Editor layout, live wiring, resize persistence

**Files:**
- Modify: `src/PluginEditor.h`, `src/PluginEditor.cpp`

**Interfaces:**
- Consumes: everything from Tasks 7–9; `StereoAnalyzer` (Task 6).
- Produces: `OpenGonioEditor` owning one `StereoAnalyzer` (prepared with `processor.currentSampleRate()` on construction and whenever the rate changes), the scope, two `LevelMeter`s, one `StatusPill`, three `ReadoutPill`s (`Correlation`, `Width`, `Balance`), one `CorrelationBar`, a header strip and a footer label `"OpenGonio 0.1.0"` (left) + `"github.com/brady1408/opengonio"` (right, `textMuted`, opens the URL on click). `juce::Timer` at 60 Hz: drain fifo → `analyzer.process` → `scope.decay(); scope.pushFrames` → push `readings()` into every component → `repaint()`.

- [ ] **Step 1: Layout in `resized()`**: header 22 px; footer 18 px; correlation bar block 40 px above the footer; body = remainder; scope = square of side `body.height` at the left; column = rest minus 12-px gaps, minimum 180 px; column contents via `juce::FlexBox` column with `justifyContent = spaceBetween`: StatusPill (36 px), meters group (two meters, 44 px each), three ReadoutPills (40 px each). `setResizable(true, true); setResizeLimits(400, 280, 2000, 1400);` initial size from `processor.editorSize()`; `resized()` calls `processor.setEditorSize(...)`. On construction call `processor.fifo().clearFromConsumer()` so stale audio isn't replayed.

- [ ] **Step 2: Live values**: correlation pill shows `juce::String::formatted("%+.2f", c)` in `red` when `c < -0.3`, `amber` when `|c| ≤ 0.3`, else `textPrimary`; width pill shows `"%.0f%%"`; balance pill shows `formatBalance`. Meters get `levelDbL/R`, `holdDbL/R`, `clipL/R`. StatusPill gets `status, monoSource`. Scope gets `autoGain` from `apvts.getRawParameterValue("autoGain")` each tick. Clicking the `AUTO` corner of the scope toggles the parameter (via `apvts.getParameter("autoGain")->setValueNotifyingHost`).

- [ ] **Step 3: Build, run pluginval at strictness 5 on VST3 and AU** — Expected: `ALL TESTS PASSED` (editor open/close and resize are exercised).

- [ ] **Step 4: Hand check in Audacity 4**: play a stereo file → `MONO OK`; apply Invert to one channel → `PHASE ISSUE` within a second; a mono track → `MONO SOURCE`; resize the window, close and reopen the project → size persists. Save `docs/screenshots/editor-0.1.0.png`.

- [ ] **Step 5: Commit** `git commit -m "editor: full layout wired to the analyzer, resizable, size persisted"`

---

### Task 11: Visual A/B tuning with Brady (manual)

**Files:**
- Modify: `src/ui/ScopeComponent.cpp` (constants), `src/dsp/StatusLight.h` (thresholds only if Brady asks), `docs/tuning-notes.md`

- [ ] **Step 1: Build two Standalone/VST3 variants per question** and run them side by side in Audacity 4 on the Mac with Brady watching (per the dotamp lesson: don't reason your way to "better"). Questions, in order: persistence decay (`kDecayAlpha` 0.12 / 0.18 / 0.28), trace line width (1 px / 1.5 px), hot-core length (32 / 64 / 128 points), auto-gain release (0.5 s / 1.5 s), status thresholds on real dialogue and music stems.

- [ ] **Step 2: Record each decision and why in `docs/tuning-notes.md`**; set the constants.

- [ ] **Step 3: Run `ctest`** — Expected: `100% tests passed` (threshold changes must update the tests in Task 6 in the same commit).

- [ ] **Step 4: Commit** `git commit -m "ui: tuned persistence and ballistics from A/B session"`

---

### Task 12: CI on GitHub Actions

**Files:**
- Create: `.github/workflows/build.yml`

**Interfaces:**
- Produces: workflow `build` on `push`, `pull_request`, and tags `v*`. Matrix: `macos-14` (universal, `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0`), `windows-2022`, `ubuntu-22.04` (apt: `libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libxext-dev libfreetype6-dev libfontconfig1-dev libgl1-mesa-dev libegl1-mesa-dev`). Steps per job: checkout → configure `Release` → build → `ctest` → download pluginval v1.0.4 for the runner OS and run strictness 5 on the VST3 (and AU on macOS) with `--skip-gui-tests` on Linux → upload artifact `OpenGonio-<os>` containing the built VST3/AU/LV2 bundles (never the Standalone). On a `v*` tag, a final `release` job downloads all artifacts, runs the packaging scripts from Task 13, and creates a GitHub Release with `softprops/action-gh-release` attaching the package files.

- [ ] **Step 1: Write the workflow.** Cache `build/_deps` keyed on the JUCE and Catch2 tags.

- [ ] **Step 2: Push and watch**: `git push` then `gh run watch`. Expected: all three jobs green; artifacts listed on the run.

- [ ] **Step 3: Commit** (the workflow itself) `git commit -m "ci: build, test and pluginval on macOS, Windows and Linux"`

---

### Task 13: Packaging and README

**Files:**
- Create: `packaging/macos/build-pkg.sh`, `packaging/windows/opengonio.iss`, `packaging/linux/install.sh`
- Modify: `README.md`, `.github/workflows/build.yml` (release job calls these)

**Interfaces:**
- Produces: `build-pkg.sh <version> <vst3 path> <component path> <out dir>` → `OpenGonio-<version>-macOS.pkg` with two components installing to `/Library/Audio/Plug-Ins/VST3` and `/Library/Audio/Plug-Ins/Components` (unsigned for 0.1.0; a `--sign <identity>` flag is parsed and ignored until 0.2.0). `opengonio.iss` → `OpenGonio-<version>-Windows-Setup.exe` installing `OpenGonio.vst3` to `{commoncf64}\VST3`, plus the CI also zips the bare bundle as `OpenGonio-<version>-Windows-VST3.zip`. Linux: `OpenGonio-<version>-Linux.tar.gz` holding `OpenGonio.vst3/`, `OpenGonio.lv2/`, `install.sh` (copies to `~/.vst3` and `~/.lv2`).
- README sections, in order: tagline (the exact sentence from the spec's Decisions table), screenshot, **Install** per OS with the unsigned-build notes verbatim: macOS — "This 0.1.0 build is not yet notarized. After installing, if Audacity reports the plugin is damaged or blocked, open System Settings → Privacy & Security and click Open Anyway, or run `xattr -dr com.apple.quarantine ~/Library/Audio/Plug-Ins/VST3/OpenGonio.vst3`"; Windows — "SmartScreen may show 'Windows protected your PC'. Click More info → Run anyway."; Linux — "untested by the author; reports welcome." Then **Using it in Audacity** (Effects panel, realtime, Plugin Manager rescan), **What the display means** (the spec's "What each element means" condensed), **Build from source**, **Support** (Ko-fi link), **License** (AGPL-3.0).

- [ ] **Step 1: Write the three packaging scripts and run each locally where possible** (mac on the Mac; Windows `.iss` compiled on the Windows machine over SSH with `iscc`; Linux script by inspection + CI).
Expected: `installer -pkg OpenGonio-0.1.0-macOS.pkg -target CurrentUserHomeDirectory` leaves both bundles in place and Audacity still loads them.

- [ ] **Step 2: Write the README** per the Interfaces block; confirm the Ko-fi URL with Brady.

- [ ] **Step 3: Wire the release job** in `build.yml` and dry-run it with a pre-release tag `v0.1.0-rc1`: `git tag v0.1.0-rc1 && git push --tags`. Expected: a GitHub pre-release with four assets (pkg, exe, zip, tar.gz). Delete the rc tag and release afterwards.

- [ ] **Step 4: Commit** `git commit -m "release: macOS pkg, Windows installer, Linux tarball, README install notes"`

---

### Task 14: Windows hand test over SSH (manual)

**Files:**
- Modify: `docs/tuning-notes.md` (Windows findings), code fixes as needed

- [ ] **Step 1: Get the Windows host details from Brady** (SSH host, user, where Audacity is installed, whether VS 2022 Build Tools are present). Record them in the vault, not the repo.

- [ ] **Step 2: On the Windows box**: clone, `cmake -B build -G "Visual Studio 17 2022" -A x64`, `cmake --build build --config Release`, `ctest --test-dir build -C Release`, run `pluginval` from `pluginval_Windows.zip` on the VST3. Expected: all pass. Install the rc Setup.exe from Task 13 instead if CI already produced it.

- [ ] **Step 3: In Audacity on Windows** (Brady, or Claude Code running there with this plan's Task 10 Step 4 checklist as its prompt): same checks as Task 10 Step 4. Expected: identical behaviour; note any font/DPI differences.

- [ ] **Step 4: Fix anything found, re-run CI, commit** `git commit -m "windows: <what changed>"`

---

### Task 15: Release 0.1.0

- [ ] **Step 1: Final pass**: `ctest` green, pluginval green on Mac (VST3 + AU) and Windows (VST3), README screenshots current, `CMakeLists.txt` version `0.1.0`, CHANGELOG section in README or `CHANGELOG.md` listing what 0.1.0 does and that it is unsigned.

- [ ] **Step 2: Tag and release**: `git tag -a v0.1.0 -m "OpenGonio 0.1.0" && git push --tags`. Expected: the release job publishes `OpenGonio-0.1.0-macOS.pkg`, `OpenGonio-0.1.0-Windows-Setup.exe`, `OpenGonio-0.1.0-Windows-VST3.zip`, `OpenGonio-0.1.0-Linux.tar.gz`.

- [ ] **Step 3: Fresh-machine check**: on the Windows box, download the Setup.exe from the release page (not the local build), install, open Audacity, confirm. On the Mac, download the pkg, follow the README's Gatekeeper note exactly as written, confirm it works.

- [ ] **Step 4: Update the vault**: spec status → "0.1.0 released <date>"; `personal/Tasks.md` → add the 0.2.0 items (Apple Developer ID steps from [[dotamp — Apple Signing Setup]], CI notarization, Audacity directory + KVR submissions).

---

## Self-review notes (2026-10-23)

- Spec coverage: window/elements → Tasks 8–10; DSP semantics → Tasks 3–6; host behaviour → Tasks 7, 10; milestones/packaging/README workarounds → Tasks 13, 15; testing section → Tasks 2–7 (unit), 1/7/10/12 (pluginval), 10/14/15 (hand tests), 11 (A/B). Linux ships untested → Task 12 builds it, Task 13 packages it, README says so.
- Deviations from the spec, all my calls: JUCE 9.0.4 instead of 8 (spec patched); system monospaced font instead of a bundled one for 0.1.0; a development-only Standalone format; width formula fixed as `200·S/(M+S)` (spec left the curve open but pinned 0 / ~100 / >100, which this satisfies).
- Open for Brady: the Ko-fi URL (Task 13) and the Windows SSH host details (Task 14).
