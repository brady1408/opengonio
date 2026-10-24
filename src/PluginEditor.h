#pragma once

#include "PluginProcessor.h"
#include "ui/ScopeComponent.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <vector>

class OpenGonioEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit OpenGonioEditor(OpenGonioProcessor&);
    ~OpenGonioEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    OpenGonioProcessor& proc;
    opengonio::ui::ScopeComponent scope;
    std::vector<float> drainL, drainR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGonioEditor)
};
