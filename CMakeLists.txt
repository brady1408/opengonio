cmake_minimum_required(VERSION 3.22)
project(OpenGonio VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(OPENGONIO_BUILD_TESTS "Build the Catch2 unit tests" ON)

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
    PRODUCT_NAME "OpenGonio"
    COMPANY_NAME "Brady Cole"
    BUNDLE_ID com.bradycole.opengonio
    PLUGIN_MANUFACTURER_CODE Brdy
    PLUGIN_CODE Ogon
    FORMATS ${OPENGONIO_FORMATS}
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES Fx Analyzer
    AU_MAIN_TYPE kAudioUnitType_Effect
    LV2URI https://github.com/brady1408/opengonio)

target_sources(OpenGonio PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp)

target_include_directories(OpenGonio PRIVATE src)

target_compile_definitions(OpenGonio PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

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
