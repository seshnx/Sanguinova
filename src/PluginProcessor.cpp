#include "PluginProcessor.h"
#include "PluginEditor.h"

SanguinovaAudioProcessor::SanguinovaAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

SanguinovaAudioProcessor::~SanguinovaAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout SanguinovaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === INPUT SECTION (Left) ===

    // Input Filter Q (0.1 - 1.0)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"INPUT_Q", 1},
        "Input Q",
        juce::NormalisableRange<float>(0.1f, 1.0f, 0.01f),
        0.5f));

    // Input Filter Frequency / Color (20Hz - 20kHz, logarithmic)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"COLOR", 1},
        "Color",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f),
        1000.0f));

    // Filter Mode (HP, LP, BP)
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"FILTER_MODE", 1},
        "Filter Mode",
        juce::StringArray{"Low Pass", "High Pass", "Band Pass"},
        2));  // Default to Band Pass

    // === CENTER SECTION ===

    // Pre-Amp / Drive (0-40 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"DRIVE", 1},
        "Drive",
        juce::NormalisableRange<float>(0.0f, 40.0f, 0.1f, 0.5f),
        0.0f));  // Default to 0 (no overdrive)

    // === OUTPUT SECTION (Right) ===

    // Post-Filter / Output LowPass (2kHz - 20kHz, 1-pole LPF)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"OUTPUT_LP", 1},
        "Post Filter",
        juce::NormalisableRange<float>(2000.0f, 20000.0f, 1.0f, 0.25f),
        20000.0f));  // Default wide open

    // Post-Gain / Trim (-12 to +12 dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"OUTPUT_GAIN", 1},
        "Trim",
        juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f),
        0.0f));

    // Ignition Stages (Boolean toggles)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"STAGE_2X", 1},
        "Stage I (2x)",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"STAGE_5X", 1},
        "Stage II (5x)",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"STAGE_10X", 1},
        "Stage III (10x)",
        false));

    // Pad Enable (compensates for multiplier gain)
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"PAD_ENABLED", 1},
        "Pad",
        true));  // Default on

    // Wet/Dry Mix (0-100%)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"MIX", 1},
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
        100.0f));  // Default 100% wet

    return { params.begin(), params.end() };
}

const juce::String SanguinovaAudioProcessor::getName() const
{
    return "Sanguinova";
}

bool SanguinovaAudioProcessor::acceptsMidi() const
{
    return false;
}

bool SanguinovaAudioProcessor::producesMidi() const
{
    return false;
}

bool SanguinovaAudioProcessor::isMidiEffect() const
{
    return false;
}

double SanguinovaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SanguinovaAudioProcessor::getNumPrograms()
{
    return 1;
}

int SanguinovaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SanguinovaAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String SanguinovaAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SanguinovaAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

void SanguinovaAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    // Prepare all DSP components
    for (int ch = 0; ch < 2; ++ch)
    {
        preFilters[ch].prepare(static_cast<float>(sampleRate));
        postFilters[ch].prepare(static_cast<float>(sampleRate));
    }

    // Calculate pad smoothing coefficient
    // Fast attack (~5ms), slow release (~150ms) for soft deactivation
    float attackMs = 5.0f;
    float releaseMs = 150.0f;
    padAttackCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * attackMs / 1000.0f));
    padReleaseCoeff = std::exp(-1.0f / (static_cast<float>(sampleRate) * releaseMs / 1000.0f));
    smoothedPadGain = 1.0f;  // Start at unity
}

void SanguinovaAudioProcessor::releaseResources()
{
    for (int ch = 0; ch < 2; ++ch)
    {
        preFilters[ch].reset();
        postFilters[ch].reset();
        oversamplers[ch].reset();
    }
}

