#include "LevelMeter.h"
#include "Theme.h"

namespace opengonio::ui
{

LevelMeter::LevelMeter(juce::String labelText) : label(std::move(labelText)) {}

void LevelMeter::setLevels(float newLevelDb, float newHoldDb, bool isClipped)
{
    if (! juce::exactlyEqual(levelDb, newLevelDb) || ! juce::exactlyEqual(holdDb, newHoldDb)
        || clipped != isClipped)
    {
        levelDb = newLevelDb;
        holdDb = newHoldDb;
        clipped = isClipped;
        repaint();
    }
}

float LevelMeter::xForDb(float db, juce::Rectangle<float> track) const
{
    const float t = juce::jlimit(0.f, 1.f, (db - kMinDb) / (kMaxDb - kMinDb));
    return track.getX() + t * track.getWidth();
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    auto textRow = area.removeFromTop(14.f);
    area.removeFromTop(3.f);
    auto track = area;

    g.setFont(theme::mono(11.f));
    g.setColour(theme::textMuted);
    g.drawText(label, textRow, juce::Justification::centredLeft);
    const juce::String readout = levelDb <= kMinDb + 0.1f
                                     ? juce::String("-inf dB")
                                     : juce::String(levelDb, 1) + " dB";
    g.drawText(readout, textRow, juce::Justification::centredRight);

    g.setColour(theme::panelBg);
    g.fillRoundedRectangle(track, 4.f);

    if (levelDb > kMinDb)
    {
        auto fill = track.withRight(xForDb(levelDb, track));
        juce::ColourGradient grad(theme::green, track.getX(), 0.f, theme::red, track.getRight(), 0.f,
                                  false);
        grad.addColour(0.75, theme::green);
        grad.addColour(0.90, theme::amber);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(fill, 4.f);
        if (clipped)
        {
            g.setColour(theme::red);
            g.fillRoundedRectangle(fill.removeFromRight(6.f), 3.f);
        }
    }

    if (holdDb > kMinDb)
    {
        g.setColour(theme::white);
        const float x = xForDb(holdDb, track);
        g.fillRect(x - 1.f, track.getY(), 2.f, track.getHeight());
    }

    g.setColour(theme::panelBorder);
    g.drawRoundedRectangle(track, 4.f, 1.f);
}

} // namespace opengonio::ui
