#include "PluginEditor.h"
#include <cmath>

namespace
{
const auto bg = juce::Colour::fromRGB(12, 16, 24);
const auto cardBg = juce::Colour::fromRGB(24, 30, 40);
const auto muted = juce::Colour::fromRGB(135, 150, 170);
const auto text = juce::Colour::fromRGB(233, 239, 248);
const auto amber = juce::Colour::fromRGB(244, 166, 61);
const auto line = juce::Colour::fromRGB(57, 66, 81);

void drawPanel(juce::Graphics& g, juce::Rectangle<int> rect, const juce::String& title,
               const juce::String& subtitle, int order)
{
    auto r = rect.toFloat();
    g.setColour(cardBg);
    g.fillRoundedRectangle(r, 16.0f);
    g.setColour(line);
    g.drawRoundedRectangle(r.reduced(0.5f), 16.0f, 1.0f);
    g.setColour(amber);
    g.fillRoundedRectangle({ r.getX() + 18.0f, r.getY() + 23.0f, 5.0f, 25.0f }, 2.5f);
    g.setColour(text);
    g.setFont(juce::Font(19.0f, juce::Font::bold));
    g.drawText(title, rect.getX() + 33, rect.getY() + 17, rect.getWidth() - 55, 26,
               juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::Font(11.0f));
    g.drawText(subtitle, rect.getX() + 20, rect.getY() + 54, rect.getWidth() - 35, 20,
               juce::Justification::centredLeft);
    g.setColour(line.brighter(0.2f));
    g.drawText(juce::String(order), rect.getRight() - 39, rect.getY() + 19, 18, 24,
               juce::Justification::centredRight);
}
} // namespace

GarasiLookAndFeel::GarasiLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(36, 44, 58));
    setColour(juce::ComboBox::outlineColourId, line);
    setColour(juce::ComboBox::textColourId, text);
    setColour(juce::ComboBox::arrowColourId, amber);
    setColour(juce::PopupMenu::backgroundColourId, cardBg);
    setColour(juce::PopupMenu::textColourId, text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, amber);
    setColour(juce::PopupMenu::highlightedTextColourId, bg);
    setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(42, 50, 65));
    setColour(juce::TextButton::buttonOnColourId, amber);
    setColour(juce::TextButton::textColourOffId, text);
    setColour(juce::TextButton::textColourOnId, bg);
}

void GarasiLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float position, float startAngle, float endAngle,
                                        juce::Slider&)
{
    const float size = static_cast<float>(std::min(width, height)) - 18.0f;
    const float radius = std::max(8.0f, size * 0.5f);
    const auto centreX = static_cast<float>(x + width / 2);
    const auto centreY = static_cast<float>(y + height / 2);
    const float angle = startAngle + position * (endAngle - startAngle);
    juce::Path track;
    track.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                        startAngle, endAngle, true);
    g.setColour(line);
    g.strokePath(track, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
    juce::Path progress;
    progress.addCentredArc(centreX, centreY, radius, radius, 0.0f,
                           startAngle, angle, true);
    g.setColour(amber);
    g.strokePath(progress, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    g.setColour(juce::Colour::fromRGB(45, 54, 68));
    g.fillEllipse(centreX - radius * 0.76f, centreY - radius * 0.76f,
                  radius * 1.52f, radius * 1.52f);
    g.setColour(juce::Colour::fromRGB(88, 96, 109));
    g.drawEllipse(centreX - radius * 0.76f, centreY - radius * 0.76f,
                   radius * 1.52f, radius * 1.52f, 1.0f);
    const float markerLength = radius * 0.42f;
    const float dx = std::sin(angle) * markerLength;
    const float dy = -std::cos(angle) * markerLength;
    g.setColour(text);
    g.drawLine(centreX, centreY, centreX + dx, centreY + dy, 3.0f);
}

ToneKnob::ToneKnob(juce::AudioProcessorValueTreeState& state,
                   const juce::String& id, const juce::String& name)
{
    caption.setText(name.toUpperCase(), juce::dontSendNotification);
    caption.setJustificationType(juce::Justification::centred);
    caption.setColour(juce::Label::textColourId, muted);
    caption.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(caption);

    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 75, 22);
    slider.setColour(juce::Slider::textBoxTextColourId, text);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, cardBg);
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(slider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        state, id, slider);
}

void ToneKnob::resized()
{
    auto bounds = getLocalBounds();
    caption.setBounds(bounds.removeFromTop(20));
    slider.setBounds(bounds);
}

