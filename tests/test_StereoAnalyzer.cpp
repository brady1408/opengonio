#include "dsp/StereoAnalyzer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

using Catch::Approx;
using opengonio::dsp::Status;
using opengonio::dsp::StereoAnalyzer;

namespace
{
constexpr double kPi = 3.14159265358979323846;
}

TEST_CASE("inverted stereo reads Issue, correlation -1, width 200 within a second")
{
    StereoAnalyzer a;
    a.prepare(48000);
    std::vector<float> l(48000), r(48000);
    for (int i = 0; i < 48000; ++i)
    {
        l[i] = 0.5f * (float) std::sin(2 * kPi * 440 * i / 48000.0);
        r[i] = -l[i];
    }
    a.process(l.data(), r.data(), 48000);
    const auto R = a.readings();
    REQUIRE(R.status == Status::Issue);
    REQUIRE(R.correlation == Approx(-1.f).margin(0.01));
    REQUIRE(R.widthPercent == Approx(200.f).margin(1));
}

TEST_CASE("timings survive a sample-rate change: hold is 500 ms at 96 kHz too")
{
    StereoAnalyzer a;
    a.prepare(48000);
    a.prepare(96000);
    std::vector<float> l((size_t) (96000 * 0.4)), r(l.size());
    for (size_t i = 0; i < l.size(); ++i)
    {
        l[i] = (float) std::sin(2 * kPi * 440 * (double) i / 96000.0);
        r[i] = l[i];
    }
    a.process(l.data(), r.data(), (int) l.size());
    REQUIRE(a.readings().status == Status::NoSignal);
    a.process(l.data(), r.data(), (int) l.size());
    REQUIRE(a.readings().status == Status::Ok);
}

TEST_CASE("monoSource flag is echoed")
{
    StereoAnalyzer a;
    a.prepare(48000);
    a.setMonoSource(true);
    REQUIRE(a.readings().monoSource);
}

TEST_CASE("signal then one second of digital silence reads NoSignal")
{
    StereoAnalyzer a;
    a.prepare(48000);
    std::vector<float> l(48000), r(48000);
    for (int i = 0; i < 48000; ++i)
        l[i] = r[i] = 0.5f * (float) std::sin(2 * kPi * 440 * i / 48000.0);
    a.process(l.data(), r.data(), 48000);
    REQUIRE(a.readings().status == Status::Ok);
    std::vector<float> z(48000, 0.f);
    a.process(z.data(), z.data(), 48000);
    REQUIRE(a.readings().status == Status::NoSignal);
}

TEST_CASE("left-only signal through the analyzer reads LeftOnly")
{
    StereoAnalyzer a;
    a.prepare(48000);
    std::vector<float> l(48000), z(48000, 0.f);
    for (int i = 0; i < 48000; ++i)
        l[i] = 0.5f * (float) std::sin(2 * kPi * 440 * i / 48000.0);
    a.process(l.data(), z.data(), 48000);
    REQUIRE(a.readings().status == Status::LeftOnly);
    a.process(z.data(), l.data(), 48000);
    REQUIRE(a.readings().status == Status::RightOnly);
}

TEST_CASE("processSilence lets meters fall and status go NoSignal while readouts hold")
{
    StereoAnalyzer a;
    a.prepare(48000);
    std::vector<float> l(48000), r(48000);
    for (int i = 0; i < 48000; ++i)
    {
        l[i] = 0.5f * (float) std::sin(2 * kPi * 440 * i / 48000.0);
        r[i] = 0.8f * l[i];
    }
    a.process(l.data(), r.data(), 48000);
    const auto before = a.readings();
    REQUIRE(before.status == Status::Ok);

    a.processSilence(48000);
    const auto after = a.readings();
    REQUIRE(after.status == Status::NoSignal);
    REQUIRE(after.levelDbL < before.levelDbL - 15.f);
    REQUIRE(after.correlation == Approx(before.correlation).margin(0.001));
    REQUIRE(after.widthPercent == Approx(before.widthPercent).margin(0.01));
    REQUIRE(after.balanceDb == Approx(before.balanceDb).margin(0.01));
}
