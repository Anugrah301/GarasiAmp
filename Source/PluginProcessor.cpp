#include "PluginProcessor.h"
#include "PluginEditor.h"

GarasiAmpAudioProcessor::GarasiAmpAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Guitar In", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Amp Out", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "GarasiAmpParameters", createParameters())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout GarasiAmpAudioProcessor::createParameters()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    const auto addFloat = [&layout](const char* id, const char* name,
                                    float low, float high, float initial, float step)
    {
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float> { low, high, step }, initial));
    };
    const auto addBool = [&layout](const char* id, const char* name, bool initial)
    {
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { id, 1 }, name, initial));
    };

    addFloat("inputDb", "Input gain (dB)", -18.0f, 18.0f, 0.0f, 0.1f);
    addFloat("gateThreshold", "Gate threshold (dB)", -85.0f, -20.0f, -58.0f, 1.0f);
    addFloat("boost", "Stomp boost", 0.0f, 10.0f, 0.0f, 0.1f);
    addFloat("drive", "Amp drive", 0.0f, 10.0f, 4.0f, 0.1f);
    addFloat("bass", "Bass", 0.0f, 10.0f, 5.0f, 0.1f);
    addFloat("mid", "Mid", 0.0f, 10.0f, 5.0f, 0.1f);
    addFloat("treble", "Treble", 0.0f, 10.0f, 5.0f, 0.1f);
    addFloat("presence", "Presence", 0.0f, 10.0f, 5.0f, 0.1f);
    addFloat("outputDb", "Master (dB)", -30.0f, 6.0f, -9.0f, 0.1f);
    addFloat("delayMs", "Delay time (ms)", 70.0f, 900.0f, 320.0f, 1.0f);
    addFloat("delayFeedback", "Delay feedback", 0.0f, 0.75f, 0.27f, 0.01f);
    addFloat("delayMix", "Delay mix", 0.0f, 0.60f, 0.15f, 0.01f);
    addFloat("reverbSize", "Reverb size", 0.0f, 1.0f, 0.42f, 0.01f);
    addFloat("reverbMix", "Reverb mix", 0.0f, 0.60f, 0.15f, 0.01f);

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "ampMode", 1 }, "Amp model",
        juce::StringArray { "CLEAN", "CRUNCH", "LEAD" }, 1));
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID { "cabMode", 1 }, "Cab model",
        juce::StringArray { "1x12 OPEN", "2x12 VINTAGE", "4x12 MODERN" }, 2));

    addBool("gateOn", "Gate on", true);
    addBool("boostOn", "Boost on", false);
    addBool("cabOn", "Cab on", true);
    addBool("delayOn", "Delay on", false);
    addBool("reverbOn", "Reverb on", true);
    addBool("bypass", "Bypass", false);
    return layout;
}

void GarasiAmpAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dsp.prepare(sampleRate, samplesPerBlock);
    outputPeak.store(0.0f, std::memory_order_relaxed);
}

void GarasiAmpAudioProcessor::releaseResources() {}

bool GarasiAmpAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    return (input == juce::AudioChannelSet::mono() ||
            input == juce::AudioChannelSet::stereo()) && input == output;
}

void GarasiAmpAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals guard;
    garasi::Settings s;
    const auto v = [this](const char* key) -> float
    {
        return parameters.getRawParameterValue(key)->load(std::memory_order_relaxed);
    };
    s.inputDb = v("inputDb");
    s.gateThresholdDb = v("gateThreshold");
    s.boost = v("boost");
    s.drive = v("drive");
    s.bass = v("bass");
    s.mid = v("mid");
    s.treble = v("treble");
    s.presence = v("presence");
    s.outputDb = v("outputDb");
    s.delayMs = v("delayMs");
    s.delayFeedback = v("delayFeedback");
    s.delayMix = v("delayMix");
    s.reverbSize = v("reverbSize");
    s.reverbMix = v("reverbMix");
    s.ampMode = static_cast<int>(v("ampMode"));
    s.cabMode = static_cast<int>(v("cabMode"));
    s.gateOn = v("gateOn") > 0.5f;
    s.boostOn = v("boostOn") > 0.5f;
    s.cabOn = v("cabOn") > 0.5f;
    s.delayOn = v("delayOn") > 0.5f;
    s.reverbOn = v("reverbOn") > 0.5f;
    s.bypass = v("bypass") > 0.5f;
    dsp.setSettings(s);

    float peak = 0.0f;
    const int channels = std::min(buffer.getNumChannels(), garasi::AmpEngine::maxChannels);
    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            data[sample] = dsp.processSample(ch, data[sample]);
            peak = std::max(peak, std::abs(data[sample]));
        }
    }
    for (int ch = channels; ch < buffer.getNumChannels(); ++ch)
        buffer.clear(ch, 0, buffer.getNumSamples());
    outputPeak.store(peak, std::memory_order_relaxed);
}

void GarasiAmpAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters.copyState().createXml())
        juce::copyXmlToBinary(*xml, destination);
}

void GarasiAmpAudioProcessor::setStateInformation(const void* data, int byteCount)
{
    if (auto xml = juce::getXmlFromBinary(data, byteCount))
    {
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessorEditor* GarasiAmpAudioProcessor::createEditor()
{
    return new GarasiAmpAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GarasiAmpAudioProcessor();
}