ToneSwitch::ToneSwitch(juce::AudioProcessorValueTreeState& state,
                       const juce::String& id, const juce::String& name)
{
    button.setButtonText(name);
    button.setClickingTogglesState(true);
    addAndMakeVisible(button);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        state, id, button);
}

void ToneSwitch::resized() { button.setBounds(getLocalBounds()); }

ToneSelector::ToneSelector(juce::AudioProcessorValueTreeState& state,
                           const juce::String& id, const juce::String& name,
                           const juce::StringArray& options)
{
    caption.setText(name.toUpperCase(), juce::dontSendNotification);
    caption.setJustificationType(juce::Justification::centredLeft);
    caption.setColour(juce::Label::textColourId, muted);
    caption.setFont(juce::Font(12.0f, juce::Font::bold));
    addAndMakeVisible(caption);
    for (int i = 0; i < options.size(); ++i)
        box.addItem(options[i], i + 1);
    addAndMakeVisible(box);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        state, id, box);
}

void ToneSelector::resized()
{
    auto bounds = getLocalBounds();
    caption.setBounds(bounds.removeFromTop(22));
    box.setBounds(bounds.removeFromTop(34));
}

ToneKnob& GarasiAmpAudioProcessorEditor::addKnob(const juce::String& id,
                                                 const juce::String& label)
{
    auto control = std::make_unique<ToneKnob>(processor.parameters, id, label);
    auto& reference = *control;
    addAndMakeVisible(reference);
    knobs.push_back(std::move(control));
    return reference;
}

ToneSwitch& GarasiAmpAudioProcessorEditor::addSwitch(const juce::String& id,
                                                     const juce::String& label)
{
    auto control = std::make_unique<ToneSwitch>(processor.parameters, id, label);
    auto& reference = *control;
    addAndMakeVisible(reference);
    switches.push_back(std::move(control));
    return reference;
}

ToneSelector& GarasiAmpAudioProcessorEditor::addSelector(const juce::String& id,
                                                         const juce::String& label,
                                                         const juce::StringArray& choices)
{
    auto control = std::make_unique<ToneSelector>(processor.parameters, id, label, choices);
    auto& reference = *control;
    addAndMakeVisible(reference);
    selectors.push_back(std::move(control));
    return reference;
}

GarasiAmpAudioProcessorEditor::GarasiAmpAudioProcessorEditor(GarasiAmpAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&skin);
    // Order is referenced in resized(): keep in sync with the groups below.
    addKnob("inputDb", "INPUT dB");              // knobs[0]
    addKnob("gateThreshold", "GATE dB");        // knobs[1]
    addKnob("boost", "BOOST");                  // knobs[2]
    addKnob("drive", "DRIVE");                  // knobs[3]
    addKnob("bass", "BASS");                    // knobs[4]
    addKnob("mid", "MID");                      // knobs[5]
    addKnob("treble", "TREBLE");                // knobs[6]
    addKnob("presence", "PRESENCE");            // knobs[7]
    addKnob("outputDb", "MASTER dB");           // knobs[8]
    addKnob("delayMs", "TIME ms");              // knobs[9]
    addKnob("delayFeedback", "FEEDBACK");       // knobs[10]
    addKnob("delayMix", "DELAY MIX");           // knobs[11]
    addKnob("reverbSize", "ROOM SIZE");         // knobs[12]
    addKnob("reverbMix", "REVERB MIX");         // knobs[13]

    addSwitch("gateOn", "NOISE GATE");          // switches[0]
    addSwitch("boostOn", "STOMP BOOST");        // switches[1]
    addSwitch("cabOn", "CAB ENABLED");         // switches[2]
    addSwitch("delayOn", "DELAY ON");           // switches[3]
    addSwitch("reverbOn", "REVERB ON");         // switches[4]
    addSwitch("bypass", "BYPASS ALL");          // switches[5]
    addSelector("ampMode", "Amplifier", { "CLEAN", "CRUNCH", "LEAD" }); // selectors[0]
    addSelector("cabMode", "Speaker cabinet", { "1x12 OPEN", "2x12 VINTAGE", "4x12 MODERN" }); // selectors[1]

    factoryPreset.addItem("CUSTOM / CURRENT", 1);
    factoryPreset.addItem("01  GLASS CLEAN", 2);
    factoryPreset.addItem("02  BLUES CRUNCH", 3);
    factoryPreset.addItem("03  CLASSIC ROCK", 4);
    factoryPreset.addItem("04  MODERN LEAD", 5);
    factoryPreset.setSelectedId(1, juce::dontSendNotification);
    factoryPreset.onChange = [this] { applyFactoryPreset(factoryPreset.getSelectedId()); };
    addAndMakeVisible(factoryPreset);

    saveButton.onClick = [this] { savePreset(); };
    loadButton.onClick = [this] { loadPreset(); };
    addAndMakeVisible(saveButton);
    addAndMakeVisible(loadButton);
    setSize(1160, 640);
    setResizable(true, true);
    setResizeLimits(1050, 600, 1600, 800);
    startTimerHz(25);
}

