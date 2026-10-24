#include "CorrelationBar.h"
#include "Theme.h"

namespace opengonio::ui
{

void CorrelationBar::setValue(float correlation)
{
    const float v = juce::jlimit(-1.f, 1.f, correlation);
    if (! juce::exactlyEqual(value, v))
    {
        value = v;
        repaint();
    }
}

void CorrelationBar::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    auto labels = area.removeFromBottom(14.f);
    area.removeFromBottom(4.f);
    auto bar = area.withSizeKeepingCentre(area.getWidth(), 12.f);

    juce::ColourGradient grad(theme::red, bar.getX(), 0.f, theme::green, bar.getRight(), 0.f, false);
    grad.addColour(0.35, theme::amber);
    grad.addColour(0.70, theme::green);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bar, 6.f);

    const float x = bar.getX() + (value + 1.f) * 0.5f * bar.getWidth();
    g.setColour(theme::white);
    g.fillRoundedRectangle(x - 1.5f, bar.getY() - 4.f, 3.f, bar.getHeight() + 8.f, 1.5f);

    g.setColour(theme::textMuted);
    g.setFont(theme::mono(11.f));
    g.drawText("-1", labels, juce::Justification::centredLeft);
    g.drawText("phase correlation", labels, juce::Justification::centred);
    g.drawText("+1", labels, juce::Justification::centredRight);
}

} // namespace opengonio::ui
