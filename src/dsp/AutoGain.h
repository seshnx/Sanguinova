#pragma once

#include <cmath>
#include <algorithm>

/**
 * AutoGain - Intelligent Gain Compensation
 *
 * Measures input/output envelopes and applies correction
 * so output loudness matches input loudness.
 *
 * Uses fast attack (prevent digital spikes) and medium release
 * (prevent volume fluttering).
 */
class AutoGain
{
public:
    AutoGain()
        : inputEnvelope(0.0f)
        , outputEnvelope(0.0f)
        , gainReduction(1.0f)
        , smoothedGR(1.0f)
        , sampleRate(44100.0f)
    {
        calculateCoefficients();
    }

    void prepare(float newSampleRate)
    {
        sampleRate = newSampleRate;
        calculateCoefficients();
        reset();
    }

    void reset()
    {
        inputEnvelope = 0.0f;
        outputEnvelope = 0.0f;
        gainReduction = 1.0f;
        smoothedGR = 1.0f;
    }

    /**
     * Update the input envelope with a sample (call before processing)
     */
    void updateInputEnvelope(float sample)
    {
        float absSample = std::fabs(sample);
        float alpha = (absSample > inputEnvelope) ? attackCoeff : releaseCoeff;
        inputEnvelope = inputEnvelope + alpha * (absSample - inputEnvelope);
    }

    /**
     * Update the output envelope with a sample (call after processing)
     */
    void updateOutputEnvelope(float sample)
    {
        float absSample = std::fabs(sample);
        float alpha = (absSample > outputEnvelope) ? attackCoeff : releaseCoeff;
        outputEnvelope = outputEnvelope + alpha * (absSample - outputEnvelope);
    }

    /**
     * Calculate and return the gain reduction factor
     * GR = E_in / E_out
     */
    float getGainReduction()
    {
        // Avoid division by zero and limit extreme values
        constexpr float epsilon = 1e-6f;
        constexpr float minGR = 0.1f;   // -20dB max reduction
        constexpr float maxGR = 5.0f;   // +14dB max boost

        if (outputEnvelope > epsilon)
        {
            gainReduction = inputEnvelope / outputEnvelope;
            gainReduction = std::clamp(gainReduction, minGR, maxGR);
        }
        else
        {
            gainReduction = 1.0f;
        }

        // Smooth the gain reduction to avoid sudden jumps
        smoothedGR = smoothedGR + grSmoothCoeff * (gainReduction - smoothedGR);

        return smoothedGR;
    }

    /**
     * Process a sample with auto-gain applied
     * This is a convenience method that applies the current GR
     */
    float applySample(float sample)
    {
        return sample * smoothedGR;
    }

    /**
     * Update both envelopes with input/output pairs from a block
     */
    void processEnvelopes(const float* inputBuffer, const float* outputBuffer, int numSamples)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            updateInputEnvelope(inputBuffer[i]);
            updateOutputEnvelope(outputBuffer[i]);
        }
    }

    float getInputLevel() const { return inputEnvelope; }
    float getOutputLevel() const { return outputEnvelope; }
    float getCurrentGR() const { return smoothedGR; }

private:
    void calculateCoefficients()
    {
        // Attack: 1ms (fast to prevent digital spikes)
        float attackTimeMs = 1.0f;
        attackCoeff = 1.0f - std::exp(-1.0f / (sampleRate * attackTimeMs * 0.001f));

        // Release: 100ms (medium to prevent fluttering)
        float releaseTimeMs = 100.0f;
        releaseCoeff = 1.0f - std::exp(-1.0f / (sampleRate * releaseTimeMs * 0.001f));

        // GR smoothing: 50ms
        float grSmoothTimeMs = 50.0f;
        grSmoothCoeff = 1.0f - std::exp(-1.0f / (sampleRate * grSmoothTimeMs * 0.001f));
    }

    float inputEnvelope;
    float outputEnvelope;
    float gainReduction;
    float smoothedGR;

    float sampleRate;
    float attackCoeff;
    float releaseCoeff;
    float grSmoothCoeff;
};
