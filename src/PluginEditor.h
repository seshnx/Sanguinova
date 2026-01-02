#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include "PluginProcessor.h"

/**
 * SanguinovaLookAndFeel - Blood Star Theme
 */
class SanguinovaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr int scopeSize = 256;

    SanguinovaLookAndFeel();

    void setDriveIntensity(float intensity) { driveIntensity = intensity; }
    void setMultiplierLevel(float level) { multiplierLevel = level; }
    void setScopeData(const std::array<float, scopeSize>& data) { scopeData = data; }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    // Color palette
    static const juce::Colour backgroundDark;
    static const juce::Colour backgroundMid;
    static const juce::Colour crimsonBase;
    static const juce::Colour crimsonBright;
    static const juce::Colour crimsonDark;
    static const juce::Colour textLight;
    static const juce::Colour textDim;

private:
    float driveIntensity = 0.0f;
    float multiplierLevel = 1.0f;
    std::array<float, scopeSize> scopeData{};
};

/**
 * IgnitionButton - Stage multiplier button
 */
class IgnitionButton : public juce::ToggleButton
{
public:
    IgnitionButton(const juce::String& text, const juce::String& multiplierText);
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown) override;

private:
    juce::String labelText;
    juce::String multText;
};

/**
 * OscilloscopeComponent - Hardware-accelerated oscilloscope display
 * Renders to a cached image and only updates when data changes
 */
class OscilloscopeComponent : public juce::Component
{
public:
    static constexpr int scopeSize = 256;

    OscilloscopeComponent();

    void setScopeData(const std::array<float, scopeSize>& data);
    void paint(juce::Graphics& g) override;

private:
    std::array<float, scopeSize> scopeData{};
    std::array<float, scopeSize> lastScopeData{};
    juce::Image cachedBackground;
    bool needsBackgroundRedraw = true;

    void drawBackground(juce::Graphics& g, float centreX, float centreY, float scopeRadius);
    void drawWaveform(juce::Graphics& g, float centreX, float centreY, float scopeRadius);
};

/**
 * SanguinovaAudioProcessorEditor - Main UI
 *
 * Layout:
 * [LEFT]           [CENTER]          [RIGHT]
 * Input Q          DRIVE (big)       Output LP
 * Color            Stage1 2 3        Output Gain
 * FilterMode       Multiplier        Mix
 */
class SanguinovaAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit SanguinovaAudioProcessorEditor(SanguinovaAudioProcessor&);
    ~SanguinovaAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SanguinovaAudioProcessor& audioProcessor;
    SanguinovaLookAndFeel lookAndFeel;

    // OpenGL hardware acceleration
    juce::OpenGLContext openGLContext;

    // Cached background for static elements
    juce::Image cachedBackground;
    bool needsBackgroundRedraw = true;
    int lastWidth = 0, lastHeight = 0;
    float lastCoreIntensity = -1.0f;

    // Logo
    juce::Image logoImage;

    // Title
    juce::Label titleLabel;

    // Preset Controls
    juce::ComboBox presetBox;
    juce::TextButton savePresetButton{"SAVE"};
    void refreshPresetList();
    void savePresetDialog();

    // LEFT - Input Section
    juce::Slider inputQKnob;
    juce::Slider colorKnob;
    juce::Label inputQLabel;
    juce::Label colorLabel;
    juce::ComboBox filterModeBox;
    juce::Label filterModeLabel;

    // CENTER - Drive Section
    juce::Slider driveKnob;
    juce::Label driveLabel;
    OscilloscopeComponent oscilloscope;
    IgnitionButton stage2xButton;
    IgnitionButton stage5xButton;
    IgnitionButton stage10xButton;
    juce::Label multiplierDisplay;

    // RIGHT - Output Section
    juce::Slider outputLpKnob;
    juce::Slider outputGainKnob;
    juce::Slider mixKnob;
    juce::Label outputLpLabel;
    juce::Label outputGainLabel;
    juce::Label mixLabel;
    juce::ToggleButton padButton;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> colorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputLpAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stage2xAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stage5xAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stage10xAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> padAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SanguinovaAudioProcessorEditor)
};
