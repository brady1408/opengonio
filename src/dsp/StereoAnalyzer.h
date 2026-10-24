#pragma once

// Facade over the meters: feed stereo samples, read back everything the UI
// shows. Runs on whichever thread calls process(); the plugin runs it on the
// GUI thread after draining the SampleFifo. Plain C++ only.

#include "CorrelationMeter.h"
#include "EnergyTracker.h"
#include "PeakMeter.h"
#include "StatusLight.h"

#include <algorithm>
#include <cmath>

namespace opengonio::dsp
{

struct Readings
{
    float correlation = 0.f;
    float widthPercent = 0.f;
    float balanceDb = 0.f;
    float levelDbL = -60.f, levelDbR = -60.f;
    float holdDbL = -60.f, holdDbR = -60.f;
    bool clipL = false, clipR = false;
    Status status = Status::NoSignal;
    bool monoSource = false;
};

class StereoAnalyzer
{
public:
    void prepare(double sampleRate)
    {
        correlation.prepare(sampleRate);
        energy.prepare(sampleRate);
        peakL.prepare(sampleRate);
        peakR.prepare(sampleRate);
        light.prepare(sampleRate);
        // Gate followers for the status light: fast enough that digital
        // silence is seen within ~100 ms, slow enough that zero crossings of a
        // low tone never read as silence. 700 dB/s.
        gateRelease = std::pow(10.0, -kGateReleaseDbPerSecond / 20.0 / std::max(1.0, sampleRate));
        gateL = gateR = 0.0;
    }

    void setMonoSource(bool mono) { monoSource = mono; }

    void process(const float* l, const float* r, int n)
    {
        for (int i = 0; i < n; ++i)
        {
            correlation.push(l[i], r[i]);
            energy.push(l[i], r[i]);
            peakL.push(l[i]);
            peakR.push(r[i]);
            gateL = std::max(absOrZero(l[i]), gateL * gateRelease);
            gateR = std::max(absOrZero(r[i]), gateR * gateRelease);
            light.push(correlation.value(), gateDb(gateL), gateDb(gateR));
        }
    }

    // Transport stopped / no audio arriving: peak meters and the status gate
    // see silence, while correlation, width and balance hold their last value.
    void processSilence(int n)
    {
        for (int i = 0; i < n; ++i)
        {
            peakL.push(0.f);
            peakR.push(0.f);
            gateL *= gateRelease;
            gateR *= gateRelease;
            light.push(correlation.value(), gateDb(gateL), gateDb(gateR));
        }
    }

    Readings readings() const
    {
        Readings out;
        out.correlation = correlation.value();
        out.widthPercent = widthPercent(energy.rmsM(), energy.rmsS());
        out.balanceDb = balanceDb(energy.rmsL(), energy.rmsR());
        out.levelDbL = peakL.levelDb();
        out.levelDbR = peakR.levelDb();
        out.holdDbL = peakL.holdDb();
        out.holdDbR = peakR.holdDb();
        out.clipL = peakL.clipped();
        out.clipR = peakR.clipped();
        out.status = light.status();
        out.monoSource = monoSource;
        return out;
    }

    void reset()
    {
        correlation.reset();
        energy.reset();
        peakL.reset();
        peakR.reset();
        light.reset();
        gateL = gateR = 0.0;
    }

private:
    static constexpr double kGateReleaseDbPerSecond = 700.0;

    static double absOrZero(float x) { return std::isfinite(x) ? std::abs((double) x) : 0.0; }
    // Unfloored dB for the silence gate; -200 stands in for -inf.
    static float gateDb(double lin) { return lin > 0.0 ? (float) (20.0 * std::log10(lin)) : -200.f; }

    double gateRelease = 1.0;
    double gateL = 0.0, gateR = 0.0;

    CorrelationMeter correlation;
    EnergyTracker energy;
    PeakMeter peakL, peakR;
    StatusLight light;
    bool monoSource = false;
};

} // namespace opengonio::dsp
