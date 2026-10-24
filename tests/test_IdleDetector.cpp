#include "dsp/IdleDetector.h"

#include <catch2/catch_test_macros.hpp>

using opengonio::dsp::IdleDetector;

TEST_CASE("a few empty ticks during playback inject no silence")
{
    IdleDetector d;
    REQUIRE(d.onTick(800, 800) == 0);
    REQUIRE(d.onTick(0, 800) == 0);
    REQUIRE(d.onTick(0, 800) == 0);
    REQUIRE(d.onTick(0, 800) == 0);
    REQUIRE(d.onTick(1600, 800) == 0);
}

TEST_CASE("after the grace period the whole gap is fed as silence, then one tick at a time")
{
    IdleDetector d;
    d.onTick(800, 800);
    int total = 0;
    for (int i = 0; i < IdleDetector::kGraceTicks - 1; ++i)
        total += d.onTick(0, 800);
    REQUIRE(total == 0);
    REQUIRE(d.onTick(0, 800) == IdleDetector::kGraceTicks * 800);
    REQUIRE(d.onTick(0, 800) == 800);
    REQUIRE(d.onTick(0, 800) == 800);
    REQUIRE(d.onTick(512, 800) == 0); // audio resumed
}
