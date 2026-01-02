cmake_minimum_required(VERSION 3.15)

project(Sanguinova VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find or fetch JUCE
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

# Plugin sources
set(PLUGIN_SOURCES
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

set(PLUGIN_HEADERS
    src/PluginProcessor.h
    src/PluginEditor.h
    src/PresetManager.h
    src/dsp/SanguinovaEngine.h
    src/dsp/SVFFilter.h
    src/dsp/AutoGain.h
    src/dsp/OnePole.h
    src/dsp/Oversampler.h
)

# Define the plugin
juce_add_plugin(Sanguinova
    COMPANY_NAME "SeshNx"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Sesh
    PLUGIN_CODE Sang
    FORMATS VST3 AU Standalone
    PRODUCT_NAME "Sanguinova"
    DESCRIPTION "Blood Star Distortion"
)

# Add binary resources (logo)
juce_add_binary_data(SanguinovaData
    SOURCES
        Resources/company_logo.png
)

# Set PIC flag for binary data (required for shared libraries on Linux)
set_target_properties(SanguinovaData PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
)

# Add sources
target_sources(Sanguinova
    PRIVATE
        ${PLUGIN_SOURCES}
        ${PLUGIN_HEADERS}
)

# Preprocessor definitions
target_compile_definitions(Sanguinova
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

# Link JUCE modules and binary data
target_link_libraries(Sanguinova
    PRIVATE
        SanguinovaData
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_opengl
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Include directories
target_include_directories(Sanguinova
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Print build info
message(STATUS "Sanguinova Version: ${PROJECT_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
