#pragma once

// Sample-peak meter: instant attack, 20 dB/s release, 1.5 s peak hold, and a
// 2 s clip latch above -0.1 dBFS. Floors at -60 dBFS. Plain C++ only.

#include <algorithm>
#include <cmath>

namespace opengonio::dsp
{

class PeakMeter
{
public:
    void prepare(double sampleRate)
    {
        const double sr = std::max(1.0, sampleRate);
        releaseCoef = std::pow(10.0, -kReleaseDbPerSecond / 20.0 / sr);
        holdSamples = (long) (kHoldSeconds * sr);
        clipSamples = (long) (kClipSeconds * sr);
        reset();
    }

    void push(float x)
    {
        const double a = std::isfinite(x) ? std::abs((double) x) : 0.0;

        peak = std::max(a, peak * releaseCoef);

        if (a >= holdPeak)
        {
            holdPeak = a;
            holdCounter = 0;
        }
        else if (++holdCounter > holdSamples)
        {
            holdPeak = peak;
        }

        if (a > kClipThreshold)
            clipCounter = clipSamples;
        else if (clipCounter > 0)
            --clipCounter;
    }

    float levelDb() const { return toDb(peak); }
    float holdDb() const { return toDb(holdPeak); }
    bool clipped() const { return clipCounter > 0; }

    void reset()
    {
        peak = holdPeak = 0.0;
        holdCounter = 0;
        clipCounter = 0;
    }

private:
    static constexpr double kReleaseDbPerSecond = 20.0;
    static constexpr double kHoldSeconds = 1.5;
    static constexpr double kClipSeconds = 2.0;
    static constexpr double kClipThreshold = 0.98855; // -0.1 dBFS
    static constexpr float kFloorDb = -60.f;

    static float toDb(double lin)
    {
        if (lin <= 0.0)
            return kFloorDb;
        return std::max(kFloorDb, (float) (20.0 * std::log10(lin)));
    }

    double releaseCoef = 1.0;
    long holdSamples = 0, clipSamples = 0;
    double peak = 0.0, holdPeak = 0.0;
    long holdCounter = 0, clipCounter = 0;
};

} // namespace opengonio::dsp
