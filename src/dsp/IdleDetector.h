#pragma once

// Decides when "no audio arrived this tick" means the transport is stopped
// rather than a large host block landing between two ticks. A host delivering
// 2048–4096-sample blocks leaves several consecutive empty 60 Hz ticks during
// normal playback; feeding silence on each of those would knock the status
// gate down and freeze the light. So: nothing for the first kGraceTicks empty
// ticks, then the whole accumulated gap at once, then one tick's worth per
// empty tick. Plain C++ only.

namespace opengonio::dsp
{

class IdleDetector
{
public:
    static constexpr int kGraceTicks = 6; // ~100 ms at 60 Hz

    // Returns how many frames of silence the caller should feed the analyzer.
    int onTick(int drainedFrames, int framesPerTick)
    {
        if (drainedFrames > 0)
        {
            emptyTicks = 0;
            return 0;
        }
        ++emptyTicks;
        if (emptyTicks < kGraceTicks)
            return 0;
        if (emptyTicks == kGraceTicks)
            return kGraceTicks * framesPerTick;
        return framesPerTick;
    }

    void reset() { emptyTicks = 0; }

private:
    int emptyTicks = 0;
};

} // namespace opengonio::dsp
