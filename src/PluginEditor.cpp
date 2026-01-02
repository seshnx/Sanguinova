#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

//==============================================================================
// Color Palette - Blood Star Theme (Plasma Red on Obsidian)
//==============================================================================
const juce::Colour SanguinovaLookAndFeel::backgroundDark = juce::Colour(0xFF050505);   // Obsidian
const juce::Colour SanguinovaLookAndFeel::backgroundMid = juce::Colour(0xFF0D0D0D);
const juce::Colour SanguinovaLookAndFeel::crimsonBase = juce::Colour(0xFFE01030);      // Plasma Red
const juce::Colour SanguinovaLookAndFeel::crimsonBright = juce::Colour(0xFFFF2040);    // Plasma Bright
const juce::Colour SanguinovaLookAndFeel::crimsonDark = juce::Colour(0xFF800820);      // Deep Blood
const juce::Colour SanguinovaLookAndFeel::textLight = juce::Colour(0xFFE0E0E0);
const juce::Colour SanguinovaLookAndFeel::textDim = juce::Colour(0xFF555555);

//==============================================================================
// SanguinovaLookAndFeel
//==============================================================================
SanguinovaLookAndFeel::SanguinovaLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, crimsonBase);
    setColour(juce::Slider::rotarySliderOutlineColourId, backgroundMid);
    setColour(juce::Slider::thumbColourId, crimsonBright);
    setColour(juce::Slider::textBoxTextColourId, textLight);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF0D0D0D));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF222222));
    setColour(juce::Label::textColourId, textLight);
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xFF0D0D0D));
    setColour(juce::ComboBox::textColourId, textLight);
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xFF333333));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xFF111111));
    setColour(juce::PopupMenu::textColourId, textLight);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, crimsonDark);
    setColour(juce::ToggleButton::textColourId, textLight);
    setColour(juce::ToggleButton::tickColourId, crimsonBright);
}

void SanguinovaLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider& slider)
{
    juce::ignoreUnused(slider);

    auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                          static_cast<float>(width), static_cast<float>(height));
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f - 4.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Calculate glow intensity
    float glowIntensity = driveIntensity * (1.0f + std::log2(std::max(1.0f, multiplierLevel)) * 0.25f);
    glowIntensity = juce::jlimit(0.0f, 1.0f, glowIntensity);

    // Outer glow (subtle)
    if (glowIntensity > 0.1f)
    {
        for (int i = 5; i >= 1; --i)
        {
            float glowRadius = radius + static_cast<float>(i) * 4.0f;
            float alpha = glowIntensity * (0.08f / static_cast<float>(i));
            g.setColour(crimsonBright.withAlpha(alpha));
            g.fillEllipse(centreX - glowRadius, centreY - glowRadius,
                         glowRadius * 2.0f, glowRadius * 2.0f);
        }
    }

    // Outer ring (dark)
    g.setColour(juce::Colour(0xFF1A1A1A));
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Track background
    float trackRadius = radius - 4.0f;
    juce::Path trackBg;
    trackBg.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xFF222222));
    g.strokePath(trackBg, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

    // Value arc with gradient
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, trackRadius, trackRadius, 0.0f,
                           rotaryStartAngle, angle, true);

    juce::Colour arcStart = crimsonDark;
    juce::Colour arcEnd = crimsonBright.interpolatedWith(juce::Colours::white, glowIntensity * 0.2f);

    juce::ColourGradient arcGradient(arcStart, centreX, centreY + trackRadius,
                                      arcEnd, centreX, centreY - trackRadius, false);
    g.setGradientFill(arcGradient);
    g.strokePath(valueArc, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // Inner knob body
    float knobRadius = radius * 0.65f;
    bool isLargeKnob = (radius > 100.0f);  // Detect center knob

    // Knob shadow
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.fillEllipse(centreX - knobRadius + 2, centreY - knobRadius + 2,
                  knobRadius * 2.0f, knobRadius * 2.0f);

    // Knob gradient
    juce::ColourGradient knobGradient(juce::Colour(0xFF252525), centreX, centreY - knobRadius,
                                       juce::Colour(0xFF0D0D0D), centreX, centreY + knobRadius, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(centreX - knobRadius, centreY - knobRadius,
                  knobRadius * 2.0f, knobRadius * 2.0f);

    // Note: Oscilloscope is now rendered as a separate hardware-accelerated component
    // for performance. The large knob center area is left empty for the overlay.

    // Knob edge highlight
    g.setColour(juce::Colour(0xFF333333));
    g.drawEllipse(centreX - knobRadius, centreY - knobRadius,
                  knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

    // Pointer/indicator (only for small knobs - large knob uses outer arc only)
    if (!isLargeKnob)
    {
        juce::Path pointer;
        float pointerLength = knobRadius * 0.75f;
        float pointerWidth = 4.0f;
        pointer.addRoundedRectangle(-pointerWidth / 2.0f, -knobRadius + 6.0f,
                                     pointerWidth, pointerLength, 2.0f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

        g.setColour(crimsonBright.interpolatedWith(juce::Colours::white, glowIntensity * 0.4f));
        g.fillPath(pointer);

        // Center dot
        float dotRadius = 3.0f;
        g.setColour(juce::Colour(0xFF444444));
        g.fillEllipse(centreX - dotRadius, centreY - dotRadius, dotRadius * 2.0f, dotRadius * 2.0f);
    }
}

void SanguinovaLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsDown);

    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    bool isOn = button.getToggleState();

    // Shadow
    g.setColour(juce::Colours::black.withAlpha(0.3f));
    g.fillRoundedRectangle(bounds.translated(1.0f, 1.0f), 5.0f);

    // Background
    juce::ColourGradient bg(isOn ? crimsonBase : juce::Colour(0xFF1A1A1A), bounds.getX(), bounds.getY(),
                            isOn ? crimsonDark : juce::Colour(0xFF0D0D0D), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 5.0f);

    // Border
    g.setColour(isOn ? crimsonBright.withAlpha(0.8f) : juce::Colour(0xFF333333));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

    if (shouldDrawButtonAsHighlighted && !isOn)
    {
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.fillRoundedRectangle(bounds, 5.0f);
    }

    // Text
    g.setColour(isOn ? textLight : textDim);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred);
}

void SanguinovaLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                          int buttonX, int buttonY, int buttonW, int buttonH,
                                          juce::ComboBox& box)
{
    juce::ignoreUnused(buttonX, buttonY, buttonW, buttonH, isButtonDown);

    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    // Background
    juce::ColourGradient bg(juce::Colour(0xFF1A1A1A), 0.0f, 0.0f,
                            juce::Colour(0xFF0D0D0D), 0.0f, static_cast<float>(height), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border
    g.setColour(box.isPopupActive() ? crimsonBright : juce::Colour(0xFF333333));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.0f);

    // Arrow
    auto arrowZone = juce::Rectangle<float>(static_cast<float>(width) - 22.0f, 0.0f, 18.0f, static_cast<float>(height));
    juce::Path arrow;
    float arrowSize = 5.0f;
    arrow.addTriangle(arrowZone.getCentreX() - arrowSize, arrowZone.getCentreY() - 2.0f,
                      arrowZone.getCentreX() + arrowSize, arrowZone.getCentreY() - 2.0f,
                      arrowZone.getCentreX(), arrowZone.getCentreY() + 4.0f);
    g.setColour(textDim);
    g.fillPath(arrow);
}

//==============================================================================
// IgnitionButton
//==============================================================================
IgnitionButton::IgnitionButton(const juce::String& text, const juce::String& multiplierText)
    : labelText(text), multText(multiplierText)
{
}

void IgnitionButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsDown);

    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    bool stageActive = getToggleState();

    // Glow when active
    if (stageActive)
    {
        for (int i = 4; i >= 1; --i)
        {
            float expand = static_cast<float>(i) * 3.0f;
            g.setColour(SanguinovaLookAndFeel::crimsonBright.withAlpha(0.06f / static_cast<float>(i)));
            g.fillRoundedRectangle(bounds.expanded(expand), 8.0f);
        }
    }

    // Shadow
    g.setColour(juce::Colours::black.withAlpha(0.4f));
    g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 6.0f);

    // Background
    juce::ColourGradient bg(stageActive ? SanguinovaLookAndFeel::crimsonBase : juce::Colour(0xFF1E1E1E),
                            bounds.getX(), bounds.getY(),
                            stageActive ? SanguinovaLookAndFeel::crimsonDark : juce::Colour(0xFF0A0A0A),
                            bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bg);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Inner highlight
    if (stageActive)
    {
        g.setColour(SanguinovaLookAndFeel::crimsonBright.withAlpha(0.15f));
        g.fillRoundedRectangle(bounds.removeFromTop(bounds.getHeight() * 0.4f), 6.0f);
    }

    bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Border
    g.setColour(stageActive ? SanguinovaLookAndFeel::crimsonBright : juce::Colour(0xFF3A3A3A));
    g.drawRoundedRectangle(bounds, 6.0f, stageActive ? 1.5f : 1.0f);

    if (shouldDrawButtonAsHighlighted && !stageActive)
    {
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.fillRoundedRectangle(bounds, 6.0f);
    }

    // Stage label (top)
    g.setColour(stageActive ? SanguinovaLookAndFeel::textLight.withAlpha(0.7f) : SanguinovaLookAndFeel::textDim);
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.drawText(labelText, bounds.removeFromTop(bounds.getHeight() * 0.38f),
               juce::Justification::centredBottom);

    // Multiplier (center)
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.setColour(stageActive ? juce::Colours::white : SanguinovaLookAndFeel::textDim);
    g.drawText(multText, bounds, juce::Justification::centred);
}

