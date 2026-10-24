#include "dsp/SampleFifo.h"

#include <catch2/catch_test_macros.hpp>

using opengonio::dsp::SampleFifo;

TEST_CASE("fifo round-trips frames in order")
{
    SampleFifo f(8);
    float l[3] = {1, 2, 3}, r[3] = {-1, -2, -3};
    REQUIRE(f.push(l, r, 3) == 3);
    REQUIRE(f.size() == 3);
    float ol[8], orr[8];
    REQUIRE(f.pop(ol, orr, 8) == 3);
    REQUIRE(ol[2] == 3.f);
    REQUIRE(orr[0] == -1.f);
    REQUIRE(f.size() == 0);
}

TEST_CASE("fifo drops the tail on overflow and never exceeds capacity")
{
    SampleFifo f(8); // capacity 8
    float l[16], r[16];
    for (int i = 0; i < 16; ++i)
        l[i] = r[i] = (float) i;
    int written = f.push(l, r, 16);
    REQUIRE(written <= f.capacity());
    REQUIRE(f.size() <= f.capacity());
    float ol[16], orr[16];
    int got = f.pop(ol, orr, 16);
    REQUIRE(got == written);
    REQUIRE(ol[0] == 0.f);
}

TEST_CASE("capacity rounds up to a power of two")
{
    REQUIRE(SampleFifo(100).capacity() == 128);
}

TEST_CASE("clearFromConsumer empties the ring")
{
    SampleFifo f(8);
    float x[4] = {1, 1, 1, 1};
    f.push(x, x, 4);
    f.clearFromConsumer();
    REQUIRE(f.size() == 0);
}

TEST_CASE("fifo wraps around the ring correctly")
{
    SampleFifo f(8);
    float in[6] = {1, 2, 3, 4, 5, 6}, out[8], outr[8];
    REQUIRE(f.push(in, in, 6) == 6);
    REQUIRE(f.pop(out, outr, 6) == 6);
    float in2[6] = {7, 8, 9, 10, 11, 12};
    REQUIRE(f.push(in2, in2, 6) == 6); // crosses the end of the ring
    REQUIRE(f.pop(out, outr, 6) == 6);
    REQUIRE(out[0] == 7.f);
    REQUIRE(out[5] == 12.f);
}

TEST_CASE("the processor's fixed FIFO capacity covers more than a second at 192 kHz")
{
    REQUIRE(opengonio::dsp::kProcessorFifoFrames >= 192000 * 1.3);
    REQUIRE(SampleFifo(opengonio::dsp::kProcessorFifoFrames).capacity() == opengonio::dsp::kProcessorFifoFrames);
}
