#pragma once

#include "PluginProcessor.h"
#include "dsp/StereoAnalyzer.h"
#include "ui/CorrelationBar.h"
#include "ui/LevelMeter.h"
#include "ui/ReadoutPill.h"
#include "ui/ScopeComponent.h"
#include "ui/StatusPill.h"

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
    void drainAudio();
    void pushReadings();
    void toggleAutoGain();

    OpenGonioProcessor& proc;
    opengonio::dsp::StereoAnalyzer analyzer;
    double analyzerRate = 0.0;

    opengonio::ui::ScopeComponent scope;
    opengonio::ui::StatusPill statusPill;
    opengonio::ui::LevelMeter meterL{"L"}, meterR{"R"};
    opengonio::ui::ReadoutPill correlationPill{"Correlation"}, widthPill{"Width"},
        balancePill{"Balance"};
    opengonio::ui::CorrelationBar correlationBar;
    juce::HyperlinkButton projectLink{"github.com/brady1408/opengonio",
                                      juce::URL("https://github.com/brady1408/opengonio")};

    std::vector<float> drainL, drainR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGonioEditor)
};
