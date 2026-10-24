#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace opengonio::ui
{

// Label on the left, value on the right, in a bordered pill.
class ReadoutPill : public juce::Component
{
public:
    explicit ReadoutPill(juce::String label);

    void setValue(juce::String value);
    void setValueColour(juce::Colour colour);
    void paint(juce::Graphics&) override;

private:
    juce::String label, value;
    juce::Colour valueColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReadoutPill)
};

} // namespace opengonio::ui
