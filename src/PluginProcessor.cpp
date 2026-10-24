#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr int kInitialFifoFrames = 96000;
constexpr const char* kEditorW = "editorW";
constexpr const char* kEditorH = "editorH";
} // namespace

OpenGonioProcessor::OpenGonioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "OpenGonio", createLayout()),
      sampleFifo(std::make_unique<opengonio::dsp::SampleFifo>(kInitialFifoFrames))
{
}

juce::AudioProcessorValueTreeState::ParameterLayout OpenGonioProcessor::createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"autoGain", 1},
                                                          "Auto Gain", false));
    return layout;
}

void OpenGonioProcessor::prepareToPlay(double newSampleRate, int)
{
    sampleRate.store(newSampleRate);
    setLatencySamples(0);

    // Two seconds of audio. Only re-create the ring when the capacity changes,
    // and never from processBlock.
    const int wanted = (int) (2.0 * newSampleRate);
    if (sampleFifo->capacity() < wanted)
        sampleFifo = std::make_unique<opengonio::dsp::SampleFifo>(wanted);
}

bool OpenGonioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    if (in != out)
        return false;
    return in == juce::AudioChannelSet::stereo() || in == juce::AudioChannelSet::mono();
}

void OpenGonioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numIn = getMainBusNumInputChannels();
    if (numIn < 1 || buffer.getNumChannels() < 1)
        return;

    const float* l = buffer.getReadPointer(0);
    const float* r = (numIn >= 2 && buffer.getNumChannels() >= 2) ? buffer.getReadPointer(1) : l;
    sampleFifo->push(l, r, buffer.getNumSamples());
    // Audio passes through untouched.
}

juce::AudioProcessorEditor* OpenGonioProcessor::createEditor()
{
    return new OpenGonioEditor(*this);
}

juce::Point<int> OpenGonioProcessor::editorSize() const
{
    const auto w = (int) apvts.state.getProperty(kEditorW, kDefaultWidth);
    const auto h = (int) apvts.state.getProperty(kEditorH, kDefaultHeight);
    return {w > 0 ? w : kDefaultWidth, h > 0 ? h : kDefaultHeight};
}

void OpenGonioProcessor::setEditorSize(juce::Point<int> size)
{
    apvts.state.setProperty(kEditorW, size.x, nullptr);
    apvts.state.setProperty(kEditorH, size.y, nullptr);
}

void OpenGonioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void OpenGonioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpenGonioProcessor();
}
