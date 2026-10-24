#include "ScopeComponent.h"
#include "Theme.h"

#include <algorithm>
#include <cmath>

namespace opengonio::ui
{

ScopeComponent::ScopeComponent()
{
    setOpaque(true);
}

juce::Rectangle<int> ScopeComponent::scopeBounds() const
{
    const int side = std::min(getWidth(), getHeight());
    return juce::Rectangle<int>(side, side).withCentre(getLocalBounds().getCentre());
}

juce::Rectangle<int> ScopeComponent::autoTagBounds() const
{
    const auto b = scopeBounds();
    return juce::Rectangle<int>(b.getRight() - 44, b.getBottom() - 18, 44, 18).reduced(4, 2);
}

juce::Point<float> ScopeComponent::toPixel(float x, float y) const
{
    const auto b = persistence.getBounds().toFloat();
    const float half = b.getWidth() * 0.5f;
    return {half + x * half, half - y * half};
}

void ScopeComponent::resized()
{
    const auto b = scopeBounds();
    const int side = std::max(1, b.getWidth());
    if (persistence.getWidth() != side)
    {
        persistence = juce::Image(juce::Image::ARGB, side, side, true);
        hasLastPoint = false;
    }
}

void ScopeComponent::setAutoGain(bool enabled)
{
    if (autoGainEnabled != enabled)
    {
        autoGainEnabled = enabled;
        runningPeak = 0.f;
        autoGainScale = 1.f;
    }
}

void ScopeComponent::decay()
{
    if (!persistence.isValid())
        return;
    // Fade the trace toward transparent so the grid beneath stays visible.
    persistence.multiplyAllAlphas(1.f - kDecayAlpha);
}

void ScopeComponent::pushFrames(const float* l, const float* r, int n)
{
    if (!persistence.isValid() || n <= 0)
        return;

    juce::Graphics g(persistence);

    // Auto-gain: follow the running peak with a slow release, scale 1..20.
    if (autoGainEnabled)
    {
        const float release = std::pow(0.001f, 1.f / (kAutoGainReleaseSeconds * 60.f));
        float blockPeak = 0.f;
        for (int i = 0; i < n; ++i)
        {
            const float x = 0.5f * (r[i] - l[i]);
            const float y = 0.5f * (l[i] + r[i]);
            blockPeak = std::max({blockPeak, std::abs(x), std::abs(y)});
        }
        runningPeak = std::max(blockPeak, runningPeak * release);
        autoGainScale = runningPeak > 1e-4f ? juce::jlimit(1.f, 20.f, 1.f / runningPeak) : 1.f;
    }
    const float scale = autoGainEnabled ? autoGainScale : 1.f;

    const int hotStart = std::max(0, n - kHotPoints);
    for (int i = 0; i < n; ++i)
    {
        const float x = juce::jlimit(-1.f, 1.f, 0.5f * (r[i] - l[i]) * scale);
        const float y = juce::jlimit(-1.f, 1.f, 0.5f * (l[i] + r[i]) * scale);
        const auto p = toPixel(x, y);
        if (hasLastPoint)
        {
            g.setColour(i >= hotStart ? theme::greenHot : theme::green);
            g.drawLine({lastPoint, p}, 1.0f);
        }
        lastPoint = p;
        hasLastPoint = true;
    }
}

void ScopeComponent::paint(juce::Graphics& g)
{
    g.fillAll(theme::windowBg);
    auto b = scopeBounds();
    g.setColour(theme::scopeBg);
    g.fillRoundedRectangle(b.toFloat(), 6.f);

    const auto bf = b.toFloat();
    const auto c = bf.getCentre();
    g.setColour(theme::grid);
    g.drawLine(c.x, bf.getY(), c.x, bf.getBottom(), 1.f);
    g.drawLine(bf.getX(), c.y, bf.getRight(), c.y, 1.f);
    g.drawLine(bf.getX(), bf.getY(), bf.getRight(), bf.getBottom(), 1.f);
    g.drawLine(bf.getRight(), bf.getY(), bf.getX(), bf.getBottom(), 1.f);
    const float radius = bf.getWidth() * 0.5f * 0.7f;
    g.drawEllipse(c.x - radius, c.y - radius, 2 * radius, 2 * radius, 1.f);

    if (persistence.isValid())
        g.drawImageAt(persistence, b.getX(), b.getY());

    g.setColour(theme::textMuted);
    g.setFont(theme::mono(11.f));
    const int pad = 4;
    g.drawText("M", b.withHeight(14).translated(0, pad), juce::Justification::centred);
    g.drawText("S", b.withTop(b.getBottom() - 14).translated(0, -pad), juce::Justification::centred);
    g.drawText("L", b.withWidth(14).translated(pad, 0), juce::Justification::centred);
    g.drawText("R", b.withLeft(b.getRight() - 14).translated(-pad, 0), juce::Justification::centred);

    if (autoGainEnabled)
        g.drawText("AUTO", autoTagBounds(), juce::Justification::centredRight);
}

void ScopeComponent::mouseUp(const juce::MouseEvent& e)
{
    if (autoTagBounds().expanded(8).contains(e.getPosition()) && onAutoGainClicked)
        onAutoGainClicked();
}

} // namespace opengonio::ui
