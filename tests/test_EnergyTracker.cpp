#include "dsp/EnergyTracker.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <random>
#include <string>

using Catch::Approx;
using namespace opengonio::dsp;

namespace
{
constexpr double kPi = 3.14159265358979323846;
}

TEST_CASE("mono gives width 0 and balance C")
{
    EnergyTracker e;
    e.prepare(48000);
    for (int i = 0; i < 48000; ++i)
    {
        const float s = (float) std::sin(2 * kPi * 440 * i / 48000.0);
        e.push(s, s);
    }
    REQUIRE(widthPercent(e.rmsM(), e.rmsS()) == Approx(0.f).margin(0.5));
    REQUIRE(formatBalance(balanceDb(e.rmsL(), e.rmsR())) == "C");
}

TEST_CASE("pure side gives width 200")
{
    EnergyTracker e;
    e.prepare(48000);
    for (int i = 0; i < 48000; ++i)
    {
        const float s = (float) std::sin(2 * kPi * 440 * i / 48000.0);
        e.push(s, -s);
    }
    REQUIRE(widthPercent(e.rmsM(), e.rmsS()) == Approx(200.f).margin(1));
}

TEST_CASE("uncorrelated equal-level noise gives width about 100")
{
    EnergyTracker e;
    e.prepare(48000);
    std::mt19937 g(2);
    std::uniform_real_distribution<float> d(-1, 1);
    for (int i = 0; i < 96000; ++i)
        e.push(d(g), d(g));
    REQUIRE(widthPercent(e.rmsM(), e.rmsS()) == Approx(100.f).margin(10));
}

TEST_CASE("right channel 6 dB louder reads R 6.0 dB")
{
    EnergyTracker e;
    e.prepare(48000);
    for (int i = 0; i < 48000; ++i)
    {
        const float s = (float) std::sin(2 * kPi * 440 * i / 48000.0);
        e.push(0.5f * s, s);
    }
    REQUIRE(formatBalance(balanceDb(e.rmsL(), e.rmsR())) == "R 6.0 dB");
}

TEST_CASE("formatBalance edges")
{
    REQUIRE(formatBalance(0.09f) == "C");
    REQUIRE(formatBalance(0.11f) == "L 0.1 dB");
    REQUIRE(formatBalance(-1.24f) == "R 1.2 dB");
}
