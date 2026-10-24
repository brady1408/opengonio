#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

class OpenGonioEditor : public juce::AudioProcessorEditor
{
public:
    explicit OpenGonioEditor(OpenGonioProcessor&);
    ~OpenGonioEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override {}

private:
    OpenGonioProcessor& proc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGonioEditor)
};
