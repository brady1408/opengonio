#pragma once

// Palette from the design spec. Every colour the UI draws comes from here.

#include <juce_gui_basics/juce_gui_basics.h>

namespace opengonio::theme
{
inline const juce::Colour windowBg{0xff0d1117};
inline const juce::Colour scopeBg{0xff000000};
inline const juce::Colour grid{0xff21262d};
inline const juce::Colour panelBg{0xff161b22};
inline const juce::Colour panelBorder{0xff30363d};
inline const juce::Colour textMuted{0xff8b949e};
inline const juce::Colour textPrimary{0xffe6edf3};
inline const juce::Colour green{0xff3fb950};
inline const juce::Colour greenHot{0xff7ee787};
inline const juce::Colour amber{0xffd29922};
inline const juce::Colour red{0xfff85149};
inline const juce::Colour okBg{0xff12331f};
inline const juce::Colour okBorder{0xff238636};
inline const juce::Colour issueBg{0xff3d1d1d};
inline const juce::Colour issueBorder{0xffda3633};
inline const juce::Colour white{0xffffffff};

inline juce::Font mono(float height)
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), height,
                                        juce::Font::plain));
}
} // namespace opengonio::theme
