#pragma once

#include <JuceHeader.h>
#include "AmpEngine.h"
#include <atomic>

class GarasiAmpAudioProcessor final : public juce::AudioProcessor
{
public:
    GarasiAmpAudioProcessor();
    ~GarasiAmpAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Garasi Amp"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Current Tone"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> outputPeak { 0.0f };

private:
    garasi::AmpEngine dsp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GarasiAmpAudioProcessor)
};
