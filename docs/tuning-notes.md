# OpenGonio tuning notes (A/B session, 2026-10-24, Brady at the Mac)

Material: a-ha, "Take On Me" (1985 FLAC), Audacity 4.0.1, three variant builds side by side.

## Round 1 — persistence decay (kDecayAlpha)
- A = 0.18 (then-current), B = 0.10 (slower fade), C = 0.30 (faster fade).
- Brady: B "too full", least favourite. A "keeps the lines a little longer, just fine". C "very clean". Torn between A and C.
- Decision: 0.24, the midpoint, carried into round 2.

## Round 2 — trace width and hot trail (material: Skrillex, "Scary Monsters and Nice Sprites")
- All at decay 0.24. A = 1.0 px / 64 hot points, B = 1.6 px / 64, C = 1.0 px / 160 hot points.
- Brady: B (thick) out. Split between A and C, leaning C.
- Decision: line width 1.0 px, hot trail 160 points.
- Observation: a wide commercial master sits at correlation ≈ +0.15 and reads CAUTION under the ±0.3 thresholds → round 3 tests the thresholds.

## Round 3 — status light thresholds (material: Skrillex)
- All at decay 0.24 / 160 hot points / 1.0 px. A = OK above +0.3, issue below −0.3 (original). B = OK above 0.0, issue below −0.3. C = OK above +0.1, issue below −0.2.
- On this master A flickered CAUTION/OK around +0.2–0.4; B and C read MONO OK throughout.
- Brady: can't judge without a sound engineer's input; pick a starting point and adjust on feedback.
- Decision (Claude's call): C. Positive correlation is the conventional "mono-safe" reading, a small caution band around zero still flags drift, and −0.2 sustained for 500 ms is a real problem. README asks for feedback on it.

## Not A/B'd
- Auto-gain release: 1.0 s (Claude's call; 0.5 s felt twitchy on transients during the session, 1.5 s lags a drop).

## Final 0.1.0 constants
- kDecayAlpha 0.24, kHotPoints 160, kLineWidth 1.0 px, kAutoGainReleaseSeconds 1.0, kOkAbove +0.1, kIssueBelow −0.2, hold 500 ms, silence −70 dBFS.
