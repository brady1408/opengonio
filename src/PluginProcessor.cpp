#include "PluginProcessor.h"
#include "PluginEditor.h"

OpenGonioProcessor::OpenGonioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void OpenGonioProcessor::prepareToPlay(double, int)
{
    setLatencySamples(0);
}

bool OpenGonioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    if (in != out)
        return false;
    return in == juce::AudioChannelSet::stereo() || in == juce::AudioChannelSet::mono();
}

void OpenGonioProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
    // Pass-through: the buffer is left untouched.
}

juce::AudioProcessorEditor* OpenGonioProcessor::createEditor()
{
    return new OpenGonioEditor(*this);
}

void OpenGonioProcessor::getStateInformation(juce::MemoryBlock&) {}

void OpenGonioProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpenGonioProcessor();
}
