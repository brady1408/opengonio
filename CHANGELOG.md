# Changelog

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
