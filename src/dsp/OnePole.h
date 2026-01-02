#pragma once

#include <cmath>

/**
 * OnePole - Simple 1-pole Low Pass Filter
 *
 * Used for post-filtering to smooth harsh harmonics.
 * Provides a gentle 6dB/octave rolloff.
 *
 * Transfer function: H(z) = g / (1 - (1-g)z^-1)
 */
class OnePole
{
public:
    OnePole() = default;

    void prepare(float sampleRate)
    {
        fs = sampleRate;
        reset();
    }

    void reset()
    {
        z1 = 0.0f;
    }

    void setFrequency(float frequency)
    {
        // Clamp frequency to valid range
        frequency = std::max(20.0f, std::min(frequency, fs * 0.49f));

        // Calculate coefficient using exact formula for 1-pole LPF
        // g = 1 - exp(-2 * pi * fc / fs)
        float w = 2.0f * 3.14159265359f * frequency / fs;
        g = 1.0f - std::exp(-w);
    }

    float processSample(float input)
    {
        // Simple 1-pole LPF: y[n] = y[n-1] + g * (x[n] - y[n-1])
        z1 = z1 + g * (input - z1);
        return z1;
    }

private:
    float fs = 44100.0f;
    float g = 1.0f;      // Filter coefficient
    float z1 = 0.0f;     // State variable
};
