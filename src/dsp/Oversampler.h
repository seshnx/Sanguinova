#pragma once

#include <cmath>
#include <array>
#include <algorithm>

/**
 * Oversampler - 4x Oversampling for Anti-Aliasing
 *
 * Uses polyphase FIR filter for efficient up/downsampling.
 * Critical for preventing aliasing in nonlinear distortion.
 */
class Oversampler
{
public:
    static constexpr int Factor = 4;
    static constexpr int FilterOrder = 32;  // FIR filter taps
    static constexpr int HalfOrder = FilterOrder / 2;

    Oversampler()
    {
        initializeFilter();
        reset();
    }

    void reset()
    {
        std::fill(upsampleBuffer.begin(), upsampleBuffer.end(), 0.0f);
        std::fill(downsampleBuffer.begin(), downsampleBuffer.end(), 0.0f);
        upsampleIndex = 0;
        downsampleIndex = 0;
    }

    /**
     * Upsample a single input sample to 4 output samples
     * @param input Single input sample
     * @param output Array of 4 upsampled samples
     */
    void upsample(float input, std::array<float, Factor>& output)
    {
        // Insert input with zero-stuffing
        upsampleBuffer[upsampleIndex] = input * Factor;  // Compensate for energy loss

        // Generate 4 output samples using polyphase decomposition
        for (int phase = 0; phase < Factor; ++phase)
        {
            float sum = 0.0f;
            for (int tap = 0; tap < FilterOrder; ++tap)
            {
                int bufIdx = (upsampleIndex - tap + FilterOrder) % FilterOrder;
                int coeffIdx = tap * Factor + phase;
                if (coeffIdx < FilterOrder * Factor)
                {
                    sum += upsampleBuffer[bufIdx] * getFilterCoeff(coeffIdx);
                }
            }
            output[phase] = sum;
        }

        upsampleIndex = (upsampleIndex + 1) % FilterOrder;
    }

    /**
     * Downsample 4 input samples to 1 output sample
     * @param input Array of 4 oversampled samples
     * @return Single downsampled output
     */
    float downsample(const std::array<float, Factor>& input)
    {
        float sum = 0.0f;

        // Anti-aliasing filter and decimate
        for (int phase = 0; phase < Factor; ++phase)
        {
            // Shift buffer and insert new sample
            downsampleBuffer[downsampleIndex] = input[phase];

            // Apply filter at decimation point
            if (phase == Factor - 1)
            {
                for (int tap = 0; tap < FilterOrder; ++tap)
                {
                    int bufIdx = (downsampleIndex - tap * Factor + FilterOrder * Factor) % (FilterOrder * Factor);
                    if (bufIdx < static_cast<int>(downsampleBuffer.size()))
                    {
                        sum += downsampleBuffer[bufIdx] * filterCoeffs[tap];
                    }
                }
            }

            downsampleIndex = (downsampleIndex + 1) % (FilterOrder * Factor);
        }

        return sum;
    }

    /**
     * Process a sample through oversampling with a waveshaper function
     * @param input Input sample
     * @param processor Lambda/function that processes each oversampled sample
     * @return Downsampled output
     */
    template<typename ProcessFunc>
    float process(float input, ProcessFunc processor)
    {
        std::array<float, Factor> upsampled;
        upsample(input, upsampled);

        // Process each oversampled sample through the nonlinearity
        for (int i = 0; i < Factor; ++i)
        {
            upsampled[i] = processor(upsampled[i]);
        }

        return downsample(upsampled);
    }

private:
    void initializeFilter()
    {
        // Design lowpass FIR filter at Nyquist/4 (cutoff = 0.25 * Fs)
        // Using windowed-sinc design with Kaiser window
        constexpr float cutoff = 0.22f;  // Slightly below Nyquist/4 for better rolloff
        constexpr float beta = 7.0f;     // Kaiser window beta

        for (int i = 0; i < FilterOrder; ++i)
        {
            float n = static_cast<float>(i) - static_cast<float>(HalfOrder - 1);

            // Sinc function
            float sinc;
            if (std::fabs(n) < 1e-6f)
            {
                sinc = 2.0f * cutoff;
            }
            else
            {
                float x = 2.0f * 3.14159265359f * cutoff * n;
                sinc = std::sin(x) / (3.14159265359f * n);
            }

            // Kaiser window
            float windowArg = 1.0f - std::pow(2.0f * static_cast<float>(i) / (FilterOrder - 1) - 1.0f, 2.0f);
            windowArg = std::max(0.0f, windowArg);
            float window = bessel_i0(beta * std::sqrt(windowArg)) / bessel_i0(beta);

            filterCoeffs[i] = sinc * window;
        }

        // Normalize filter
        float sum = 0.0f;
        for (int i = 0; i < FilterOrder; ++i)
        {
            sum += filterCoeffs[i];
        }
        for (int i = 0; i < FilterOrder; ++i)
        {
            filterCoeffs[i] /= sum;
        }
    }

    float getFilterCoeff(int index) const
    {
        int coeffIdx = index / Factor;
        if (coeffIdx < FilterOrder)
        {
            return filterCoeffs[coeffIdx];
        }
        return 0.0f;
    }

    // Modified Bessel function of the first kind, order 0
    static float bessel_i0(float x)
    {
        float sum = 1.0f;
        float term = 1.0f;
        float xHalfSq = (x / 2.0f) * (x / 2.0f);

        for (int k = 1; k < 20; ++k)
        {
            term *= xHalfSq / (static_cast<float>(k) * static_cast<float>(k));
            sum += term;
            if (term < 1e-10f) break;
        }

        return sum;
    }

    std::array<float, FilterOrder> filterCoeffs{};
    std::array<float, FilterOrder> upsampleBuffer{};
    std::array<float, FilterOrder * Factor> downsampleBuffer{};
    int upsampleIndex = 0;
    int downsampleIndex = 0;
};
