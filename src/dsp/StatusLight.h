#pragma once

// MONO OK / CAUTION / PHASE ISSUE / NO SIGNAL / L ONLY / R ONLY state machine with hysteresis:
// the displayed status changes only after the candidate state has been stable
// for `holdMs` (500 ms by default). Plain C++ only.

#include <algorithm>

namespace opengonio::dsp
{

enum class Status
{
    NoSignal,
    Ok,
    Caution,
    Issue,
    LeftOnly,  // right channel silent, left has signal
    RightOnly  // left channel silent, right has signal
};

class StatusLight
{
public:
    void prepare(double sampleRate, double holdMs = 500.0)
    {
        holdSamples = (long) (holdMs * 0.001 * std::max(1.0, sampleRate));
        reset();
    }

    // Call once per sample with the current correlation and both peak levels.
    void push(float correlation, float peakDbL, float peakDbR)
    {
        const Status candidate = classify(correlation, peakDbL, peakDbR);
        if (candidate != pending)
        {
            pending = candidate;
            pendingCount = 0;
        }
        else if (pendingCount < holdSamples)
        {
            ++pendingCount;
        }
        if (pendingCount >= holdSamples)
            current = pending;
    }

    Status status() const { return current; }

    void reset()
    {
        current = pending = Status::NoSignal;
        pendingCount = 0;
    }

private:
    static constexpr float kSilenceDb = -70.f;
    static constexpr float kOkAbove = 0.3f;
    static constexpr float kIssueBelow = -0.3f;

    static Status classify(float c, float dbL, float dbR)
    {
        const bool silentL = dbL < kSilenceDb, silentR = dbR < kSilenceDb;
        if (silentL && silentR)
            return Status::NoSignal;
        if (silentR)
            return Status::LeftOnly;
        if (silentL)
            return Status::RightOnly;
        if (c > kOkAbove)
            return Status::Ok;
        if (c < kIssueBelow)
            return Status::Issue;
        return Status::Caution;
    }

    long holdSamples = 0;
    long pendingCount = 0;
    Status current = Status::NoSignal;
    Status pending = Status::NoSignal;
};

} // namespace opengonio::dsp
