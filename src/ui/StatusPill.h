#pragma once

#include "dsp/StatusLight.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace opengonio::ui
{

// MONO OK / CAUTION / PHASE ISSUE / NO SIGNAL, or MONO SOURCE when the input is mono.
class StatusPill : public juce::Component
{
public:
    void setStatus(opengonio::dsp::Status status, bool monoSource);
    void paint(juce::Graphics&) override;

private:
    opengonio::dsp::Status status = opengonio::dsp::Status::NoSignal;
    bool monoSource = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StatusPill)
};

} // namespace opengonio::ui
