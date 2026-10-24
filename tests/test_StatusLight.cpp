#include "dsp/StatusLight.h"

#include <catch2/catch_test_macros.hpp>

using opengonio::dsp::Status;
using opengonio::dsp::StatusLight;

TEST_CASE("starts NoSignal; silence stays NoSignal")
{
    StatusLight s;
    s.prepare(48000);
    for (int i = 0; i < 48000; ++i)
        s.push(0.f, -80.f, -80.f);
    REQUIRE(s.status() == Status::NoSignal);
}

TEST_CASE("Ok after 500 ms of correlation above the OK threshold, not before")
{
    StatusLight s;
    s.prepare(48000);
    for (int i = 0; i < 48000 * 0.4; ++i)
        s.push(0.9f, -6.f, -6.f);
    REQUIRE(s.status() == Status::NoSignal);
    for (int i = 0; i < 48000 * 0.2; ++i)
        s.push(0.9f, -6.f, -6.f);
    REQUIRE(s.status() == Status::Ok);
}

TEST_CASE("Issue below the issue threshold, Caution between")
{
    StatusLight s;
    s.prepare(48000);
    for (int i = 0; i < 48000; ++i)
        s.push(-0.8f, -6.f, -6.f);
    REQUIRE(s.status() == Status::Issue);
    for (int i = 0; i < 48000; ++i)
        s.push(0.f, -6.f, -6.f);
    REQUIRE(s.status() == Status::Caution);
}

TEST_CASE("a 100 ms blip does not flip the state")
{
    StatusLight s;
    s.prepare(48000);
    for (int i = 0; i < 48000; ++i)
        s.push(0.9f, -6.f, -6.f);
    for (int i = 0; i < 4800; ++i)
        s.push(-0.9f, -6.f, -6.f);
    REQUIRE(s.status() == Status::Ok);
}

TEST_CASE("one silent channel reads LeftOnly or RightOnly, not Caution")
{
    StatusLight s;
    s.prepare(48000);
    for (int i = 0; i < 48000; ++i)
        s.push(0.f, -6.f, -80.f);
    REQUIRE(s.status() == Status::LeftOnly);
    for (int i = 0; i < 48000; ++i)
        s.push(0.f, -80.f, -6.f);
    REQUIRE(s.status() == Status::RightOnly);
}
