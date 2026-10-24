#pragma once

#include "dsp/SampleFifo.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <memory>

class OpenGonioProcessor : public juce::AudioProcessor
{
public:
    OpenGonioProcessor();
    ~OpenGonioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // --- accessors used by the editor ---
    opengonio::dsp::SampleFifo& fifo() { return *sampleFifo; }
    double currentSampleRate() const { return sampleRate.load(); }
    bool isMonoInput() const { return getMainBusNumInputChannels() == 1; }
    juce::Point<int> editorSize() const;
    void setEditorSize(juce::Point<int> size);

    juce::AudioProcessorValueTreeState apvts;

    static constexpr int kDefaultWidth = 520;
    static constexpr int kDefaultHeight = 360;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::unique_ptr<opengonio::dsp::SampleFifo> sampleFifo;
    std::atomic<double> sampleRate{48000.0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGonioProcessor)
};