//==============================================================================
// OscilloscopeComponent - Hardware-accelerated oscilloscope
//==============================================================================
OscilloscopeComponent::OscilloscopeComponent()
{
    setOpaque(true);
    setBufferedToImage(true);  // Enable double buffering for smoother rendering
}

void OscilloscopeComponent::setScopeData(const std::array<float, scopeSize>& data)
{
    // Only repaint if data has significantly changed
    bool hasChanged = false;
    for (int i = 0; i < scopeSize; i += 4)  // Sample every 4th point for quick comparison
    {
        if (std::abs(data[i] - lastScopeData[i]) > 0.01f)
        {
            hasChanged = true;
            break;
        }
    }

    if (hasChanged)
    {
        scopeData = data;
        lastScopeData = data;
        repaint();
    }
}

void OscilloscopeComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float centreX = bounds.getCentreX();
    float centreY = bounds.getCentreY();
    float scopeRadius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f - 2.0f;

    // Draw cached background or update if needed
    if (needsBackgroundRedraw || cachedBackground.isNull() ||
        cachedBackground.getWidth() != getWidth() || cachedBackground.getHeight() != getHeight())
    {
        cachedBackground = juce::Image(juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics bgG(cachedBackground);
        drawBackground(bgG, centreX, centreY, scopeRadius);
        needsBackgroundRedraw = false;
    }

    g.drawImageAt(cachedBackground, 0, 0);
    drawWaveform(g, centreX, centreY, scopeRadius);
}

void OscilloscopeComponent::drawBackground(juce::Graphics& g, float centreX, float centreY, float scopeRadius)
{
    // === OUTER BEZEL (metal rim) ===
    float bezelWidth = 6.0f;

    // Bezel shadow
    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillEllipse(centreX - scopeRadius - bezelWidth + 2, centreY - scopeRadius - bezelWidth + 2,
                  (scopeRadius + bezelWidth) * 2.0f, (scopeRadius + bezelWidth) * 2.0f);

    // Bezel gradient (brushed metal look)
    juce::ColourGradient bezelGrad(juce::Colour(0xFF3A3A3A), centreX, centreY - scopeRadius - bezelWidth,
                                    juce::Colour(0xFF1A1A1A), centreX, centreY + scopeRadius + bezelWidth, false);
    g.setGradientFill(bezelGrad);
    g.fillEllipse(centreX - scopeRadius - bezelWidth, centreY - scopeRadius - bezelWidth,
                  (scopeRadius + bezelWidth) * 2.0f, (scopeRadius + bezelWidth) * 2.0f);

    // Inner bezel edge
    g.setColour(juce::Colour(0xFF4A4A4A));
    g.drawEllipse(centreX - scopeRadius - bezelWidth, centreY - scopeRadius - bezelWidth,
                  (scopeRadius + bezelWidth) * 2.0f, (scopeRadius + bezelWidth) * 2.0f, 1.0f);

    // === CRT SCREEN BACKGROUND ===
    juce::ColourGradient screenGrad(juce::Colour(0xFF0A0808), centreX, centreY - scopeRadius * 0.5f,
                                     juce::Colour(0xFF050404), centreX, centreY + scopeRadius, false);
    g.setGradientFill(screenGrad);
    g.fillEllipse(centreX - scopeRadius, centreY - scopeRadius,
                  scopeRadius * 2.0f, scopeRadius * 2.0f);

    // === INNER SHADOW (tube depth) ===
    for (int i = 8; i >= 1; --i)
    {
        float shadowRadius = scopeRadius - static_cast<float>(i) * 2.0f;
        float alpha = 0.08f * static_cast<float>(i) / 8.0f;
        g.setColour(juce::Colours::black.withAlpha(alpha));
        g.drawEllipse(centreX - shadowRadius, centreY - shadowRadius,
                      shadowRadius * 2.0f, shadowRadius * 2.0f, 3.0f);
    }

    // === GRID LINES (etched glass look) ===
    g.setColour(juce::Colour(0xFF1A1512).withAlpha(0.4f));
    g.drawLine(centreX - scopeRadius * 0.85f, centreY, centreX + scopeRadius * 0.85f, centreY, 0.5f);
    g.drawLine(centreX, centreY - scopeRadius * 0.85f, centreX, centreY + scopeRadius * 0.85f, 0.5f);
    g.drawLine(centreX - scopeRadius * 0.85f, centreY - scopeRadius * 0.4f,
               centreX + scopeRadius * 0.85f, centreY - scopeRadius * 0.4f, 0.3f);
    g.drawLine(centreX - scopeRadius * 0.85f, centreY + scopeRadius * 0.4f,
               centreX + scopeRadius * 0.85f, centreY + scopeRadius * 0.4f, 0.3f);

    // === GLASS REFLECTION (convex highlight) ===
    juce::Path glassHighlight;
    glassHighlight.addArc(centreX - scopeRadius * 0.75f, centreY - scopeRadius * 0.85f,
                          scopeRadius * 1.2f, scopeRadius * 0.8f, -2.5f, -1.2f, true);
    juce::ColourGradient highlightGrad(juce::Colours::white.withAlpha(0.15f),
                                        centreX - scopeRadius * 0.3f, centreY - scopeRadius * 0.6f,
                                        juce::Colours::white.withAlpha(0.0f),
                                        centreX, centreY - scopeRadius * 0.2f, false);
    g.setGradientFill(highlightGrad);
    g.strokePath(glassHighlight, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                 juce::PathStrokeType::rounded));

    // Subtle overall glass sheen
    juce::ColourGradient glassSheen(juce::Colours::white.withAlpha(0.06f), centreX, centreY - scopeRadius,
                                     juce::Colours::transparentWhite, centreX, centreY + scopeRadius * 0.3f, false);
    g.setGradientFill(glassSheen);
    g.fillEllipse(centreX - scopeRadius, centreY - scopeRadius,
                  scopeRadius * 2.0f, scopeRadius * 2.0f);

    // Small specular highlight dot
    float specX = centreX - scopeRadius * 0.5f;
    float specY = centreY - scopeRadius * 0.5f;
    juce::ColourGradient specGrad(juce::Colours::white.withAlpha(0.25f), specX, specY,
                                   juce::Colours::transparentWhite, specX + 15, specY + 15, true);
    g.setGradientFill(specGrad);
    g.fillEllipse(specX - 8, specY - 8, 16, 16);

    // === INNER RIM (glass edge) ===
    g.setColour(juce::Colour(0xFF2A2A2A));
    g.drawEllipse(centreX - scopeRadius, centreY - scopeRadius,
                  scopeRadius * 2.0f, scopeRadius * 2.0f, 2.0f);
}

