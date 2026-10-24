#include "dsp/PeakMeter.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

using Catch::Approx;
using opengonio::dsp::PeakMeter;

namespace
{
constexpr double kPi = 3.14159265358979323846;
}

TEST_CASE("full-scale square reads 0 dBFS and clips; -0.2 dBFS sine does not clip")
{
    PeakMeter m;
    m.prepare(48000);
    for (int i = 0; i < 4800; ++i)
        m.push(i % 2 ? 1.f : -1.f);
    REQUIRE(m.levelDb() == Approx(0.f).margin(0.01));
    REQUIRE(m.clipped());

    PeakMeter n;
    n.prepare(48000);
    const float g = std::pow(10.f, -0.2f / 20.f);
    for (int i = 0; i < 48000; ++i)
        n.push(g * (float) std::sin(2 * kPi * 1000 * i / 48000.0));
    REQUIRE_FALSE(n.clipped());
    REQUIRE(n.levelDb() == Approx(-0.2f).margin(0.05));
}

TEST_CASE("release is 20 dB per second")
{
    PeakMeter m;
    m.prepare(48000);
    m.push(1.f);
    for (int i = 0; i < 48000; ++i)
        m.push(0.f);
    REQUIRE(m.levelDb() == Approx(-20.f).margin(0.5));
}

TEST_CASE("hold keeps the peak for 1.5 s then drops")
{
    PeakMeter m;
    m.prepare(48000);
    m.push(1.f);
    for (int i = 0; i < 48000; ++i)
        m.push(0.f);
    REQUIRE(m.holdDb() == Approx(0.f).margin(0.01));
    for (int i = 0; i < 48000; ++i)
        m.push(0.f);
    REQUIRE(m.holdDb() < -0.5f);
}

TEST_CASE("clip latch clears after 2 s")
{
    PeakMeter m;
    m.prepare(48000);
    m.push(1.f);
    for (int i = 0; i < 48000 * 2.1; ++i)
        m.push(0.f);
    REQUIRE_FALSE(m.clipped());
}

TEST_CASE("silence floors at -60")
{
    PeakMeter m;
    m.prepare(48000);
    REQUIRE(m.levelDb() == -60.f);
    REQUIRE(m.holdDb() == -60.f);
}
