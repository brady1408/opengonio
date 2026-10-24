# Changelog

## 0.2.0 — 2026-10-24

- **macOS builds are now signed and notarized.** The pkg installs with no Gatekeeper prompt; the bundles carry a Developer ID signature with hardened runtime.
- Every package (macOS pkg, Windows installer and zip, Linux tarball) now carries LICENSE.txt and README.txt.
- CI refuses a release tag that does not match the project version.
- Windows builds remain unsigned (SmartScreen "Run anyway"). Linux builds remain untested by the author.

## 0.1.1 — 2026-10-24

Fixes from the post-release code review. Use this instead of 0.1.0.

- **Crash fix:** at sample rates of 88.2 kHz and above the processor re-created its sample ring while the editor could still be reading it. The ring is now allocated once at a fixed size.
- Status light no longer freezes in hosts that deliver large audio blocks (2048–4096 samples): idle silence is only injected after ~100 ms without audio.
- Balance reads `L 60.0 dB` / `R 60.0 dB` instead of `C` when one channel is digital silence.
- The AUTO gain toggle in the scope's corner is now visible when off (dimmed), not only when on.
- The correlation readout colours now match the status-light thresholds (+0.1 / −0.2).
- Windows installer asks you to close a DAW that has OpenGonio loaded instead of failing to replace the file.
- Seven more unit tests (37 total).

## 0.1.0 — 2026-10-24

First release.

- Goniometer (Lissajous / vectorscope) with phosphor-style persistence and an optional auto-gain scale.
- Phase correlation readout and bar, stereo width, L/R balance.
- L and R sample-peak meters with 1.5 s peak hold and a 2 s clip latch.
- Status light: MONO OK, CAUTION, PHASE ISSUE, NO SIGNAL, L ONLY, R ONLY, with 500 ms hysteresis.
- Resizable window; size is saved with the project.
- Formats: VST3 and AU on macOS (universal, 11+), VST3 on Windows (x64, 10+), VST3 and LV2 on Linux (x64).
- Audio passes through untouched; zero latency.

Known limitations:

- Builds are **not code-signed**. macOS needs the Gatekeeper workaround and Windows shows a SmartScreen warning; see the README. Signing is planned for 0.2.0.
- Linux builds are produced by CI but untested by the author.
- Status-light thresholds (OK above +0.1, issue below −0.2) are a starting point; feedback welcome.
