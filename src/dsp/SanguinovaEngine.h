#pragma once

#include <cmath>

/**
 * SanguinovaEngine - Core Distortion Engine
 *
 * Implements Hyperbolic Asymmetry algorithm:
 * - Positive cycle: Exponential saturation (warm, tube-like)
 * - Negative cycle: Rational folding (gritty, compressed)
 */
class SanguinovaEngine
{
public:
    SanguinovaEngine() = default;

    /**
     * Process a single sample through the asymmetric waveshaper
     * @param input The input sample
     * @param driveDb The drive amount in dB (0-40)
     * @param stageMult The combinatorial stage multiplier (1x to 100x)
     * @return The distorted sample
     */
    float processSample(float input, float driveDb, float stageMult)
    {
        // 1. Apply Gain and Multiplier
        // Convert dB to linear gain: gain = 10^(dB/20)
        float driveLinear = std::pow(10.0f, driveDb / 20.0f);
        float x = input * driveLinear * stageMult;

        // 2. Asymmetrical Transfer Function
        float output;
        if (x > 0.0f)
        {
            // Positive cycle: Exponential saturation (warm, soft tube-like)
            output = 1.0f - std::exp(-x);
        }
        else
        {
            // Negative cycle: Rational folding (gritty, compressed)
            output = x / (1.0f + (x * x));
        }

        return output;
    }

    /**
     * Process a block of samples
     * @param buffer Pointer to the sample buffer
     * @param numSamples Number of samples to process
     * @param driveDb The drive amount in dB (0-40)
     * @param stageMult The combinatorial stage multiplier
     */
    void processBlock(float* buffer, int numSamples, float drive, float stageMult)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            buffer[i] = processSample(buffer[i], drive, stageMult);
        }
    }

private:
};
