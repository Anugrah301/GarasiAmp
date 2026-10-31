#include "AmpEngine.h"

namespace garasi
{
namespace
{
float clampFreq(double sampleRate, float hertz)
{
    return std::clamp(hertz, 20.0f, static_cast<float>(sampleRate * 0.45));
}

std::size_t millisecondsToSamples(double sampleRate, double milliseconds)
{
    return static_cast<std::size_t>(std::max(1.0, std::round(sampleRate * milliseconds / 1000.0)));
}
}

float AmpEngine::dbToGain(float dB) noexcept
{
    return std::pow(10.0f, dB / 20.0f);
}

void AmpEngine::OnePole::setLowPass(double sampleRate, float frequency) noexcept
{
    const auto f = clampFreq(sampleRate, frequency);
    coefficient = 1.0f - std::exp(-2.0f * pi * f / static_cast<float>(sampleRate));
}

void AmpEngine::Biquad::set(float nb0, float nb1, float nb2,
                             float na0, float na1, float na2) noexcept
{
    const float inverse = 1.0f / na0;
    b0 = nb0 * inverse;
    b1 = nb1 * inverse;
    b2 = nb2 * inverse;
    a1 = na1 * inverse;
    a2 = na2 * inverse;
}

void AmpEngine::Biquad::bell(double sampleRate, float frequency, float q, float gainDb) noexcept
{
    const float a = std::pow(10.0f, gainDb / 40.0f);
    const float w = 2.0f * pi * clampFreq(sampleRate, frequency) / static_cast<float>(sampleRate);
    const float co = std::cos(w);
    const float alpha = std::sin(w) / (2.0f * q);
    set(1.0f + alpha * a, -2.0f * co, 1.0f - alpha * a,
        1.0f + alpha / a, -2.0f * co, 1.0f - alpha / a);
}

void AmpEngine::Biquad::lowShelf(double sampleRate, float frequency, float gainDb) noexcept
{
    const float a = std::pow(10.0f, gainDb / 40.0f);
    const float w = 2.0f * pi * clampFreq(sampleRate, frequency) / static_cast<float>(sampleRate);
    const float co = std::cos(w);
    const float root = 2.0f * std::sqrt(a) * (std::sin(w) / std::sqrt(2.0f));
    set(a * ((a + 1.0f) - (a - 1.0f) * co + root),
        2.0f * a * ((a - 1.0f) - (a + 1.0f) * co),
        a * ((a + 1.0f) - (a - 1.0f) * co - root),
        (a + 1.0f) + (a - 1.0f) * co + root,
        -2.0f * ((a - 1.0f) + (a + 1.0f) * co),
        (a + 1.0f) + (a - 1.0f) * co - root);
}

void AmpEngine::Biquad::highShelf(double sampleRate, float frequency, float gainDb) noexcept
{
    const float a = std::pow(10.0f, gainDb / 40.0f);
    const float w = 2.0f * pi * clampFreq(sampleRate, frequency) / static_cast<float>(sampleRate);
    const float co = std::cos(w);
    const float root = 2.0f * std::sqrt(a) * (std::sin(w) / std::sqrt(2.0f));
    set(a * ((a + 1.0f) + (a - 1.0f) * co + root),
        -2.0f * a * ((a - 1.0f) + (a + 1.0f) * co),
        a * ((a + 1.0f) + (a - 1.0f) * co - root),
        (a + 1.0f) - (a - 1.0f) * co + root,
        2.0f * ((a - 1.0f) - (a + 1.0f) * co),
        (a + 1.0f) - (a - 1.0f) * co - root);
}

void AmpEngine::DelayLine::prepare(std::size_t length)
{
    buffer.assign(std::max<std::size_t>(length, 2), 0.0f);
    index = 0;
}

void AmpEngine::DelayLine::clear() noexcept
{
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    index = 0;
}

float AmpEngine::DelayLine::process(float x, std::size_t distance, float feedback) noexcept
{
    if (buffer.empty())
        return 0.0f;

    distance = std::clamp<std::size_t>(distance, 1, buffer.size() - 1);
    const auto read = (index + buffer.size() - distance) % buffer.size();
    const float delayed = buffer[read];
    buffer[index] = std::clamp(x + delayed * feedback, -4.0f, 4.0f);
    index = (index + 1) % buffer.size();
    return delayed;
}

void AmpEngine::Comb::prepare(std::size_t length)
{
    buffer.assign(std::max<std::size_t>(length, 2), 0.0f);
    index = 0;
    filterState = 0.0f;
}

void AmpEngine::Comb::clear() noexcept
{
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    index = 0;
    filterState = 0.0f;
}

float AmpEngine::Comb::process(float x, float feedback, float damping) noexcept
{
    const float delayed = buffer[index];
    filterState = delayed * (1.0f - damping) + filterState * damping;
    buffer[index] = std::clamp(x + filterState * feedback, -4.0f, 4.0f);
    index = (index + 1) % buffer.size();
    return delayed;
}

void AmpEngine::AllPass::prepare(std::size_t length)
{
    buffer.assign(std::max<std::size_t>(length, 2), 0.0f);
    index = 0;
}

void AmpEngine::AllPass::clear() noexcept
{
    std::fill(buffer.begin(), buffer.end(), 0.0f);
    index = 0;
}

float AmpEngine::AllPass::process(float x) noexcept
{
    constexpr float feedback = 0.5f;
    const float delayed = buffer[index];
    const float result = delayed - feedback * x;
    buffer[index] = x + feedback * delayed;
    index = (index + 1) % buffer.size();
    return result;
}

void AmpEngine::Channel::clear() noexcept
{
    inputHp.clear();
    preEmphasis.clear();
    cabHp.clear();
    cabLp.clear();
    cabLp2.clear();
    bass.clear();
    mid.clear();
    treble.clear();
    presence.clear();
    cabMid.clear();
    gateEnvelope = 0.0f;
    gateGain = 0.0f;
    delay.clear();
    for (auto& comb : reverbs) comb.clear();
    for (auto& allPass : diffusers) allPass.clear();
}

void AmpEngine::prepare(double sampleRate, int)
{
    sr = std::max(8000.0, sampleRate);
    constexpr std::array<double, 4> combTimesMs { 29.7, 37.1, 41.1, 43.7 };
    constexpr std::array<double, 2> allPassTimesMs { 5.0, 1.7 };

    for (int c = 0; c < maxChannels; ++c)
    {
        auto& channel = channels[static_cast<std::size_t>(c)];
        channel.delay.prepare(millisecondsToSamples(sr, 1200.0) + 1);
        for (std::size_t i = 0; i < combTimesMs.size(); ++i)
            channel.reverbs[i].prepare(millisecondsToSamples(sr, combTimesMs[i] + c * 0.55));
        for (std::size_t i = 0; i < allPassTimesMs.size(); ++i)
            channel.diffusers[i].prepare(millisecondsToSamples(sr, allPassTimesMs[i] + c * 0.35));
    }
    reset();
    setSettings(settings);
}

void AmpEngine::reset()
{
    for (auto& channel : channels)
        channel.clear();
}

void AmpEngine::setSettings(const Settings& newSettings)
{
    settings = newSettings;
    settings.ampMode = std::clamp(settings.ampMode, 0, 2);
    settings.cabMode = std::clamp(settings.cabMode, 0, 2);
    inGain = dbToGain(std::clamp(settings.inputDb, -18.0f, 18.0f));
    outGain = dbToGain(std::clamp(settings.outputDb, -30.0f, 6.0f));
    gateThreshold = dbToGain(std::clamp(settings.gateThresholdDb, -85.0f, -20.0f));
    gateAttack = 1.0f - std::exp(-1.0f / (0.003f * static_cast<float>(sr)));
    gateRelease = 1.0f - std::exp(-1.0f / (0.080f * static_cast<float>(sr)));
    delaySamples = millisecondsToSamples(sr, std::clamp(settings.delayMs, 70.0f, 900.0f));
    const float driveFactor = 1.0f + std::pow(std::clamp(settings.drive, 0.0f, 10.0f), 1.35f) * 0.42f;
    constexpr std::array<float, 3> modeGain { 0.75f, 1.45f, 2.1f };
    preampGain = driveFactor * modeGain[static_cast<std::size_t>(settings.ampMode)];
    saturationOutputGain = 0.85f / std::sqrt(1.0f + 0.08f * driveFactor);

    constexpr std::array<float, 3> cabHighPass { 75.0f, 90.0f, 110.0f };
    constexpr std::array<float, 3> cabLowPass { 6500.0f, 5700.0f, 4900.0f };
    constexpr std::array<float, 3> cabMidHz { 1400.0f, 1650.0f, 1900.0f };
    for (auto& channel : channels)
    {
        channel.inputHp.setLowPass(sr, 32.0f);
        channel.preEmphasis.setLowPass(sr, 1100.0f);
        channel.bass.lowShelf(sr, 140.0f, (settings.bass - 5.0f) * 2.4f);
        channel.mid.bell(sr, 850.0f, 0.8f, (settings.mid - 5.0f) * 2.3f);
        channel.treble.highShelf(sr, 3400.0f, (settings.treble - 5.0f) * 2.2f);
        channel.presence.highShelf(sr, 2400.0f, (settings.presence - 5.0f) * 1.6f);
        channel.cabHp.setLowPass(sr, cabHighPass[static_cast<std::size_t>(settings.cabMode)]);
        channel.cabLp.setLowPass(sr, cabLowPass[static_cast<std::size_t>(settings.cabMode)]);
        channel.cabLp2.setLowPass(sr, cabLowPass[static_cast<std::size_t>(settings.cabMode)]);
        channel.cabMid.bell(sr, cabMidHz[static_cast<std::size_t>(settings.cabMode)],
                            0.95f, settings.cabMode == 2 ? 2.5f : 1.5f);
    }
}

float AmpEngine::processSample(int channelIndex, float sample) noexcept
{
    if (channelIndex < 0 || channelIndex >= maxChannels)
        return sample;
    if (!std::isfinite(sample))
        return 0.0f;
    if (settings.bypass)
        return sample;

    auto& channel = channels[static_cast<std::size_t>(channelIndex)];
    const float input = std::clamp(sample, -8.0f, 8.0f);
    float x = channel.inputHp.highPass(input * inGain);

    const float magnitude = std::abs(x);
    const float envelopeRate = magnitude > channel.gateEnvelope ? gateAttack : gateRelease;
    channel.gateEnvelope += envelopeRate * (magnitude - channel.gateEnvelope);
    const float gateTarget = std::clamp(
        (channel.gateEnvelope - gateThreshold * 0.5f) / (gateThreshold * 1.5f), 0.0f, 1.0f);
    channel.gateGain += (gateTarget - channel.gateGain) *
                        (gateTarget > channel.gateGain ? 0.075f : 0.0025f);
    if (settings.gateOn)
        x *= channel.gateGain;

    if (settings.boostOn)
        x *= 1.0f + std::clamp(settings.boost, 0.0f, 10.0f) * 0.36f;

    const float low = channel.preEmphasis.lowPass(x);
    x += (x - low) * (settings.ampMode == 0 ? 0.05f : 0.30f);

    const float asymmetry = x >= 0.0f ? 1.06f : 0.94f;
    const float first = std::tanh(x * preampGain * asymmetry);
    float amplified = std::tanh(first * (settings.ampMode == 2 ? 1.60f : 1.18f));
    amplified *= saturationOutputGain;

    x = channel.bass.process(amplified);
    x = channel.mid.process(x);
    x = channel.treble.process(x);
    x = channel.presence.process(x);

    if (settings.cabOn)
    {
        x = channel.cabHp.highPass(x);
        x = channel.cabMid.process(x);
        x = channel.cabLp.lowPass(x);
        x = channel.cabLp2.lowPass(x);
    }

    const float dryFx = x;
    const auto feedback = std::clamp(settings.delayFeedback, 0.0f, 0.75f);
    const float delayed = channel.delay.process(settings.delayOn ? dryFx : 0.0f,
                                                 delaySamples, feedback);
    if (settings.delayOn)
        x += delayed * std::clamp(settings.delayMix, 0.0f, 0.60f);

    const float roomInput = settings.reverbOn ? x * 0.20f : 0.0f;
    float reverb = 0.0f;
    const float roomFeedback = 0.72f + std::clamp(settings.reverbSize, 0.0f, 1.0f) * 0.21f;
    for (auto& comb : channel.reverbs)
        reverb += comb.process(roomInput, roomFeedback, 0.30f);
    reverb *= 0.25f;
    for (auto& diffuser : channel.diffusers)
        reverb = diffuser.process(reverb);
    if (settings.reverbOn)
        x += reverb * std::clamp(settings.reverbMix, 0.0f, 0.60f) * 1.3f;

    // Soft protection for accidental level spikes; no look-ahead brickwall limiting.
    const float output = x * outGain;
    return std::tanh(output * 1.1f) / 1.1f;
}
} // namespace garasi