GarasiAmpAudioProcessorEditor::~GarasiAmpAudioProcessorEditor()
{
    stopTimer();
    chooser.reset();
    setLookAndFeel(nullptr);
}

void GarasiAmpAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    const auto bounds = getLocalBounds();
    g.setColour(juce::Colour::fromRGB(18, 24, 34));
    g.fillRect(bounds.withHeight(90));

    g.setFont(juce::Font(31.0f, juce::Font::bold));
    g.setColour(text);
    g.drawText("GARASI", 26, 20, 138, 42, juce::Justification::centredLeft);
    g.setColour(amber);
    g.drawText("AMP", 167, 20, 92, 42, juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::Font(11.5f));
    g.drawText("GUITAR TONE SYSTEM  /  VST3", 28, 62, 260, 17,
               juce::Justification::centredLeft);

    g.setColour(line);
    g.drawHorizontalLine(89, 0.0f, static_cast<float>(getWidth()));

    // The signal chain uses text rather than fake 3D hardware graphics.
    auto chain = bounds.reduced(26).withTrimmedTop(94).withHeight(32);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    const juce::StringArray stages { "GUITAR IN", "GATE / BOOST", "AMP + EQ", "CAB", "DELAY / REVERB", "OUTPUT" };
    const int stageWidth = chain.getWidth() / stages.size();
    for (int i = 0; i < stages.size(); ++i)
    {
        auto r = chain.withX(chain.getX() + i * stageWidth).withWidth(stageWidth);
        g.setColour(i == 2 ? amber : muted);
        g.drawText(stages[i], r, juce::Justification::centred);
        if (i < stages.size() - 1)
        {
            g.setColour(amber.withAlpha(0.7f));
            const int px = r.getRight() - 2;
            g.drawLine(static_cast<float>(px - 7), static_cast<float>(r.getCentreY()),
                       static_cast<float>(px + 5), static_cast<float>(r.getCentreY()), 1.6f);
        }
    }

    const juce::StringArray titles { "PREAMP", "AMPLIFIER", "CABINET", "SPACE FX" };
    const juce::StringArray subtitles {
        "TAME NOISE. PUSH HARDER.", "THE SOUND STARTS HERE.",
        "SPEAKER VOICING.", "ADD DEPTH & ATMOSPHERE."
    };
    for (int i = 0; i < 4; ++i)
        drawPanel(g, cards[static_cast<std::size_t>(i)], titles[i], subtitles[i], i + 1);

    // Output meter. UI reads peak atomically, audio thread never paints.
    g.setColour(muted);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    auto labelRect = meterRect.withHeight(20).translated(0, -26);
    g.drawText("OUTPUT LEVEL", labelRect, juce::Justification::centredLeft);
    const auto meter = meterRect.toFloat();
    g.setColour(bg);
    g.fillRoundedRectangle(meter, 5.0f);
    const float filled = meter.getWidth() * juce::jlimit(0.0f, 1.0f, smoothedPeak);
    g.setColour(smoothedPeak > 0.90f ? juce::Colours::orangered : amber);
    g.fillRoundedRectangle({ meter.getX(), meter.getY(), filled, meter.getHeight() }, 5.0f);
    g.setColour(line);
    g.drawRoundedRectangle(meter, 5.0f, 1.0f);

    g.setColour(muted);
    g.setFont(juce::Font(11.0f));
    g.drawText("GARASI AUDIO  /  0.1.0  /  CAB = FILTER MODEL (NO IR YET)",
               29, getHeight() - 30, getWidth() - 300, 18,
               juce::Justification::centredLeft);
}

