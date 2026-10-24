#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace opengonio::ui
{

// -1 .. +1 gradient bar with a white needle and labels beneath.
class CorrelationBar : public juce::Component
{
public:
    CorrelationBar() = default;

    void setValue(float correlation);
    void paint(juce::Graphics&) override;

private:
    float value = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CorrelationBar)
};

} // namespace opengonio::ui
