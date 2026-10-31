#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <memory>
#include <vector>

class GarasiLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    GarasiLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float position, float startAngle, float endAngle,
                          juce::Slider&) override;
};

class ToneKnob final : public juce::Component
{
public:
    ToneKnob(juce::AudioProcessorValueTreeState&, const juce::String& parameterId,
             const juce::String& displayName);
    void resized() override;

private:
    juce::Slider slider;
    juce::Label caption;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

class ToneSwitch final : public juce::Component
{
public:
    ToneSwitch(juce::AudioProcessorValueTreeState&, const juce::String& parameterId,
               const juce::String& displayName);
    void resized() override;

private:
    juce::TextButton button;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
};

class ToneSelector final : public juce::Component
{
public:
    ToneSelector(juce::AudioProcessorValueTreeState&, const juce::String& parameterId,
                 const juce::String& displayName, const juce::StringArray& options);
    void resized() override;

private:
    juce::Label caption;
    juce::ComboBox box;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

class GarasiAmpAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit GarasiAmpAudioProcessorEditor(GarasiAmpAudioProcessor&);
    ~GarasiAmpAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GarasiAmpAudioProcessor& processor;
    GarasiLookAndFeel skin;

    std::vector<std::unique_ptr<ToneKnob>> knobs;
    std::vector<std::unique_ptr<ToneSwitch>> switches;
    std::vector<std::unique_ptr<ToneSelector>> selectors;
    juce::ComboBox factoryPreset;
    juce::TextButton saveButton { "SAVE" };
    juce::TextButton loadButton { "LOAD" };
    std::unique_ptr<juce::FileChooser> chooser;
    float smoothedPeak = 0.0f;
    std::array<juce::Rectangle<int>, 4> cards;
    juce::Rectangle<int> meterRect;

    ToneKnob& addKnob(const juce::String& id, const juce::String& label);
    ToneSwitch& addSwitch(const juce::String& id, const juce::String& label);
    ToneSelector& addSelector(const juce::String& id, const juce::String& label,
                              const juce::StringArray& options);
    void applyFactoryPreset(int presetId);
    void savePreset();
    void loadPreset();
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GarasiAmpAudioProcessorEditor)
};