void GarasiAmpAudioProcessorEditor::resized()
{
    const int total = getWidth();
    const int pad = 24;
    const int gap = 14;
    const int left = pad;
    const int span = total - 2 * pad - 3 * gap;
    const int w1 = static_cast<int>(span * 0.21f);
    const int w2 = static_cast<int>(span * 0.315f);
    const int w3 = static_cast<int>(span * 0.195f);
    const int w4 = span - w1 - w2 - w3;
    const int top = 138;
    const int cardHeight = getHeight() - top - 48;
    cards[0] = { left, top, w1, cardHeight };
    cards[1] = { left + w1 + gap, top, w2, cardHeight };
    cards[2] = { left + w1 + gap + w2 + gap, top, w3, cardHeight };
    cards[3] = { left + w1 + gap + w2 + gap + w3 + gap, top, w4, cardHeight };

    const int presetWidth = juce::jlimit(200, 320, total / 4);
    const int controlX = total - 24 - presetWidth - 144;
    factoryPreset.setBounds(controlX, 26, presetWidth, 36);
    saveButton.setBounds(total - 154, 26, 61, 36);
    loadButton.setBounds(total - 86, 26, 61, 36);

    // PREAMP
    const auto a = cards[0].reduced(15);
    const int aKnobW = (a.getWidth() - 10) / 2;
    knobs[0]->setBounds(a.getX(), a.getY() + 90, aKnobW, 120);
    knobs[1]->setBounds(a.getX() + aKnobW + 10, a.getY() + 90, aKnobW, 120);
    switches[0]->setBounds(a.getX() + 1, a.getY() + 220, a.getWidth() - 2, 32);
    switches[1]->setBounds(a.getX() + 1, a.getY() + 262, a.getWidth() - 2, 32);
    knobs[2]->setBounds(a.getX() + (a.getWidth() - 112) / 2, a.getY() + 305, 112, 120);

    // AMPLIFIER
    const auto b = cards[1].reduced(15);
    selectors[0]->setBounds(b.getX() + 6, b.getY() + 83, b.getWidth() - 12, 60);
    const int bWidth = b.getWidth() / 3;
    for (int i = 0; i < 3; ++i)
        knobs[static_cast<std::size_t>(3 + i)]->setBounds(b.getX() + i * bWidth,
                                                          b.getY() + 160, bWidth, 115);
    for (int i = 0; i < 3; ++i)
        knobs[static_cast<std::size_t>(6 + i)]->setBounds(b.getX() + i * bWidth,
                                                          b.getY() + 288, bWidth, 115);

    // CABINET
    const auto c = cards[2].reduced(15);
    selectors[1]->setBounds(c.getX() + 3, c.getY() + 100, c.getWidth() - 6, 60);
    switches[2]->setBounds(c.getX() + 2, c.getY() + 191, c.getWidth() - 4, 36);
    meterRect = { c.getX() + 4, c.getY() + 333, c.getWidth() - 8, 18 };

    // FX
    const auto d = cards[3].reduced(15);
    const int dWidth = d.getWidth() / 3;
    for (int i = 0; i < 3; ++i)
        knobs[static_cast<std::size_t>(9 + i)]->setBounds(d.getX() + i * dWidth,
                                                          d.getY() + 92, dWidth, 115);
    switches[3]->setBounds(d.getX() + 5, d.getY() + 212, d.getWidth() - 10, 32);
    const int reverbWidth = d.getWidth() / 2;
    knobs[12]->setBounds(d.getX(), d.getY() + 265, reverbWidth, 120);
    knobs[13]->setBounds(d.getX() + reverbWidth, d.getY() + 265, reverbWidth, 120);
    switches[4]->setBounds(d.getX() + 5, d.getY() + 390, d.getWidth() - 10, 31);

    switches[5]->setBounds(total - 177, getHeight() - 37, 150, 27);
    repaint();
}

