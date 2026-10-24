#include "dsp/CorrelationMeter.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <random>

using Catch::Approx;
using opengonio::dsp::CorrelationMeter;

namespace
{
constexpr double kPi = 3.14159265358979323846;

void feedSine(CorrelationMeter& m, float gainR, int n = 48000)
{
    for (int i = 0; i < n; ++i)
    {
        const float s = (float) std::sin(2 * kPi * 440 * i / 48000.0);
        m.push(s, gainR * s);
    }
}
} // namespace

TEST_CASE("identical channels read +1")
{
    CorrelationMeter m;
    m.prepare(48000);
    feedSine(m, 1.f);
    REQUIRE(m.value() == Approx(1.f).margin(0.01));
}

TEST_CASE("inverted channels read -1")
{
    CorrelationMeter m;
    m.prepare(48000);
    feedSine(m, -1.f);
    REQUIRE(m.value() == Approx(-1.f).margin(0.01));
}

TEST_CASE("independent noise reads near 0")
{
    CorrelationMeter m;
    m.prepare(48000);
    std::mt19937 g(1);
    std::uniform_real_distribution<float> d(-1, 1);
    for (int i = 0; i < 96000; ++i)
        m.push(d(g), d(g));
    REQUIRE(std::abs(m.value()) < 0.1f);
}

TEST_CASE("silence reads 0 and NaN does not poison it")
{
    CorrelationMeter m;
    m.prepare(48000);
    REQUIRE(m.value() == 0.f);
    m.push(NAN, 0.f);
    feedSine(m, 1.f);
    REQUIRE(std::isfinite(m.value()));
    REQUIRE(m.value() == Approx(1.f).margin(0.01));
}

TEST_CASE("window is about 300 ms: value crosses 0 within 300 ms after a polarity flip")
{
    CorrelationMeter m;
    m.prepare(48000);
    feedSine(m, 1.f);
    int samples = 0;
    while (m.value() > 0.f && samples < 48000)
    {
        const float s = (float) std::sin(2 * kPi * 440 * samples / 48000.0);
        m.push(s, -s);
        ++samples;
    }
    REQUIRE(samples < 48000 * 0.3);
}
