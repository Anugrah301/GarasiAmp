#include "../Source/AmpEngine.h"
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

int main()
{
    garasi::AmpEngine engine;
    garasi::Settings settings;
    engine.prepare(48000.0, 512);

    // Silence must stay silent (or close to numerical zero), including reverb state.
    engine.setSettings(settings);
    for (int n = 0; n < 20000; ++n)
    {
        const float y = engine.processSample(n % 2, 0.0f);
        assert(std::isfinite(y));
        assert(std::abs(y) < 1.0e-5f);
    }

    // A guitar-range sine input should produce nonzero, finite, controlled output.
    for (int mode = 0; mode < 3; ++mode)
    {
        settings.ampMode = mode;
        settings.cabMode = mode;
        settings.boostOn = mode > 0;
        settings.delayOn = mode == 2;
        settings.reverbOn = mode != 1;
        settings.gateOn = false;
        settings.drive = mode == 2 ? 9.0f : 4.0f;
        engine.setSettings(settings);
        float maxOut = 0.0f;
        for (int n = 0; n < 96000; ++n)
        {
            const float x = 0.1f * std::sin(2.0f * 3.14159265358979323846f *
                                              110.0f * static_cast<float>(n) / 48000.0f);
            const float y = engine.processSample(n % 2, x);
            assert(std::isfinite(y));
            assert(std::abs(y) <= 1.0f);
            maxOut = std::max(maxOut, std::abs(y));
        }
        assert(maxOut > 0.004f);
    }

    // Bypass must preserve the sample exactly (needed for A/B comparison).
    settings.bypass = true;
    engine.setSettings(settings);
    for (float input : std::array<float, 5> { -0.7f, -0.1f, 0.0f, 0.1f, 0.7f })
        assert(engine.processSample(0, input) == input);

    // Changing sample rates and extreme allowed tone controls must not blow up.
    for (double sr : { 44100.0, 48000.0, 96000.0 })
    {
        engine.prepare(sr, 2048);
        settings.bypass = false;
        settings.bass = 10.0f;
        settings.mid = 0.0f;
        settings.treble = 10.0f;
        settings.presence = 10.0f;
        settings.reverbSize = 1.0f;
        settings.outputDb = 6.0f;
        settings.drive = 10.0f;
        engine.setSettings(settings);
        for (int n = 0; n < 40000; ++n)
        {
            const float y = engine.processSample(0, (n % 16 == 0) ? 0.7f : 0.0f);
            assert(std::isfinite(y));
            assert(std::abs(y) <= 1.0f);
        }
    }
    std::cout << "PASS: DSP silence, tone modes, bypass, numerical stability (44.1/48/96 kHz)\n";
    return 0;
}
