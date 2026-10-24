#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>

namespace opengonio::ui
{

// Goniometer: L/R rotated 45 degrees onto a square persistence image.
// x = (R - L) / 2, y = (L + R) / 2; +/-1 spans the box at fixed scale.
class ScopeComponent : public juce::Component
{
public:
    ScopeComponent();

    // GUI thread. Appends points to the persistence image.
    void pushFrames(const float* l, const float* r, int n);
    // Call once per timer tick before pushFrames.
    void decay();
    void setAutoGain(bool enabled);
    bool autoGain() const { return autoGainEnabled; }

    // Fired when the AUTO tag is clicked.
    std::function<void()> onAutoGainClicked;

    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseUp(const juce::MouseEvent&) override;

    // Tuned in the A/B session (plan Task 11). Overridable from CMake for
    // side-by-side builds; the defaults here are the shipped values.
#ifndef OPENGONIO_DECAY_ALPHA
#define OPENGONIO_DECAY_ALPHA 0.24f
#endif
#ifndef OPENGONIO_HOT_POINTS
#define OPENGONIO_HOT_POINTS 160
#endif
#ifndef OPENGONIO_LINE_WIDTH
#define OPENGONIO_LINE_WIDTH 1.0f
#endif
#ifndef OPENGONIO_AUTOGAIN_RELEASE
#define OPENGONIO_AUTOGAIN_RELEASE 1.0f
#endif
    static constexpr float kDecayAlpha = OPENGONIO_DECAY_ALPHA;
    static constexpr int kHotPoints = OPENGONIO_HOT_POINTS;
    static constexpr float kLineWidth = OPENGONIO_LINE_WIDTH;
    static constexpr float kAutoGainReleaseSeconds = OPENGONIO_AUTOGAIN_RELEASE;

private:
    juce::Rectangle<int> scopeBounds() const;
    juce::Rectangle<int> autoTagBounds() const;
    juce::Point<float> toPixel(float x, float y) const;

    juce::Image persistence;
    bool hasLastPoint = false;
    juce::Point<float> lastPoint;
    bool autoGainEnabled = false;
    float runningPeak = 0.f;
    float autoGainScale = 1.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopeComponent)
};

} // namespace opengonio::ui
