#pragma once

// Facade over the meters: feed stereo samples, read back everything the UI
// shows. Runs on whichever thread calls process(); the plugin runs it on the
// GUI thread after draining the SampleFifo. Plain C++ only.

#include "CorrelationMeter.h"
#include "EnergyTracker.h"
#include "PeakMeter.h"
#include "StatusLight.h"

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
            light.push(correlation.value(), peakL.levelDb(), peakR.levelDb());
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
    }

private:
    CorrelationMeter correlation;
    EnergyTracker energy;
    PeakMeter peakL, peakR;
    StatusLight light;
    bool monoSource = false;
};

} // namespace opengonio::dsp
