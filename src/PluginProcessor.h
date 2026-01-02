#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "dsp/SanguinovaEngine.h"
#include "dsp/SVFFilter.h"
#include "dsp/OnePole.h"
#include "dsp/Oversampler.h"
#include "PresetManager.h"

/**
 * SanguinovaAudioProcessor
 *
 * "Blood Star" Distortion Plugin
 * Features:
 * - Asymmetric waveshaping (tube-like saturation)
 * - Multi-mode SVF pre-filter (Color control)
 * - Ignition Stages (2x, 5x, 10x combinatorial multipliers)
 * - Intelligent auto-gain compensation
 */
class SanguinovaAudioProcessor : public juce::AudioProcessor
{
public:
    SanguinovaAudioProcessor();
    ~SanguinovaAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter access
    juce::AudioProcessorValueTreeState& getState() { return state; }

    // Preset access
    PresetManager& getPresetManager() { return presetManager; }

    // Metering (for UI)
    float getCurrentInputLevel() const { return currentInputLevel.load(); }
    float getCurrentOutputLevel() const { return currentOutputLevel.load(); }
    float getCurrentGainReduction() const { return currentGR.load(); }
    float getTotalMultiplier() const { return totalMultiplier.load(); }

    // Oscilloscope buffer access
    static constexpr int scopeSize = 256;
    void getScopeData(std::array<float, scopeSize>& data) const
    {
        int pos = scopeWritePos.load();
        for (int i = 0; i < scopeSize; ++i)
        {
            int idx = (pos + i) % scopeSize;
            data[i] = scopeBuffer[idx].load();
        }
    }

private:
    juce::AudioProcessorValueTreeState state;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    PresetManager presetManager{state};

    // DSP Components (per channel)
    std::array<SanguinovaEngine, 2> engines;
    std::array<SVFFilter, 2> preFilters;
    std::array<OnePole, 2> postFilters;  // 1-pole LPF for smoothing
    std::array<Oversampler, 2> oversamplers;  // 4x oversampling

    // Pad smoothing (soft release on deactivation)
    float smoothedPadGain = 1.0f;
    float padAttackCoeff = 0.0f;
    float padReleaseCoeff = 0.0f;

    // Metering
    std::atomic<float> currentInputLevel{0.0f};
    std::atomic<float> currentOutputLevel{0.0f};
    std::atomic<float> currentGR{1.0f};
    std::atomic<float> totalMultiplier{1.0f};

    // Oscilloscope circular buffer
    std::array<std::atomic<float>, scopeSize> scopeBuffer{};
    std::atomic<int> scopeWritePos{0};
    int scopeDecimation = 0;
    int scopeDecimationFactor = 8;  // Downsample for display

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SanguinovaAudioProcessor)
};