bool SanguinovaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void SanguinovaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get parameters
    float inputQ = *state.getRawParameterValue("INPUT_Q");
    float color = *state.getRawParameterValue("COLOR");
    int filterModeInt = static_cast<int>(*state.getRawParameterValue("FILTER_MODE"));
    float drive = *state.getRawParameterValue("DRIVE");
    float outputLpFreq = *state.getRawParameterValue("OUTPUT_LP");
    float outputGainDb = *state.getRawParameterValue("OUTPUT_GAIN");
    float mixPercent = *state.getRawParameterValue("MIX");
    float wetAmount = mixPercent / 100.0f;
    float dryAmount = 1.0f - wetAmount;

    // Calculate stage multipliers (combinatorial)
    float stage1 = *state.getRawParameterValue("STAGE_2X") > 0.5f ? 2.0f : 1.0f;
    float stage2 = *state.getRawParameterValue("STAGE_5X") > 0.5f ? 5.0f : 1.0f;
    float stage3 = *state.getRawParameterValue("STAGE_10X") > 0.5f ? 10.0f : 1.0f;
    float stageMult = stage1 * stage2 * stage3;

    // Calculate pad based on multiplier (compensates for gain increase from overdrive stages)
    // Pad = 1/multiplier in linear, which equals -20*log10(multiplier) in dB
    bool padEnabled = *state.getRawParameterValue("PAD_ENABLED") > 0.5f;
    float targetPadGain = padEnabled ? (1.0f / stageMult) : 1.0f;

    // Store for UI
    totalMultiplier.store(stageMult);

    // Convert filter mode
    SVFFilter::Mode filterMode = static_cast<SVFFilter::Mode>(filterModeInt);

    // Convert output gain from dB to linear
    float outputGainLinear = juce::Decibels::decibelsToGain(outputGainDb);

    // Process each channel
    int numSamples = buffer.getNumSamples();
    int numChannels = std::min(totalNumInputChannels, 2);

    float maxInputLevel = 0.0f;
    float maxOutputLevel = 0.0f;

    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        // Update filter parameters
        preFilters[channel].setParameters(color, inputQ);
        postFilters[channel].setFrequency(outputLpFreq);  // 1-pole LPF

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float input = channelData[sample];
            maxInputLevel = std::max(maxInputLevel, std::fabs(input));

            // 1. Pre-Filter (SVF) - The "Color" stage
            float filtered = preFilters[channel].processSample(input, filterMode);

            // 2. Distortion Engine with 4x Oversampling
            float distorted = oversamplers[channel].process(filtered, [&](float x) {
                return engines[channel].processSample(x, drive, stageMult);
            });

            // 3. Output 1-pole LowPass Filter (smooths harsh harmonics)
            float postFiltered = postFilters[channel].processSample(distorted);

            // 4. Smooth pad transition (fast attack, slow release for soft deactivation)
            if (channel == 0)  // Only update once per sample
            {
                float coeff = (targetPadGain < smoothedPadGain) ? padAttackCoeff : padReleaseCoeff;
                smoothedPadGain = smoothedPadGain * coeff + targetPadGain * (1.0f - coeff);
            }

            // 5. Apply smoothed pad (compensates for multiplier gain) and output gain
            float wetSignal = postFiltered * smoothedPadGain * outputGainLinear;

            // 5. Apply wet/dry mix
            float output = (wetSignal * wetAmount) + (input * dryAmount);

            channelData[sample] = output;
            maxOutputLevel = std::max(maxOutputLevel, std::fabs(output));

            // 8. Write to oscilloscope buffer (mono sum, decimated)
            if (channel == 0)
            {
                if (++scopeDecimation >= scopeDecimationFactor)
                {
                    scopeDecimation = 0;
                    int pos = scopeWritePos.load();
                    scopeBuffer[pos].store(output);
                    scopeWritePos.store((pos + 1) % scopeSize);
                }
            }
        }
    }

    // Update metering
    currentInputLevel.store(maxInputLevel);
    currentOutputLevel.store(maxOutputLevel);
    currentGR.store(smoothedPadGain);  // Store smoothed pad value for UI display
}

bool SanguinovaAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SanguinovaAudioProcessor::createEditor()
{
    return new SanguinovaAudioProcessorEditor(*this);
}

void SanguinovaAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto stateTree = state.copyState();
    std::unique_ptr<juce::XmlElement> xml(stateTree.createXml());
    copyXmlToBinary(*xml, destData);
}

void SanguinovaAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(state.state.getType()))
    {
        state.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SanguinovaAudioProcessor();
}
