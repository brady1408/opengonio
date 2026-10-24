#include "PluginEditor.h"
#include "ui/Theme.h"

OpenGonioEditor::OpenGonioEditor(OpenGonioProcessor& p) : AudioProcessorEditor(p), proc(p)
{
    drainL.resize(8192);
    drainR.resize(8192);
    addAndMakeVisible(scope);
    proc.fifo().clearFromConsumer();
    setSize(520, 360);
    startTimerHz(60);
}

OpenGonioEditor::~OpenGonioEditor()
{
    stopTimer();
}

void OpenGonioEditor::paint(juce::Graphics& g)
{
    g.fillAll(opengonio::theme::windowBg);
}

void OpenGonioEditor::resized()
{
    scope.setBounds(getLocalBounds());
}

void OpenGonioEditor::timerCallback()
{
    scope.decay();
    for (;;)
    {
        const int got = proc.fifo().pop(drainL.data(), drainR.data(), (int) drainL.size());
        if (got <= 0)
            break;
        scope.pushFrames(drainL.data(), drainR.data(), got);
        if (got < (int) drainL.size())
            break;
    }
    scope.repaint();
}
