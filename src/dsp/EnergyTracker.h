#pragma once

// EWMA RMS of L, R, M = (L+R)/2 and S = (L-R)/2, plus the stereo width and
// balance helpers derived from them. Plain C++ only.

#include <cmath>
#include <cstdio>
#include <string>

namespace opengonio::dsp
{

class EnergyTracker
{
public:
    void prepare(double sampleRate, double windowMs = 300.0)
    {
        const double samples = windowMs * 0.001 * sampleRate;
        coef = samples > 0.0 ? std::exp(-1.0 / samples) : 0.0;
        reset();
    }

    void push(float l, float r)
    {
        const double dl = std::isfinite(l) ? (double) l : 0.0;
        const double dr = std::isfinite(r) ? (double) r : 0.0;
        const double m = 0.5 * (dl + dr);
        const double s = 0.5 * (dl - dr);
        avg(sqL, dl * dl);
        avg(sqR, dr * dr);
        avg(sqM, m * m);
        avg(sqS, s * s);
    }

    float rmsL() const { return (float) std::sqrt(sqL); }
    float rmsR() const { return (float) std::sqrt(sqR); }
    float rmsM() const { return (float) std::sqrt(sqM); }
    float rmsS() const { return (float) std::sqrt(sqS); }

    void reset() { sqL = sqR = sqM = sqS = 0.0; }

private:
    void avg(double& acc, double x) { acc = coef * acc + (1.0 - coef) * x; }

    double coef = 0.0;
    double sqL = 0.0, sqR = 0.0, sqM = 0.0, sqS = 0.0;
};

// 0 % for mono (S = 0), 100 % for uncorrelated equal-level channels (M = S),
// 200 % for pure side (M = 0).
inline float widthPercent(float rmsM, float rmsS)
{
    const float sum = rmsM + rmsS;
    if (sum < 1e-9f)
        return 0.f;
    return 100.f * 2.f * rmsS / sum;
}

// Positive = left louder. Clamped to [-60, 60]; 0 when either side is silent.
inline float balanceDb(float rmsL, float rmsR)
{
    if (rmsL < 1e-9f || rmsR < 1e-9f)
        return 0.f;
    const float db = 20.f * std::log10(rmsL / rmsR);
    return db < -60.f ? -60.f : (db > 60.f ? 60.f : db);
}

// "C" within ±0.1 dB, else "L 0.4 dB" / "R 1.2 dB" with one decimal.
inline std::string formatBalance(float balanceDb)
{
    if (std::abs(balanceDb) <= 0.1f)
        return "C";
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%s %.1f dB", balanceDb > 0.f ? "L" : "R",
                  (double) std::abs(balanceDb));
    return buf;
}

} // namespace opengonio::dsp
