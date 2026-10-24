#include "PluginEditor.h"
#include "ui/Theme.h"

namespace
{
constexpr int kHeaderH = 22;
constexpr int kFooterH = 18;
constexpr int kCorrelationH = 40;
constexpr int kGap = 12;
constexpr int kMinColumnW = 180;
constexpr int kPad = 12;
constexpr int kMaxFramesPerTick = 32768; // ~0.68 s at 48 kHz; a 60 Hz tick needs ~800
} // namespace

using namespace opengonio;

OpenGonioEditor::OpenGonioEditor(OpenGonioProcessor& p) : AudioProcessorEditor(p), proc(p)
{
    drainL.resize(8192);
    drainR.resize(8192);

    for (auto* c : std::initializer_list<juce::Component*>{&scope, &statusPill, &meterL, &meterR,
                                                           &correlationPill, &widthPill,
                                                           &balancePill, &correlationBar,
                                                           &projectLink})
        addAndMakeVisible(c);

    projectLink.setFont(theme::mono(11.f), false, juce::Justification::centredRight);
    projectLink.setColour(juce::HyperlinkButton::textColourId, theme::textMuted);

    scope.onAutoGainClicked = [this] { toggleAutoGain(); };

    proc.fifo().clearFromConsumer();

    setResizable(true, true);
    setResizeLimits(400, 280, 2000, 1400);
    const auto size = proc.editorSize();
    setSize(size.x, size.y);

    startTimerHz(60);
}

OpenGonioEditor::~OpenGonioEditor()
{
    stopTimer();
}

void OpenGonioEditor::paint(juce::Graphics& g)
{
    g.fillAll(theme::windowBg);

    auto header = getLocalBounds().removeFromTop(kHeaderH).reduced(kPad, 0);
    g.setColour(theme::textMuted);
    g.setFont(theme::mono(11.f));
    g.drawText("GONIOMETER", header, juce::Justification::centredLeft);
    g.drawText("STEREO FIELD", header, juce::Justification::centredRight);

    auto footer = getLocalBounds().removeFromBottom(kFooterH).reduced(kPad, 0);
    g.drawText("OpenGonio " JucePlugin_VersionString, footer, juce::Justification::centredLeft);
}

void OpenGonioEditor::resized()
{
    // Only user/host resizes of a visible editor are remembered. Hosts can
    // resize the view while attaching or tearing it down; those are not the
    // user's choice and must not overwrite the stored size.
    if (isShowing() && getWidth() >= 400 && getHeight() >= 280)
        proc.setEditorSize({getWidth(), getHeight()});

    auto area = getLocalBounds();
    area.removeFromTop(kHeaderH);
    auto footer = area.removeFromBottom(kFooterH).reduced(kPad, 0);
    projectLink.setBounds(footer.removeFromRight(footer.getWidth() / 2));

    area = area.reduced(kPad, 0);
    auto correlationArea = area.removeFromBottom(kCorrelationH);
    correlationBar.setBounds(correlationArea.withTrimmedTop(4));
    area.removeFromBottom(4);

    auto body = area;
    const int scopeSide = juce::jmin(body.getHeight(), body.getWidth() - kMinColumnW - kGap);
    scope.setBounds(body.removeFromLeft(juce::jmax(100, scopeSide)));
    body.removeFromLeft(kGap);
    auto column = body;

    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    fb.items.add(juce::FlexItem(statusPill).withHeight(36.f).withWidth((float) column.getWidth()));
    fb.items.add(juce::FlexItem(meterL).withHeight(44.f).withWidth((float) column.getWidth()));
    fb.items.add(juce::FlexItem(meterR).withHeight(44.f).withWidth((float) column.getWidth()));
    fb.items.add(juce::FlexItem(correlationPill).withHeight(40.f).withWidth((float) column.getWidth()));
    fb.items.add(juce::FlexItem(widthPill).withHeight(40.f).withWidth((float) column.getWidth()));
    fb.items.add(juce::FlexItem(balancePill).withHeight(40.f).withWidth((float) column.getWidth()));
    fb.performLayout(column);
}

void OpenGonioEditor::timerCallback()
{
    const double rate = proc.currentSampleRate();
    if (! juce::exactlyEqual(rate, analyzerRate) && rate > 0.0)
    {
        analyzer.prepare(rate);
        analyzerRate = rate;
    }
    analyzer.setMonoSource(proc.isMonoInput());

    const bool autoGain = proc.apvts.getRawParameterValue("autoGain")->load() > 0.5f;
    scope.setAutoGain(autoGain);

    drainAudio();
    pushReadings();
    scope.repaint();
}

void OpenGonioEditor::drainAudio()
{
    scope.decay();

    // Bounded work per tick: a host running faster than realtime (offline
    // render, pluginval) must not keep this loop spinning. Anything beyond the
    // budget is dropped so the display stays live rather than falling behind.
    int budget = kMaxFramesPerTick;
    int drained = 0;
    while (budget > 0)
    {
        const int want = juce::jmin(budget, (int) drainL.size());
        const int got = proc.fifo().pop(drainL.data(), drainR.data(), want);
        if (got <= 0)
            break;
        analyzer.process(drainL.data(), drainR.data(), got);
        scope.pushFrames(drainL.data(), drainR.data(), got);
        budget -= got;
        drained += got;
    }
    if (proc.fifo().size() > kMaxFramesPerTick)
        proc.fifo().clearFromConsumer();

    // Nothing arrived this tick: the transport is stopped (or the host is not
    // calling us). Let the meters fall and the status light go quiet.
    if (drained == 0 && analyzerRate > 0.0)
        analyzer.processSilence((int) (analyzerRate / 60.0));
}

void OpenGonioEditor::pushReadings()
{
    const auto r = analyzer.readings();

    statusPill.setStatus(r.status, r.monoSource);
    meterL.setLevels(r.levelDbL, r.holdDbL, r.clipL);
    meterR.setLevels(r.levelDbR, r.holdDbR, r.clipR);

    correlationPill.setValue(juce::String::formatted("%+.2f", (double) r.correlation));
    correlationPill.setValueColour(r.correlation < -0.3f ? theme::red
                                   : std::abs(r.correlation) <= 0.3f ? theme::amber
                                                                     : theme::textPrimary);
    widthPill.setValue(juce::String::formatted("%.0f%%", (double) r.widthPercent));
    balancePill.setValue(dsp::formatBalance(r.balanceDb));
    correlationBar.setValue(r.correlation);
}

void OpenGonioEditor::toggleAutoGain()
{
    if (auto* param = proc.apvts.getParameter("autoGain"))
    {
        const bool on = param->getValue() > 0.5f;
        param->beginChangeGesture();
        param->setValueNotifyingHost(on ? 0.f : 1.f);
        param->endChangeGesture();
    }
}