void OscilloscopeComponent::drawWaveform(juce::Graphics& g, float centreX, float centreY, float scopeRadius)
{
    // Clip to circular area
    juce::Path clipPath;
    clipPath.addEllipse(centreX - scopeRadius + 2, centreY - scopeRadius + 2,
                       (scopeRadius - 2) * 2.0f, (scopeRadius - 2) * 2.0f);
    g.saveState();
    g.reduceClipRegion(clipPath);

    // === WAVEFORM with phosphor glow ===
    juce::Path waveform;
    float waveWidth = scopeRadius * 1.7f;
    float waveHeight = scopeRadius * 0.75f;
    float startX = centreX - waveWidth / 2.0f;

    bool started = false;
    for (int i = 0; i < scopeSize; ++i)
    {
        float sampleX = startX + (static_cast<float>(i) / static_cast<float>(scopeSize - 1)) * waveWidth;
        float sampleY = centreY - scopeData[i] * waveHeight;
        sampleY = juce::jlimit(centreY - scopeRadius * 0.85f, centreY + scopeRadius * 0.85f, sampleY);

        if (!started)
        {
            waveform.startNewSubPath(sampleX, sampleY);
            started = true;
        }
        else
        {
            waveform.lineTo(sampleX, sampleY);
        }
    }

    // Simplified glow - just 2 layers instead of 5 for performance
    g.setColour(SanguinovaLookAndFeel::crimsonBright.withAlpha(0.08f));
    g.strokePath(waveform, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                 juce::PathStrokeType::rounded));

    g.setColour(SanguinovaLookAndFeel::crimsonBright.withAlpha(0.15f));
    g.strokePath(waveform, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                 juce::PathStrokeType::rounded));

    // Main trace
    g.setColour(SanguinovaLookAndFeel::crimsonBright);
    g.strokePath(waveform, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                 juce::PathStrokeType::rounded));

    // Bright center of trace
    g.setColour(SanguinovaLookAndFeel::crimsonBright.interpolatedWith(juce::Colours::white, 0.5f));
    g.strokePath(waveform, juce::PathStrokeType(1.0f, juce::PathStrokeType::curved,
                 juce::PathStrokeType::rounded));

    g.restoreState();
}

