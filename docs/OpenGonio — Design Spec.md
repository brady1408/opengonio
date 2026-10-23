---
tags:
  - project
  - opengonio
  - audio-plugin
  - spec
created: '2026-10-23'
status: >-
  approved 2026-10-23; implementation plan in [[OpenGonio — Implementation
  Plan]]
---
# OpenGonio — Design Spec

A free, open-source goniometer (Lissajous / vectorscope / stereo field meter) audio plugin for Audacity and any VST3, AU or LV2 host. No account, no bundle, no social-share gate: one installer per platform that does one thing.

Brainstormed in conversation 2026-10-22/23. Layout chosen from browser mockups (layout "B refined"); the approved mockup is saved at `personal/projects/attachments/OpenGonio layout mockup.html` in this vault and gets copied into the repo's `docs/` at scaffold time. The written description below is sufficient to build from without it.

## Goal

Let a post-production sound engineer open one small window inside Audacity, press play, and see at a glance whether the stereo track is in phase, mono-safe, and balanced. The single most important moment: catching an inverted or out-of-phase channel before the mix goes to delivery.

**Success for v1:** install from a GitHub release on a Mac or Windows PC, add OpenGonio as a realtime effect in Audacity 4, press play, and watch the scope, the correlation bar, the L/R meters and the MONO OK / PHASE ISSUE light respond live. Flip one channel's polarity and the light goes red within a second. Audio passes through untouched.

## Who it is for

- Post-production sound engineers working on the audio track of a video, using Audacity for at least part of the job. These are the people who asked for it.
- Anyone who wants to confirm a stereo file isn't polarity-inverted: podcasters, voice people, students.
- Works the same in Reaper, Ardour, Logic, Live and any other VST3/AU/LV2 host; Audacity is the primary test target, not the only one.

## Why build another one

Every existing free goniometer has a catch: ToneBoosters bundles trialware in its installer, MAAT GŌN needs registration, PSP needs an account, HoRNet needs a social share. Audacity has no native one and the team has said one is unlikely. The open-source goniometers (x42, LSP) are Linux/LV2-first and awkward on Mac and Windows. The gap is a polished, signed, cross-platform, no-strings build of a simple idea.

## Non-goals (v1)

- Spectral / per-band balance view (the ToneBoosters paid feature). Later, maybe.
- Any audio processing: no width control, no M/S, no polarity flip. OpenGonio only looks.
- AAX (Pro Tools). Needs Avid's SDK and iLok signing. Later if asked.
- CLAP. Easy to add with clap-juce-extensions; v2.
- Light theme or user theming. Fixed dark scope look.
- Loudness (LUFS) metering. Other tools do that well.
- VST2. Audacity's realtime effects don't load it and Steinberg no longer licenses it.

## Anti-goals (what would make this a failure even if it works)

- Any install step that isn't "download, run installer, open Audacity" (from 0.2.0 on; 0.1.0 ships unsigned, see Release milestones). No account, no newsletter, no bundled extras, no telemetry, ever.
- A scope that looks like a scribble. Per the dotamp lesson: perception beats correctness in a visualizer; visual tuning is A/B'd with Brady watching, not reasoned out.
- Adding latency or altering audio in any host.
- Crashing or hanging Audacity. A meter that takes the host down is worse than no meter.

## The window

Default size about 520 × 360, freely resizable; the scope stays square and the side column takes the remaining width. Dark theme only.

Left: the goniometer, as large as the window allows.
Right: a column that stretches to the scope's full height, contents spread evenly top to bottom:

1. **Status light** — a full-width pill: `MONO OK` (green), `CAUTION` (amber), `PHASE ISSUE` (red), `NO SIGNAL` (grey).
2. **L and R level meters** — two horizontal bars with peak hold and a dBFS readout to the right of each label.
3. **Correlation** readout (`+0.72`).
4. **Width** readout (`38%`).
5. **Balance** readout (`L 0.4 dB`, `R 1.2 dB`, or `C`).

Below both: a full-width **phase correlation bar**, −1 on the left to +1 on the right, red → amber → green gradient, white needle, labels `-1`, `phase correlation`, `+1` underneath.

