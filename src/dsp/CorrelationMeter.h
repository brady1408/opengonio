#pragma once

// Phase correlation between two channels: sum(L*R) / sqrt(sum(L^2) * sum(R^2)),
// with each sum an exponentially weighted moving average over `windowMs`.
// +1 identical, 0 uncorrelated, -1 inverted. Plain C++ only.

#include <cmath>

namespace opengonio::dsp
{

class CorrelationMeter
{
public:
    void prepare(double sampleRate, double windowMs = 300.0)
    {
        const double samples = windowMs * 0.001 * sampleRate;
        coef = samples > 0.0 ? std::exp(-1.0 / samples) : 0.0;
        reset();
    }

    void push(float l, float r)
    {
        const double dl = std::isfinite(l) ? (double) l : 0.0;
        const double dr = std::isfinite(r) ? (double) r : 0.0;
        sumLR = coef * sumLR + (1.0 - coef) * dl * dr;
        sumLL = coef * sumLL + (1.0 - coef) * dl * dl;
        sumRR = coef * sumRR + (1.0 - coef) * dr * dr;
    }

    float value() const
    {
        const double denom = std::sqrt(sumLL * sumRR);
        if (sumLL < kEps || sumRR < kEps || denom < kEps)
            return 0.f;
        const double v = sumLR / denom;
        return (float) (v < -1.0 ? -1.0 : (v > 1.0 ? 1.0 : v));
    }

    void reset() { sumLR = sumLL = sumRR = 0.0; }

private:
    static constexpr double kEps = 1e-12;
    double coef = 0.0;
    double sumLR = 0.0, sumLL = 0.0, sumRR = 0.0;
};

} // namespace opengonio::dsp