//==============================================================================
// SanguinovaAudioProcessorEditor
//==============================================================================
SanguinovaAudioProcessorEditor::SanguinovaAudioProcessorEditor(SanguinovaAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      stage2xButton("STAGE I", "2x"),
      stage5xButton("STAGE II", "5x"),
      stage10xButton("STAGE III", "10x")
{
    setLookAndFeel(&lookAndFeel);

    // Enable OpenGL hardware acceleration
    openGLContext.setComponentPaintingEnabled(true);
    openGLContext.setContinuousRepainting(false);
    openGLContext.attachTo(*this);

    // Load logo from binary data
    logoImage = juce::ImageCache::getFromMemory(BinaryData::company_logo_png,
                                                 BinaryData::company_logo_pngSize);

    // Title (left-aligned to match other plugins)
    titleLabel.setText("SANGUINOVA", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, SanguinovaLookAndFeel::crimsonBright);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // Preset Controls
    refreshPresetList();
    presetBox.onChange = [this]() {
        int idx = presetBox.getSelectedItemIndex();
        if (idx >= 0)
            audioProcessor.getPresetManager().loadPreset(idx);
    };
    addAndMakeVisible(presetBox);

    savePresetButton.onClick = [this]() { savePresetDialog(); };
    addAndMakeVisible(savePresetButton);

    // Knob setup
    auto setupKnob = [this](juce::Slider& knob, juce::Label& label, const juce::String& text,
                            const juce::String& suffix = "") {
        knob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
        knob.setColour(juce::Slider::textBoxTextColourId, SanguinovaLookAndFeel::textLight);
        knob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xFF0D0D0D));
        knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xFF252525));
        if (suffix.isNotEmpty())
            knob.setTextValueSuffix(suffix);
        addAndMakeVisible(knob);

        label.setText(text, juce::dontSendNotification);
        label.setFont(juce::Font(10.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, SanguinovaLookAndFeel::textDim);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    };

    // LEFT - Input Section
    setupKnob(inputQKnob, inputQLabel, "PRE-BAND");
    setupKnob(colorKnob, colorLabel, "COLOR", " Hz");

    filterModeBox.addItem("LP", 1);
    filterModeBox.addItem("HP", 2);
    filterModeBox.addItem("BP", 3);
    addAndMakeVisible(filterModeBox);

    filterModeLabel.setText("FILTER MODE", juce::dontSendNotification);
    filterModeLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    filterModeLabel.setColour(juce::Label::textColourId, SanguinovaLookAndFeel::textDim);
    filterModeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterModeLabel);

    // CENTER - Pre-Amp Section
    setupKnob(driveKnob, driveLabel, "PRE-AMP", " dB");

    // Oscilloscope overlaid on the drive knob center
    addAndMakeVisible(oscilloscope);

    addAndMakeVisible(stage2xButton);
    addAndMakeVisible(stage5xButton);
    addAndMakeVisible(stage10xButton);

    multiplierDisplay.setText("1x", juce::dontSendNotification);
    multiplierDisplay.setFont(juce::Font(24.0f, juce::Font::bold));
    multiplierDisplay.setColour(juce::Label::textColourId, SanguinovaLookAndFeel::crimsonBright);
    multiplierDisplay.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(multiplierDisplay);

    // RIGHT - Output Section
    setupKnob(outputLpKnob, outputLpLabel, "POST-FILTER", " Hz");
    setupKnob(outputGainKnob, outputGainLabel, "TRIM", " dB");
    setupKnob(mixKnob, mixLabel, "MIX", "%");

    padButton.setButtonText("PAD");
    addAndMakeVisible(padButton);

    // Parameter attachments
    inputQAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "INPUT_Q", inputQKnob);
    colorAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "COLOR", colorKnob);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "DRIVE", driveKnob);
    outputLpAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "OUTPUT_LP", outputLpKnob);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "OUTPUT_GAIN", outputGainKnob);
    filterModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getState(), "FILTER_MODE", filterModeBox);
    stage2xAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getState(), "STAGE_2X", stage2xButton);
    stage5xAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getState(), "STAGE_5X", stage5xButton);
    stage10xAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getState(), "STAGE_10X", stage10xButton);
    padAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getState(), "PAD_ENABLED", padButton);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getState(), "MIX", mixKnob);

    startTimerHz(30);
    setSize(820, 580);  // Wider to fit larger center knob
}

SanguinovaAudioProcessorEditor::~SanguinovaAudioProcessorEditor()
{
    stopTimer();
    openGLContext.detach();
    setLookAndFeel(nullptr);
}

