cmake_minimum_required(VERSION 3.22)
project(OpenGonio VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(OPENGONIO_BUILD_TESTS "Build the Catch2 unit tests" ON)
option(OPENGONIO_COPY_PLUGIN "Copy built plugins into the user plugin folders" ON)

# A/B tuning knobs (plan Task 11). Defaults match the shipped constants; an
# alternate build sets OPENGONIO_VARIANT=B to get a separately named plugin
# ("OpenGonio B", plugin code OgoB) that can sit beside the default in a host.
set(OPENGONIO_VARIANT "" CACHE STRING "Variant suffix for side-by-side A/B builds (empty or B)")
set(OPENGONIO_DECAY_ALPHA "0.24f" CACHE STRING "Scope persistence decay per frame")
set(OPENGONIO_HOT_POINTS "160" CACHE STRING "Trailing points drawn in the hot colour")
set(OPENGONIO_LINE_WIDTH "1.0f" CACHE STRING "Scope trace line width in px")
set(OPENGONIO_AUTOGAIN_RELEASE "1.0f" CACHE STRING "Auto-gain release in seconds")
set(OPENGONIO_OK_ABOVE "0.1f" CACHE STRING "Correlation above which the light reads MONO OK")
set(OPENGONIO_ISSUE_BELOW "-0.2f" CACHE STRING "Correlation below which the light reads PHASE ISSUE")

if(OPENGONIO_VARIANT STREQUAL "")
    set(OPENGONIO_PRODUCT_NAME "OpenGonio")
    set(OPENGONIO_PLUGIN_CODE Ogon)
    set(OPENGONIO_BUNDLE_ID com.bradycole.opengonio)
else()
    set(OPENGONIO_PRODUCT_NAME "OpenGonio ${OPENGONIO_VARIANT}")
    set(OPENGONIO_PLUGIN_CODE Ogo${OPENGONIO_VARIANT})
    set(OPENGONIO_BUNDLE_ID com.bradycole.opengonio.${OPENGONIO_VARIANT})
endif()

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        9.0.4
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

if(APPLE)
    set(OPENGONIO_FORMATS VST3 AU Standalone)
elseif(WIN32)
    set(OPENGONIO_FORMATS VST3 Standalone)
else()
    set(OPENGONIO_FORMATS VST3 LV2 Standalone)
endif()

juce_add_plugin(OpenGonio
    PRODUCT_NAME "${OPENGONIO_PRODUCT_NAME}"
    COMPANY_NAME "Brady Cole"
    BUNDLE_ID ${OPENGONIO_BUNDLE_ID}
    PLUGIN_MANUFACTURER_CODE Brdy
    PLUGIN_CODE ${OPENGONIO_PLUGIN_CODE}
    FORMATS ${OPENGONIO_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${OPENGONIO_COPY_PLUGIN}
    VST3_CATEGORIES Fx Analyzer
    AU_MAIN_TYPE kAudioUnitType_Effect
    LV2URI https://github.com/brady1408/opengonio)

target_sources(OpenGonio PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/ui/ScopeComponent.cpp
    src/ui/LevelMeter.cpp
    src/ui/StatusPill.cpp
    src/ui/ReadoutPill.cpp
    src/ui/CorrelationBar.cpp)

target_include_directories(OpenGonio PRIVATE src)

target_compile_definitions(OpenGonio PUBLIC
    OPENGONIO_DECAY_ALPHA=${OPENGONIO_DECAY_ALPHA}
    OPENGONIO_HOT_POINTS=${OPENGONIO_HOT_POINTS}
    OPENGONIO_LINE_WIDTH=${OPENGONIO_LINE_WIDTH}
    OPENGONIO_AUTOGAIN_RELEASE=${OPENGONIO_AUTOGAIN_RELEASE}
    OPENGONIO_OK_ABOVE=${OPENGONIO_OK_ABOVE}
    OPENGONIO_ISSUE_BELOW=${OPENGONIO_ISSUE_BELOW}
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(OpenGonio
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if(OPENGONIO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
