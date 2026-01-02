#pragma once

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * SVFFilter - State Variable Filter
 *
 * Multi-mode filter offering:
 * - High Pass (Stellar Flare): Distorts only the highs
 * - Low Pass (Deep Core): Distorts only the lows
 * - Band Pass (Coronal Loop): Focused resonant distortion
 */
class SVFFilter
{
public:
    enum class Mode
    {
        LowPass = 0,
        HighPass,
        BandPass
    };

    SVFFilter() : ic1eq(0.0f), ic2eq(0.0f), sampleRate(44100.0f) {}

    void prepare(float newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void reset()
    {
        ic1eq = 0.0f;
        ic2eq = 0.0f;
    }

    /**
     * Set filter parameters
     * @param frequency Cutoff frequency in Hz (20-20000)
     * @param resonance Q factor (0.1-1.0, mapped internally)
     */
    void setParameters(float frequency, float resonance)
    {
        // Clamp frequency
        frequency = std::fmax(20.0f, std::fmin(frequency, sampleRate * 0.49f));

        // Map resonance (0.1-1.0) to Q (0.5-10.0)
        float q = 0.5f + resonance * 9.5f;

        // Calculate coefficients using the TPT (Topology Preserving Transform)
        g = std::tan(static_cast<float>(M_PI) * frequency / sampleRate);
        k = 1.0f / q;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    /**
     * Process a single sample
     * @param input The input sample
     * @param mode The filter mode (LP, HP, BP)
     * @return The filtered sample
     */
    float processSample(float input, Mode mode)
    {
        // TPT SVF processing
        float v3 = input - ic2eq;
        float v1 = a1 * ic1eq + a2 * v3;
        float v2 = ic2eq + a2 * ic1eq + a3 * v3;

        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;

        // Select output based on mode
        switch (mode)
        {
            case Mode::LowPass:
                return v2;

            case Mode::HighPass:
                return input - k * v1 - v2;

            case Mode::BandPass:
                return v1;

            default:
                return v2;
        }
    }

    /**
     * Process a block of samples
     */
    void processBlock(float* buffer, int numSamples, Mode mode)
    {
        for (int i = 0; i < numSamples; ++i)
        {
            buffer[i] = processSample(buffer[i], mode);
        }
    }

private:
    float ic1eq, ic2eq;     // State variables
    float sampleRate;
    float g, k;             // Filter coefficients
    float a1, a2, a3;       // Derived coefficients
};