void SanguinovaAudioProcessorEditor::timerCallback()
{
    // Normalize drive from 0-40dB to 0-1
    float drive = static_cast<float>(driveKnob.getValue() / 40.0);

    // Calculate multiplier directly from button states (works without audio)
    float multiplier = 1.0f;
    if (stage2xButton.getToggleState()) multiplier *= 2.0f;
    if (stage5xButton.getToggleState()) multiplier *= 5.0f;
    if (stage10xButton.getToggleState()) multiplier *= 10.0f;

    lookAndFeel.setDriveIntensity(drive);
    lookAndFeel.setMultiplierLevel(multiplier);

    // Fetch oscilloscope data and update the oscilloscope component
    // (it handles its own smart repainting)
    std::array<float, SanguinovaAudioProcessor::scopeSize> scopeData;
    audioProcessor.getScopeData(scopeData);
    oscilloscope.setScopeData(scopeData);

    int mult = static_cast<int>(multiplier);
    multiplierDisplay.setText(juce::String(mult) + "x", juce::dontSendNotification);

    // Only repaint the core visualization area if intensity changed significantly
    float currentIntensity = drive * (1.0f + std::log2(std::max(1.0f, multiplier)) * 0.15f);
    if (std::abs(currentIntensity - lastCoreIntensity) > 0.02f)
    {
        lastCoreIntensity = currentIntensity;
        // Repaint only the center area where core visualization is
        repaint(getWidth() / 4, 60, getWidth() / 2, 200);
    }
}

void SanguinovaAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Obsidian background
    g.fillAll(SanguinovaLookAndFeel::backgroundDark);

    // Subtle gradient for depth
    juce::ColourGradient bgGradient(juce::Colour(0xFF080808), 0.0f, 0.0f,
                                     juce::Colour(0xFF040404), 0.0f, static_cast<float>(getHeight()), false);
    g.setGradientFill(bgGradient);
    g.fillAll();

    // Get current intensity values
    float driveNorm = static_cast<float>(driveKnob.getValue() / 40.0);
    float multiplier = audioProcessor.getTotalMultiplier();
    float coreIntensity = driveNorm * (1.0f + std::log2(std::max(1.0f, multiplier)) * 0.15f);
    coreIntensity = juce::jlimit(0.0f, 1.5f, coreIntensity);

    // === CORE VISUALIZATION - Blood Star Nova ===
    // Position in center panel area
    float coreX = static_cast<float>(getWidth()) * 0.5f;
    float coreY = 180.0f;  // Below the title, in drive knob area
    float baseRadius = 30.0f;
    float expandedRadius = baseRadius + coreIntensity * 60.0f;

    // Outer glow layers (expanding nova)
    for (int layer = 8; layer >= 1; --layer)
    {
        float layerRadius = expandedRadius + static_cast<float>(layer) * 12.0f * coreIntensity;
        float alpha = (0.03f / static_cast<float>(layer)) * coreIntensity;

        juce::ColourGradient glowGrad(
            SanguinovaLookAndFeel::crimsonBright.withAlpha(alpha), coreX, coreY,
            juce::Colours::transparentBlack, coreX - layerRadius, coreY, true);
        g.setGradientFill(glowGrad);
        g.fillEllipse(coreX - layerRadius, coreY - layerRadius,
                      layerRadius * 2.0f, layerRadius * 2.0f);
    }

    // Core center (bright plasma)
    if (coreIntensity > 0.05f)
    {
        juce::ColourGradient coreGrad(
            SanguinovaLookAndFeel::crimsonBright.withAlpha(0.3f * coreIntensity),
            coreX, coreY,
            SanguinovaLookAndFeel::crimsonDark.withAlpha(0.1f * coreIntensity),
            coreX, coreY + expandedRadius, true);
        g.setGradientFill(coreGrad);
        g.fillEllipse(coreX - expandedRadius, coreY - expandedRadius,
                      expandedRadius * 2.0f, expandedRadius * 2.0f);
    }

    // Vignette effect (intensifies with multiplier)
    float vignetteIntensity = 0.1f + std::log2(std::max(1.0f, multiplier)) / std::log2(100.0f) * 0.2f;
    juce::ColourGradient vignette(juce::Colours::transparentBlack,
                                   static_cast<float>(getWidth()) * 0.5f,
                                   static_cast<float>(getHeight()) * 0.5f,
                                   SanguinovaLookAndFeel::crimsonDark.withAlpha(vignetteIntensity),
                                   0.0f, 0.0f, true);
    g.setGradientFill(vignette);
    g.fillAll();

    // Section panels - center is wider
    auto bounds = getLocalBounds();
    bounds.removeFromTop(60); // Title area
    int totalWidth = bounds.getWidth();
    int sideWidth = static_cast<int>(totalWidth * 0.25f);
    int centerWidth = totalWidth - (sideWidth * 2);

    // Left panel background
    auto leftPanel = bounds.removeFromLeft(sideWidth).reduced(8, 8);
    g.setColour(juce::Colour(0xFF080808));
    g.fillRoundedRectangle(leftPanel.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xFF181818));
    g.drawRoundedRectangle(leftPanel.toFloat(), 8.0f, 1.0f);

    // Center panel (plasma glow border)
    auto centerPanel = bounds.removeFromLeft(centerWidth).reduced(8, 8);
    g.setColour(juce::Colour(0xFF0A0A0A));
    g.fillRoundedRectangle(centerPanel.toFloat(), 8.0f);
    g.setColour(SanguinovaLookAndFeel::crimsonDark.withAlpha(0.4f + coreIntensity * 0.3f));
    g.drawRoundedRectangle(centerPanel.toFloat(), 8.0f, 1.5f);

    // Right panel background
    auto rightPanel = bounds.reduced(8, 8);
    g.setColour(juce::Colour(0xFF080808));
    g.fillRoundedRectangle(rightPanel.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xFF181818));
    g.drawRoundedRectangle(rightPanel.toFloat(), 8.0f, 1.0f);

    // Section labels
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.setColour(SanguinovaLookAndFeel::textDim.withAlpha(0.6f));
    g.drawText("INPUT", leftPanel.removeFromTop(20), juce::Justification::centred);
    g.setColour(SanguinovaLookAndFeel::crimsonBase.withAlpha(0.5f + coreIntensity * 0.3f));
    g.drawText("CORE", centerPanel.removeFromTop(20), juce::Justification::centred);
    g.setColour(SanguinovaLookAndFeel::textDim.withAlpha(0.6f));
    g.drawText("OUTPUT", rightPanel.removeFromTop(20), juce::Justification::centred);

    // Title underline with plasma glow
    juce::ColourGradient underlineGrad(
        SanguinovaLookAndFeel::crimsonDark, static_cast<float>(getWidth() / 2 - 60), 52.0f,
        SanguinovaLookAndFeel::crimsonBright.withAlpha(0.5f + coreIntensity * 0.3f),
        static_cast<float>(getWidth() / 2 + 60), 52.0f, false);
    g.setGradientFill(underlineGrad);
    g.fillRect(getWidth() / 2 - 60, 52, 120, 2);
}

void SanguinovaAudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    // Draw company logo centered in header (on top of all components)
    if (!logoImage.isValid())
        return;

    const int headerHeight = 55;
    const float logoHeight = 30.0f;
    const float logoAspect = static_cast<float>(logoImage.getWidth()) / static_cast<float>(logoImage.getHeight());
    const float logoWidth = logoHeight * logoAspect;

    // Center logo horizontally in header
    const float logoX = (static_cast<float>(getWidth()) - logoWidth) * 0.5f;
    const float logoY = (static_cast<float>(headerHeight) - logoHeight) * 0.5f;

    juce::Rectangle<float> logoBounds(logoX, logoY, logoWidth, logoHeight);
    g.drawImage(logoImage, logoBounds, juce::RectanglePlacement::centred);
}

void SanguinovaAudioProcessorEditor::refreshPresetList()
{
    presetBox.clear();
    auto names = audioProcessor.getPresetManager().getPresetNames();
    int id = 1;
    for (const auto& name : names)
        presetBox.addItem(name, id++);

    // Select current preset
    auto currentName = audioProcessor.getPresetManager().getCurrentPresetName();
    for (int i = 0; i < names.size(); ++i)
    {
        if (names[i] == currentName)
        {
            presetBox.setSelectedItemIndex(i, juce::dontSendNotification);
            break;
        }
    }
}

void SanguinovaAudioProcessorEditor::savePresetDialog()
{
    auto* alertWindow = new juce::AlertWindow(
        "Save Preset",
        "Enter a name for your preset:",
        juce::MessageBoxIconType::NoIcon);

    alertWindow->addTextEditor("presetName", "", "Preset Name:");
    alertWindow->addButton("Save", 1);
    alertWindow->addButton("Cancel", 0);

    alertWindow->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, alertWindow](int result) {
            if (result == 1)
            {
                auto name = alertWindow->getTextEditorContents("presetName");
                if (name.isNotEmpty())
                {
                    audioProcessor.getPresetManager().savePreset(name);
                    refreshPresetList();
                }
            }
            delete alertWindow;
        }));
}

void SanguinovaAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Title area with preset controls
    auto titleArea = bounds.removeFromTop(55);

    // Title on the left (matching other plugins)
    titleLabel.setBounds(titleArea.removeFromLeft(180).reduced(15, 10));

    // Preset controls on the right (matching other plugins)
    auto presetArea = titleArea.removeFromRight(250);
    presetArea.reduce(5, 12);
    presetBox.setBounds(presetArea.removeFromLeft(140));
    presetArea.removeFromLeft(5);
    savePresetButton.setBounds(presetArea.removeFromLeft(60));

    // Three sections - center is wider for large knob
    bounds.reduce(12, 12);
    int totalWidth = bounds.getWidth();
    int sideWidth = static_cast<int>(totalWidth * 0.25f);   // 25% each side
    int centerWidth = totalWidth - (sideWidth * 2);         // 50% center

    auto leftSection = bounds.removeFromLeft(sideWidth).reduced(6, 6);
    auto centerSection = bounds.removeFromLeft(centerWidth).reduced(6, 6);
    auto rightSection = bounds.reduced(6, 6);

    // Skip section headers
    leftSection.removeFromTop(25);
    centerSection.removeFromTop(25);
    rightSection.removeFromTop(25);

    // === LEFT SECTION - Input ===
    int smallKnobSize = 100;

    // Input Q
    auto inputQArea = leftSection.removeFromTop(smallKnobSize + 20);
    inputQLabel.setBounds(inputQArea.removeFromTop(18));
    inputQKnob.setBounds(inputQArea.withSizeKeepingCentre(smallKnobSize, smallKnobSize));

    leftSection.removeFromTop(8);

    // Color
    auto colorArea = leftSection.removeFromTop(smallKnobSize + 20);
    colorLabel.setBounds(colorArea.removeFromTop(18));
    colorKnob.setBounds(colorArea.withSizeKeepingCentre(smallKnobSize, smallKnobSize));

    leftSection.removeFromTop(15);

    // Filter mode
    filterModeLabel.setBounds(leftSection.removeFromTop(16));
    leftSection.removeFromTop(4);
    filterModeBox.setBounds(leftSection.removeFromTop(32).reduced(20, 0));

    // === CENTER SECTION - Pre-Amp with Oscilloscope ===
    int driveKnobSize = 280;  // 1.75x larger (160 * 1.75)

    // Drive (large)
    auto driveArea = centerSection.removeFromTop(driveKnobSize + 25);
    driveLabel.setBounds(driveArea.removeFromTop(20));
    auto driveKnobBounds = driveArea.withSizeKeepingCentre(driveKnobSize, driveKnobSize);
    driveKnob.setBounds(driveKnobBounds);

    // Position oscilloscope in the center of the drive knob
    // The knob radius is driveKnobSize/2, inner knob is 0.65 of that, scope is 0.92 of inner
    float knobRadius = driveKnobSize / 2.0f;
    float innerKnobRadius = knobRadius * 0.65f;
    float scopeRadius = innerKnobRadius * 0.92f;
    int scopeSize = static_cast<int>(scopeRadius * 2.0f);
    auto scopeBounds = driveKnobBounds.withSizeKeepingCentre(scopeSize, scopeSize);
    oscilloscope.setBounds(scopeBounds);

    centerSection.removeFromTop(12);

    // Stage buttons
    auto stageRow = centerSection.removeFromTop(55);
    int stageWidth = stageRow.getWidth() / 3;
    stage2xButton.setBounds(stageRow.removeFromLeft(stageWidth).reduced(4, 0));
    stage5xButton.setBounds(stageRow.removeFromLeft(stageWidth).reduced(4, 0));
    stage10xButton.setBounds(stageRow.reduced(4, 0));

    centerSection.removeFromTop(8);

    // Multiplier display
    multiplierDisplay.setBounds(centerSection.removeFromTop(35));

    // === RIGHT SECTION - Output ===
    int rightKnobSize = 80;  // Smaller to fit 3 knobs

    // Output LP
    auto outputLpArea = rightSection.removeFromTop(rightKnobSize + 18);
    outputLpLabel.setBounds(outputLpArea.removeFromTop(16));
    outputLpKnob.setBounds(outputLpArea.withSizeKeepingCentre(rightKnobSize, rightKnobSize));

    rightSection.removeFromTop(4);

    // Output Gain
    auto outputGainArea = rightSection.removeFromTop(rightKnobSize + 18);
    outputGainLabel.setBounds(outputGainArea.removeFromTop(16));
    outputGainKnob.setBounds(outputGainArea.withSizeKeepingCentre(rightKnobSize, rightKnobSize));

    rightSection.removeFromTop(4);

    // Mix
    auto mixArea = rightSection.removeFromTop(rightKnobSize + 18);
    mixLabel.setBounds(mixArea.removeFromTop(16));
    mixKnob.setBounds(mixArea.withSizeKeepingCentre(rightKnobSize, rightKnobSize));

    rightSection.removeFromTop(8);

    // Pad button
    padButton.setBounds(rightSection.removeFromTop(28).reduced(15, 0));
}
