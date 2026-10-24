#include "ReadoutPill.h"
#include "Theme.h"

namespace opengonio::ui
{

ReadoutPill::ReadoutPill(juce::String labelText)
    : label(std::move(labelText)), valueColour(theme::textPrimary)
{
}

void ReadoutPill::setValue(juce::String newValue)
{
    if (value != newValue)
    {
        value = std::move(newValue);
        repaint();
    }
}

void ReadoutPill::setValueColour(juce::Colour colour)
{
    if (valueColour != colour)
    {
        valueColour = colour;
        repaint();
    }
}

void ReadoutPill::paint(juce::Graphics& g)
{
    const auto r = getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(theme::panelBg);
    g.fillRoundedRectangle(r, 8.f);
    g.setColour(theme::panelBorder);
    g.drawRoundedRectangle(r, 8.f, 1.f);

    const auto inner = getLocalBounds().reduced(10, 0);
    g.setColour(theme::textMuted);
    g.setFont(theme::mono(12.f));
    g.drawText(label, inner, juce::Justification::centredLeft);
    g.setColour(valueColour);
    g.setFont(theme::mono(16.f));
    g.drawText(value, inner, juce::Justification::centredRight);
}

} // namespace opengonio::ui
