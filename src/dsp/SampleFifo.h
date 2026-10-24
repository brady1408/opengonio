#pragma once

// Lock-free single-producer / single-consumer ring of stereo frames.
// Producer: push() on the audio thread. Consumer: pop() and clearFromConsumer()
// on the GUI thread. No allocation after construction; no locks.
// Plain C++ only: this header must not include anything from JUCE.

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <vector>

namespace opengonio::dsp
{

class SampleFifo
{
public:
    explicit SampleFifo(int capacityFrames)
        : cap(roundUpPow2(std::max(2, capacityFrames))), mask(cap - 1), left((size_t) cap),
          right((size_t) cap)
    {
    }

    int capacity() const { return cap; }

    int size() const
    {
        const auto w = writePos.load(std::memory_order_acquire);
        const auto r = readPos.load(std::memory_order_acquire);
        return (int) (w - r);
    }

    // Returns the number of frames written. Frames that do not fit are dropped
    // (the tail of the block), so the producer never blocks or overruns.
    int push(const float* l, const float* r, int n)
    {
        const auto w = writePos.load(std::memory_order_relaxed);
        const auto rd = readPos.load(std::memory_order_acquire);
        const int free = cap - (int) (w - rd);
        const int toWrite = std::max(0, std::min(n, free));
        for (int i = 0; i < toWrite; ++i)
        {
            const auto idx = (size_t) ((w + (uint64_t) i) & (uint64_t) mask);
            left[idx] = l[i];
            right[idx] = r[i];
        }
        writePos.store(w + (uint64_t) toWrite, std::memory_order_release);
        return toWrite;
    }

    // Returns the number of frames read (oldest first).
    int pop(float* l, float* r, int maxFrames)
    {
        const auto rd = readPos.load(std::memory_order_relaxed);
        const auto w = writePos.load(std::memory_order_acquire);
        const int avail = (int) (w - rd);
        const int toRead = std::max(0, std::min(maxFrames, avail));
        for (int i = 0; i < toRead; ++i)
        {
            const auto idx = (size_t) ((rd + (uint64_t) i) & (uint64_t) mask);
            l[i] = left[idx];
            r[i] = right[idx];
        }
        readPos.store(rd + (uint64_t) toRead, std::memory_order_release);
        return toRead;
    }

    // Consumer-side discard of everything currently buffered.
    void clearFromConsumer()
    {
        readPos.store(writePos.load(std::memory_order_acquire), std::memory_order_release);
    }

private:
    static int roundUpPow2(int v)
    {
        int p = 1;
        while (p < v)
            p <<= 1;
        return p;
    }

    const int cap;
    const int mask;
    std::vector<float> left, right;
    std::atomic<uint64_t> writePos{0};
    std::atomic<uint64_t> readPos{0};
};

} // namespace opengonio::dsp