Header strip above everything: `GONIOMETER` on the left, `STEREO FIELD` on the right, small uppercase letter-spaced grey text. A small, quiet footer/corner holds the version and a link to the project page (Ko-fi lives on the project page and in the README, not in the plugin's face).

### Palette (from the approved mockup; starting point, tuned by A/B)

| Use | Hex |
|---|---|
| Window background | `#0d1117` |
| Scope background | `#000000` |
| Scope grid lines | `#21262d` |
| Panel / meter track background | `#161b22` |
| Panel border | `#30363d` |
| Muted label text | `#8b949e` |
| Primary readout text | `#e6edf3` |
| Trace / OK green | `#3fb950` (outer glow at ~18–30% alpha), hot core `#7ee787` |
| Amber | `#d29922` |
| Red | `#f85149` |
| Status OK pill | bg `#12331f`, border `#238636`, text `#3fb950` |
| Status issue pill | bg `#3d1d1d`, border `#da3633`, text `#f85149` |
| Peak-hold tick, correlation needle | `#ffffff` |

Font: a monospace UI face (the mockup used the system monospace; builder picks a bundled open font such as JetBrains Mono or Inter for readouts).

## What each element means

**Goniometer.** Classic Lissajous: L on one diagonal axis, R on the other, rotated 45° so mono (L = R) draws a vertical line, side-only (L = −R) draws a horizontal line, L-only leans one way and R-only the other. Grid: vertical and horizontal axes, both diagonals, one circle, axis labels M (top), S (bottom), L (left), R (right). Trace drawn with phosphor-style persistence and decay, so a tone reads as a steady line and music as a glowing cloud. Fixed scale: 0 dBFS reaches the box edge. **Auto-gain toggle** (off by default, my call) normalizes the cloud so quiet material still shows a shape; when on, a small `AUTO` tag shows on the scope.

**Correlation.** Standard phase correlation: Σ(L·R) / √(Σ L² · Σ R²) over a short exponentially weighted window (~300 ms, builder's call within 100–500 ms). +1 identical, 0 uncorrelated, −1 inverted.

**Status light.** Driven by correlation with hysteresis so it doesn't flicker: green above +0.3, amber between −0.3 and +0.3, red below −0.3, each change requiring the new state to hold ~500 ms. `NO SIGNAL` when both channels sit below about −70 dBFS. Thresholds are my call and will be tuned with Brady on real material.

**L/R meters.** Sample-peak meters, −60 to 0 dBFS, fast attack, release around 20 dB/s, 1.5 s peak hold shown as a thin white tick with the held value in the readout. Clip indicator (bar turns red at the end) latches for 2 s above −0.1 dBFS.

**Width.** 0% when L and R are identical, ~100% when fully uncorrelated or wider; derived from side-vs-mid RMS energy. Exact curve is the builder's call; it must read 0 for mono and climb past 100 for out-of-phase material.

**Balance.** RMS level difference between channels in dB, shown as `L 0.4 dB` / `R 1.2 dB`, or `C` when within ±0.1 dB.

**Mono input.** If the host hands over one channel, treat R = L and show a small `MONO SOURCE` tag in place of the status light.

## Host behaviour

- Audio passes through bit-identical. Zero latency reported.
- Audio thread never allocates or locks; samples go to the GUI through a lock-free FIFO. The GUI pulls on a timer (~60 fps) and does all drawing. With the editor closed, nothing is drawn.
- Stopped transport: the trace decays to nothing, meters fall, readouts hold their last value.
- Works in Audacity 4 (and 3.2+) as a realtime effect on a track or the master bus, and shows up under Effects, not Generators/Analyzers (Audacity only runs realtime plugins from the Effects panel).
- Persisted state: window size, auto-gain toggle, persistence length if we expose it. Saved through normal plugin state so a project remembers it.

## Decisions

| Decision | Choice | Why |
|---|---|---|
| Name | OpenGonio | Contains the word people search for; nothing else by that name exists. Tagline carries the other names: "Free, open-source goniometer, Lissajous / vectorscope and stereo field meter for Audacity and any VST3, AU or LV2 host." |
| Framework | JUCE 9.0.4 via CMake (was "JUCE 8" in the draft; 9.0.4 is current as of 2026-10-23 and its breaking changes don't touch plugin code) | One C++ codebase → VST3, AU, LV2 on all three platforms; free for open source; the standard for this kind of plugin. iPlug2 considered; JUCE's maturity and LV2 export win. |
| License | AGPL-3.0 | Confirmed 2026-10-23 against JUCE's LICENSE.md: "The JUCE Framework modules are dual-licensed under the AGPLv3 and the commercial JUCE licence." |
| Formats | VST3 + AU on macOS, VST3 on Windows, VST3 + LV2 on Linux | Everything Audacity's realtime effects load, nothing it doesn't. |
| Platforms | macOS universal (arm64 + x86_64, 11+), Windows x64 (10+), Linux x64 | Linux ships untested at first; the community verifies. |
| Repo | `~/ws/brady1408/opengonio`, GitHub `brady1408/opengonio`, public | Matches Brady's other repos. |
| Distribution | GitHub Releases, then submit to the Audacity plugin directory and KVR | Where the people searching will look. |
| macOS install | `.pkg` installing VST3 and AU to the user's or system plugin folders. **0.1.0 ships unsigned** with the Gatekeeper workaround (right-click → Open, or System Settings → Privacy & Security → Open Anyway) documented in the README and on the release page. **0.2.0 adds signing + notarization** once Brady completes the Apple Developer ID steps from [[dotamp — Apple Signing Setup]] (cert never created; CSR exists). | Brady's decision 2026-10-23: get a working release out first; CI stays simple; signing is its own milestone. |
| Windows install | Installer (Inno Setup, builder's call) placing the VST3 in `C:\Program Files\Common Files\VST3`, plus a bare zip | Unsigned at first (SmartScreen "Run anyway"); Azure Trusted Signing (~$10/mo) when it earns it. |
| Linux install | tar.gz with VST3 and LV2 bundles and a one-line install script | Standard for Linux plugins. |
| CI | GitHub Actions: build all three platforms on every push, run tests and pluginval, attach artifacts to tagged releases | So a release is a tag, not a ritual. |
| Support | Ko-fi link in README and project page | Free with optional thanks. |
| Testing hosts | Audacity 4.0.1 on Brady's Mac (primary); Audacity on Brady's Windows machine over SSH, driven by Brady or by Claude running there; Reaper (free eval) as the second-opinion host | Cross-host bugs show up early. |

## Release milestones

- **0.1.0** — everything in this spec, unsigned on macOS and Windows, Linux untested. Published on GitHub Releases with install notes including the Gatekeeper and SmartScreen workarounds. Not yet submitted to the Audacity directory or KVR.
- **0.2.0** — signed and notarized macOS `.pkg` (Brady's Apple steps, then CI notarization with the key in GitHub Actions secrets). Submit to the Audacity plugin directory and KVR once the Mac install is warning-free.
- **Later** — Windows signing via Azure Trusted Signing when it earns it; v2 ideas below.

The anti-goal "no install step beyond download, run, open Audacity" is the 0.2.0 bar, not the 0.1.0 bar. 0.1.0 is explicitly a working-but-unsigned release.

## Constraints

- Free forever, open source, no telemetry, no network calls.
- Must build from a clean clone with CMake and a compiler; JUCE fetched as a submodule or via CMake FetchContent.
- Nothing in the repo that is secret: signing keys and the Apple API key stay in `~/Library/` on the Mac and in GitHub Actions secrets.
- Brady's Mac has Xcode command-line tools but no CMake yet (`brew install cmake` is the one setup step).

## Testing

- **Unit tests (DSP, no host):** identical L/R → correlation +1, width 0, balance C; inverted → −1 and red status after hysteresis; independent noise → ~0 and amber; silence → NO SIGNAL; one channel only → balance reads that side; peak meter hits −0.1 dBFS on a full-scale sine; mono-input path.
- **pluginval** at strictness 5+ in CI for VST3 and AU: catches state, threading and reset bugs hosts hit.
- **Manual host checks before each release:** Audacity 4 Mac and Windows (add as realtime effect, play, invert a channel with Audacity's Invert effect, confirm red), Reaper, and one LV2 host on Linux when available.
- **Visual A/B:** every change to persistence, decay, colors or ballistics gets an A/B build with Brady watching, per the dotamp visualizer lesson.

## Left to the builder

- Source layout, class structure, and whether the scope draws with JUCE's software renderer or OpenGL (software first; OpenGL only if the frame rate suffers at large sizes).
- Exact smoothing constants, decay curves, fonts, pixel spacing. Start from the palette above and tune by A/B.
- Installer tooling on Windows and the `.pkg` build on macOS.
- Whether persistence length is a user setting in v1 or a fixed good default.
- Branch/release naming, version scheme (semver, start at 0.1.0).

## Later (v2 ideas, not promised)

- Per-band L/R balance strip (spectral balance).
- CLAP build.
- User-pickable trace color / light theme.
- Resizable scope-only "mini" mode for a corner of the screen.

Related: [[dotamp — Apple Signing Setup]], [[dotamp session gotchas (2026-10-07 to 09)]].
