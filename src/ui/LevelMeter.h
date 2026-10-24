#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace opengonio::ui
{

// One horizontal peak meter: label + dB readout above a -60..0 dBFS bar with a
// peak-hold tick. The fill end turns red while clipped.
class LevelMeter : public juce::Component
{
public:
    explicit LevelMeter(juce::String label);

    void setLevels(float levelDb, float holdDb, bool clipped);
    void paint(juce::Graphics&) override;

    static constexpr float kMinDb = -60.f;
    static constexpr float kMaxDb = 0.f;

private:
    float xForDb(float db, juce::Rectangle<float> track) const;

    juce::String label;
    float levelDb = kMinDb, holdDb = kMinDb;
    bool clipped = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

} // namespace opengonio::ui
