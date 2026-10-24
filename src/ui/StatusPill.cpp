#include "StatusPill.h"
#include "Theme.h"

namespace opengonio::ui
{

using opengonio::dsp::Status;

void StatusPill::setStatus(Status newStatus, bool isMonoSource)
{
    if (status != newStatus || monoSource != isMonoSource)
    {
        status = newStatus;
        monoSource = isMonoSource;
        repaint();
    }
}

void StatusPill::paint(juce::Graphics& g)
{
    juce::String text;
    juce::Colour bg, border, fg;

    if (monoSource)
    {
        text = "MONO SOURCE";
        bg = theme::panelBg;
        border = theme::panelBorder;
        fg = theme::textMuted;
    }
    else
    {
        switch (status)
        {
        case Status::Ok:
            text = "MONO OK";
            bg = theme::okBg;
            border = theme::okBorder;
            fg = theme::green;
            break;
        case Status::Caution:
            text = "CAUTION";
            bg = theme::panelBg;
            border = theme::amber;
            fg = theme::amber;
            break;
        case Status::Issue:
            text = "PHASE ISSUE";
            bg = theme::issueBg;
            border = theme::issueBorder;
            fg = theme::red;
            break;
        case Status::NoSignal:
        default:
            text = "NO SIGNAL";
            bg = theme::panelBg;
            border = theme::panelBorder;
            fg = theme::textMuted;
            break;
        }
    }

    const auto r = getLocalBounds().toFloat().reduced(0.5f);
    g.setColour(bg);
    g.fillRoundedRectangle(r, 8.f);
    g.setColour(border);
    g.drawRoundedRectangle(r, 8.f, 1.f);
    g.setColour(fg);
    g.setFont(theme::mono(13.f).boldened());
    g.drawText(text, getLocalBounds(), juce::Justification::centred);
}

} // namespace opengonio::ui
