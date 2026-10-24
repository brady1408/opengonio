#include "PluginEditor.h"

OpenGonioEditor::OpenGonioEditor(OpenGonioProcessor& p) : AudioProcessorEditor(p), proc(p)
{
    setSize(520, 360);
}

void OpenGonioEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0d1117));
    g.setColour(juce::Colour(0xff8b949e));
    g.setFont(juce::FontOptions(24.0f));
    g.drawText("OpenGonio", getLocalBounds(), juce::Justification::centred);
}
