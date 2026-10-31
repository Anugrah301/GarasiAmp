#pragma once

// Standalone C++17 audio DSP. No JUCE dependency; compiled by tests on any OS.
// All buffer allocations happen in prepare(), never on the audio thread.

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

namespace garasi
{
struct Settings
{
    float inputDb = 0.0f;
    float gateThresholdDb = -58.0f;
    float boost = 0.0f;
    float drive = 4.0f;
    float bass = 5.0f;
    float mid = 5.0f;
    float treble = 5.0f;
    float presence = 5.0f;
    float outputDb = -9.0f;
    float delayMs = 320.0f;
    float delayFeedback = 0.27f;
    float delayMix = 0.15f;
    float reverbSize = 0.42f;
    float reverbMix = 0.15f;
    int ampMode = 1; // 0 Clean, 1 Crunch, 2 Lead
    int cabMode = 2; // 0 1x12, 1 2x12, 2 4x12
    bool gateOn = true;
    bool boostOn = false;
    bool cabOn = true;
    bool delayOn = false;
    bool reverbOn = true;
    bool bypass = false;
};

class AmpEngine
{
public:
    static constexpr int maxChannels = 2;

    void prepare(double sampleRate, int maxBlockSamples);
    void reset();
    void setSettings(const Settings& newSettings);
    float processSample(int channel, float sample) noexcept;

private:
    static constexpr float pi = 3.14159265358979323846f;
    static float dbToGain(float dB) noexcept;

    class OnePole
    {
    public:
        void setLowPass(double sampleRate, float frequency) noexcept;
        void clear() noexcept { z = 0.0f; }
        float lowPass(float in) noexcept { z += coefficient * (in - z); return z; }
        float highPass(float in) noexcept { return in - lowPass(in); }

    private:
        float coefficient = 1.0f;
        float z = 0.0f;
    };

    class Biquad
    {
    public:
        void lowShelf(double sr, float frequency, float gainDb) noexcept;
        void highShelf(double sr, float frequency, float gainDb) noexcept;
        void bell(double sr, float frequency, float q, float gainDb) noexcept;
        void clear() noexcept { z1 = z2 = 0.0f; }
        float process(float x) noexcept
        {
            const float y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return y;
        }

    private:
        void set(float nb0, float nb1, float nb2, float na0, float na1, float na2) noexcept;
        float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f;
        float z1 = 0.0f, z2 = 0.0f;
    };

    struct DelayLine
    {
        std::vector<float> buffer;
        std::size_t index = 0;
        void prepare(std::size_t length);
        void clear() noexcept;
        float process(float x, std::size_t delaySamples, float feedback) noexcept;
    };

    struct Comb
    {
        std::vector<float> buffer;
        std::size_t index = 0;
        float filterState = 0.0f;
        void prepare(std::size_t length);
        void clear() noexcept;
        float process(float x, float feedback, float damping) noexcept;
    };

    struct AllPass
    {
        std::vector<float> buffer;
        std::size_t index = 0;
        void prepare(std::size_t length);
        void clear() noexcept;
        float process(float x) noexcept;
    };

    struct Channel
    {
        OnePole inputHp, preEmphasis, cabHp, cabLp, cabLp2;
        Biquad bass, mid, treble, presence, cabMid;
        float gateEnvelope = 0.0f;
        float gateGain = 0.0f;
        DelayLine delay;
        std::array<Comb, 4> reverbs;
        std::array<AllPass, 2> diffusers;
        void clear() noexcept;
    };

    double sr = 48000.0;
    Settings settings;
    std::array<Channel, maxChannels> channels;
    float inGain = 1.0f;
    float outGain = 1.0f;
    float gateThreshold = 0.001f;
    float gateAttack = 0.04f;
    float gateRelease = 0.0008f;
    std::size_t delaySamples = 1;
    float preampGain = 1.0f;
    float saturationOutputGain = 1.0f;
};
} // namespace garasi