void GarasiAmpAudioProcessorEditor::applyFactoryPreset(int preset)
{
    if (preset <= 1)
        return;

    const auto setParam = [this](const char* id, float value)
    {
        if (auto* parameter = processor.parameters.getParameter(id))
        {
            parameter->beginChangeGesture();
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
            parameter->endChangeGesture();
        }
    };

    // Start with a deterministic baseline so that preset changes never inherit
    // hidden settings from the previously selected sound.
    struct Value { const char* name; float value; };
    constexpr Value defaults[] {
        { "inputDb", 0.0f }, { "gateThreshold", -58.0f }, { "boost", 0.0f },
        { "drive", 4.0f }, { "bass", 5.0f }, { "mid", 5.0f },
        { "treble", 5.0f }, { "presence", 5.0f }, { "outputDb", -9.0f },
        { "delayMs", 320.0f }, { "delayFeedback", 0.27f },
        { "delayMix", 0.15f }, { "reverbSize", 0.42f },
        { "reverbMix", 0.15f }, { "ampMode", 1.0f }, { "cabMode", 2.0f },
        { "gateOn", 1.0f }, { "boostOn", 0.0f }, { "cabOn", 1.0f },
        { "delayOn", 0.0f }, { "reverbOn", 1.0f }, { "bypass", 0.0f }
    };
    for (const auto& entry : defaults)
        setParam(entry.name, entry.value);

    // All supplied values are in the physical units of the corresponding parameters.
    switch (preset)
    {
        case 2: // GLASS CLEAN
            setParam("ampMode", 0); setParam("cabMode", 0);
            setParam("drive", 1.9f); setParam("bass", 5.5f);
            setParam("mid", 4.4f); setParam("treble", 7.1f);
            setParam("presence", 6.3f); setParam("outputDb", -5.0f);
            setParam("boostOn", 0); setParam("delayOn", 1);
            setParam("delayMs", 260); setParam("delayMix", 0.10f);
            setParam("reverbOn", 1); setParam("reverbMix", 0.22f);
            break;
        case 3: // BLUES CRUNCH
            setParam("ampMode", 1); setParam("cabMode", 1);
            setParam("drive", 3.7f); setParam("bass", 5.3f);
            setParam("mid", 7.1f); setParam("treble", 5.7f);
            setParam("presence", 5.5f); setParam("outputDb", -9.0f);
            setParam("boostOn", 0); setParam("delayOn", 0);
            setParam("reverbOn", 1); setParam("reverbMix", 0.13f);
            break;
        case 4: // CLASSIC ROCK
            setParam("ampMode", 1); setParam("cabMode", 2);
            setParam("drive", 6.4f); setParam("bass", 5.8f);
            setParam("mid", 6.5f); setParam("treble", 6.2f);
            setParam("presence", 5.8f); setParam("outputDb", -12.0f);
            setParam("boostOn", 1); setParam("boost", 2.2f);
            setParam("delayOn", 0); setParam("reverbOn", 1);
            setParam("reverbMix", 0.09f);
            break;
        case 5: // MODERN LEAD
            setParam("ampMode", 2); setParam("cabMode", 2);
            setParam("drive", 8.1f); setParam("bass", 6.1f);
            setParam("mid", 4.8f); setParam("treble", 6.7f);
            setParam("presence", 6.7f); setParam("outputDb", -15.0f);
            setParam("gateOn", 1); setParam("gateThreshold", -52.0f);
            setParam("boostOn", 1); setParam("boost", 2.4f);
            setParam("delayOn", 1); setParam("delayMs", 350);
            setParam("delayFeedback", 0.22f); setParam("delayMix", 0.16f);
            setParam("reverbOn", 1); setParam("reverbMix", 0.09f);
            break;
        default: break;
    }
}

void GarasiAmpAudioProcessorEditor::savePreset()
{
    const auto defaultPath = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                 .getChildFile("GarasiTone.xml");
    chooser = std::make_unique<juce::FileChooser>("Save Garasi Amp preset", defaultPath, "*.xml");
    chooser->launchAsync(juce::FileBrowserComponent::saveMode |
                             juce::FileBrowserComponent::canSelectFiles |
                             juce::FileBrowserComponent::warnAboutOverwriting,
        [safe = juce::Component::SafePointer<GarasiAmpAudioProcessorEditor>(this)]
        (const juce::FileChooser& fc)
        {
            if (safe == nullptr) return;
            const auto file = fc.getResult();
            if (file == juce::File()) return;
            if (auto xml = safe->processor.parameters.copyState().createXml())
            {
                if (!file.replaceWithText(xml->toString()))
                    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                        "Save failed", "Cannot write the preset file.");
            }
        });
}

void GarasiAmpAudioProcessorEditor::loadPreset()
{
    const auto directory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    chooser = std::make_unique<juce::FileChooser>("Load Garasi Amp preset", directory, "*.xml");
    chooser->launchAsync(juce::FileBrowserComponent::openMode |
                             juce::FileBrowserComponent::canSelectFiles,
        [safe = juce::Component::SafePointer<GarasiAmpAudioProcessorEditor>(this)]
        (const juce::FileChooser& fc)
        {
            if (safe == nullptr) return;
            const auto file = fc.getResult();
            if (file == juce::File()) return;
            auto xml = juce::XmlDocument::parse(file);
            if (xml != nullptr && xml->hasTagName(safe->processor.parameters.state.getType()))
            {
                safe->processor.parameters.replaceState(juce::ValueTree::fromXml(*xml));
                safe->factoryPreset.setSelectedId(1, juce::dontSendNotification);
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                    "Invalid preset", "This file is not a Garasi Amp preset.");
            }
        });
}

void GarasiAmpAudioProcessorEditor::timerCallback()
{
    const float next = processor.outputPeak.load(std::memory_order_relaxed);
    smoothedPeak = std::max(next, smoothedPeak * 0.87f);
    repaint(meterRect.expanded(8, 31));
}
