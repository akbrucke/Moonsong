#include "PluginEditor.h"
#include <cmath>

namespace MoonsongPalette
{
    // Midnight Blue, Moonlight White, and Pale Ice Blue
    const juce::Colour midnightDeep   = juce::Colour::fromString ("FF070E1A");
    const juce::Colour midnightPanel  = juce::Colour::fromString ("FF0D192B");
    const juce::Colour midnightCard   = juce::Colour::fromString ("FF12223A");
    const juce::Colour midnightBorder = juce::Colour::fromString ("FF1E3659");
    const juce::Colour moonWhite      = juce::Colour::fromString ("FFF2F6FC");
    const juce::Colour moonSilver     = juce::Colour::fromString ("FFA8BCD6");
    const juce::Colour paleBlue       = juce::Colour::fromString ("FF7CC4FA");
    const juce::Colour paleIce        = juce::Colour::fromString ("FFBAE0FF");
}

namespace
{
    const juce::StringArray kDivItems {
        "1/16", "1/8T", "1/16D", "1/8", "1/4T", "1/8D", "1/4", "1/2T", "1/4D", "1/2", "1/1"
    };
}

juce::Font MoonsongLookAndFeel::getGaramondFont (float height, bool isBold)
{
    static const juce::Typeface::Ptr garamondTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::EBGaramond_ttf, BinaryData::EBGaramond_ttfSize);

    auto options = juce::FontOptions (garamondTypeface).withHeight (height);
    if (isBold)
        options = options.withStyle ("Bold");

    return juce::Font (options);
}

juce::Font MoonsongLookAndFeel::getInterFont (float height, bool isBold)
{
    static const juce::Typeface::Ptr interTypeface = juce::Typeface::createSystemTypefaceFor (
        BinaryData::Inter_ttf, BinaryData::Inter_ttfSize);

    auto options = juce::FontOptions (interTypeface).withHeight (height);
    if (isBold)
        options = options.withStyle ("Bold");

    return juce::Font (options);
}

MoonsongLookAndFeel::MoonsongLookAndFeel()
{
    setColour (juce::Label::textColourId, MoonsongPalette::moonWhite);
    setColour (juce::TextEditor::backgroundColourId, MoonsongPalette::midnightDeep);
    setColour (juce::TextEditor::textColourId, MoonsongPalette::moonWhite);
    setColour (juce::TextEditor::outlineColourId, MoonsongPalette::paleBlue);
    setColour (juce::PopupMenu::backgroundColourId, MoonsongPalette::midnightPanel);
    setColour (juce::PopupMenu::textColourId, MoonsongPalette::moonWhite);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, MoonsongPalette::paleBlue.withAlpha (0.28f));
    setColour (juce::PopupMenu::highlightedTextColourId, MoonsongPalette::moonWhite);
}

void MoonsongLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle,
                                           float rotaryEndAngle, juce::Slider& slider)
{
    const float radius  = std::min (width, height) * 0.5f - 4.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float angle   = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto accent   = slider.findColour (juce::Slider::rotarySliderFillColourId);

    g.setColour (MoonsongPalette::midnightDeep);
    g.fillEllipse (centreX - radius + 3.5f, centreY - radius + 3.5f,
                   (radius - 3.5f) * 2.0f, (radius - 3.5f) * 2.0f);

    juce::Path bgArc;
    bgArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (MoonsongPalette::midnightBorder.brighter (0.2f));
    g.strokePath (bgArc, juce::PathStrokeType (2.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    if (slider.isEnabled())
    {
        juce::Path valArc;
        valArc.addCentredArc (centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour (accent);
        g.strokePath (valArc, juce::PathStrokeType (2.9f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    juce::Path pointer;
    const float innerR = radius * 0.34f;
    const float outerR = radius - 3.0f;
    pointer.startNewSubPath (centreX + innerR * std::sin (angle), centreY - innerR * std::cos (angle));
    pointer.lineTo          (centreX + outerR * std::sin (angle), centreY - outerR * std::cos (angle));

    g.setColour (MoonsongPalette::moonWhite);
    g.strokePath (pointer, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void MoonsongLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float, float,
                                           juce::Slider::SliderStyle, juce::Slider&)
{
    const float trackW = 6.0f;
    const float cx = x + width * 0.5f;
    const auto trackRect = juce::Rectangle<float> (cx - trackW * 0.5f, static_cast<float> (y + 6),
                                                   trackW, static_cast<float> (height - 12));

    g.setColour (MoonsongPalette::midnightDeep);
    g.fillRoundedRectangle (trackRect, 3.0f);
    g.setColour (MoonsongPalette::midnightBorder);
    g.drawRoundedRectangle (trackRect, 3.0f, 1.0f);

    const auto fillRect = trackRect.withTop (sliderPos);
    g.setColour (MoonsongPalette::paleBlue);
    g.fillRoundedRectangle (fillRect, 3.0f);

    // Glowing moon orb thumb
    g.setColour (MoonsongPalette::paleBlue.withAlpha (0.32f));
    g.fillEllipse (cx - 10.0f, sliderPos - 10.0f, 20.0f, 20.0f);
    g.setColour (MoonsongPalette::moonWhite);
    g.fillEllipse (cx - 6.5f, sliderPos - 6.5f, 13.0f, 13.0f);
}

void MoonsongLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
    g.setColour (MoonsongPalette::midnightDeep);
    g.fillRoundedRectangle (bounds, 5.0f);

    g.setColour (box.hasKeyboardFocus (true) ? MoonsongPalette::paleBlue : MoonsongPalette::midnightBorder.brighter (0.2f));
    g.drawRoundedRectangle (bounds, 5.0f, 1.1f);

    const float arrowX = width - 12.0f;
    const float arrowY = height * 0.5f;
    juce::Path p;
    p.startNewSubPath (arrowX - 3.0f, arrowY - 1.5f);
    p.lineTo          (arrowX,        arrowY + 2.0f);
    p.lineTo          (arrowX + 3.0f, arrowY - 1.5f);
    g.setColour (MoonsongPalette::moonSilver);
    g.strokePath (p, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void MoonsongLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (5, 0, box.getWidth() - 18, box.getHeight());
    label.setFont (getInterFont (10.5f, true));
    label.setJustificationType (juce::Justification::centredLeft);
}

void MoonsongLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                           bool shouldDrawButtonAsHighlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = button.getToggleState();

    g.setColour (on ? MoonsongPalette::paleBlue.withAlpha (0.24f) : MoonsongPalette::midnightDeep);
    g.fillRoundedRectangle (bounds, 5.0f);

    g.setColour (on ? MoonsongPalette::paleBlue
                    : (shouldDrawButtonAsHighlighted ? MoonsongPalette::moonSilver : MoonsongPalette::midnightBorder.brighter (0.2f)));
    g.drawRoundedRectangle (bounds, 5.0f, on ? 1.3f : 1.0f);

    g.setColour (on ? MoonsongPalette::moonWhite : MoonsongPalette::moonSilver);
    g.setFont (MoonsongLookAndFeel::getInterFont (10.0f, true));
    g.drawText (button.getButtonText(), bounds, juce::Justification::centred);
}

//==============================================================================
MoonsongKnob::MoonsongKnob (juce::AudioProcessorValueTreeState& apvts,
                            const juce::String& paramID,
                            const juce::String& labelText,
                            const juce::String& unitText,
                            juce::Colour accentColour,
                            bool compact)
    : accent (accentColour), unit (unitText), isCompact (compact)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    slider.setMouseDragSensitivity (155);
    slider.onValueChange = [this] { updateValueText(); };
    addAndMakeVisible (slider);

    nameLabel.setText (labelText, juce::dontSendNotification);
    nameLabel.setJustificationType (juce::Justification::centred);
    nameLabel.setFont (MoonsongLookAndFeel::getInterFont (isCompact ? 9.5f : 10.5f, true));
    nameLabel.setColour (juce::Label::textColourId, MoonsongPalette::moonSilver);
    addAndMakeVisible (nameLabel);

    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setFont (MoonsongLookAndFeel::getInterFont (isCompact ? 10.0f : 11.0f, false));
    valueLabel.setColour (juce::Label::textColourId, MoonsongPalette::moonWhite);
    valueLabel.setEditable (false, true, false);
    valueLabel.onTextChange = [this]
    {
        const double typed = valueLabel.getText().getDoubleValue();
        slider.setValue (typed, juce::sendNotificationSync);
        updateValueText();
    };
    addAndMakeVisible (valueLabel);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramID, slider);
    updateValueText();
}

void MoonsongKnob::updateValueText()
{
    const double val = slider.getValue();
    if (unit == "PAN")
    {
        if (std::abs (val) < 0.02)
            valueLabel.setText ("C", juce::dontSendNotification);
        else if (val < 0.0)
            valueLabel.setText ("L" + juce::String (static_cast<int> (std::round (-val * 100.0))), juce::dontSendNotification);
        else
            valueLabel.setText ("R" + juce::String (static_cast<int> (std::round (val * 100.0))), juce::dontSendNotification);
        return;
    }

    if (unit == "Hz" && val >= 1000.0)
    {
        valueLabel.setText (juce::String (val / 1000.0, 2) + " kHz", juce::dontSendNotification);
        return;
    }

    int decimals = 1;
    if (unit == "Hz" || unit == "%" || unit == "ms")
        decimals = 0;
    else if (unit == "s" || unit == "HzLfo")
        decimals = 2;

    juce::String displayUnit = (unit == "HzLfo") ? "Hz" : unit;
    juce::String txt (val, decimals);
    if (unit == "dB" && val > 0.05)
        txt = "+" + txt;

    valueLabel.setText (txt + (displayUnit.isNotEmpty() ? (" " + displayUnit) : ""), juce::dontSendNotification);
}

void MoonsongKnob::resized()
{
    auto b = getLocalBounds();
    nameLabel.setBounds (b.removeFromTop (isCompact ? 13 : 15));
    valueLabel.setBounds (b.removeFromBottom (isCompact ? 14 : 16));
    slider.setBounds (b.reduced (1));
}

//==============================================================================
MoonsongAudioProcessorEditor::MoonsongAudioProcessorEditor (MoonsongAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      mixKnob        (p.apvts, "mix",         "MIX",       "%",     MoonsongPalette::moonWhite, false),
      dlyTimeLKnob   (p.apvts, "dlyTimeL",    "TIME L",    "ms",    MoonsongPalette::moonWhite, false),
      dlyTimeRKnob   (p.apvts, "dlyTimeR",    "TIME R",    "ms",    MoonsongPalette::paleBlue,  false),
      dlyFeedbackKnob(p.apvts, "dlyFeedback", "FEEDBACK",  "%",     MoonsongPalette::paleBlue,  false),
      dlyLevelKnob   (p.apvts, "dlyLevel",    "DELAY LVL", "%",     MoonsongPalette::moonWhite, false),
      revSizeKnob    (p.apvts, "revSize",     "SIZE",      "%",     MoonsongPalette::paleBlue,  false),
      revDecayKnob   (p.apvts, "revDecay",    "DECAY",     "s",     MoonsongPalette::moonWhite, false),
      revPreDelayKnob(p.apvts, "revPreDelay", "PRE-DLY",   "ms",    MoonsongPalette::paleBlue,  false),
      revDampKnob    (p.apvts, "revDamp",     "DAMPING",   "Hz",    MoonsongPalette::paleBlue,  false),
      revLevelKnob   (p.apvts, "revLevel",    "REVERB LVL","%",     MoonsongPalette::moonWhite, false),
      compPeakKnob   (p.apvts, "compPeak",    "PEAK RED",  "%",     MoonsongPalette::moonWhite, true),
      compGainKnob   (p.apvts, "compGain",    "MAKEUP",    "dB",    MoonsongPalette::paleBlue,  true),
      compMixKnob    (p.apvts, "compMix",     "BLEND",     "%",     MoonsongPalette::paleBlue,  true),
      chorusRateKnob (p.apvts, "chorusRate",  "RATE",      "HzLfo", MoonsongPalette::paleBlue,  true),
      chorusDepthKnob(p.apvts, "chorusDepth", "DEPTH",     "%",     MoonsongPalette::paleBlue,  true),
      chorusMixKnob  (p.apvts, "chorusMix",   "CHORUS",    "%",     MoonsongPalette::moonWhite, true),
      dlyHpfKnob     (p.apvts, "dlyHpf",      "HPF",       "Hz",    MoonsongPalette::paleBlue,  true),
      dlyLpfKnob     (p.apvts, "dlyLpf",      "LPF",       "Hz",    MoonsongPalette::paleBlue,  true),
      dlyEqFreqKnob  (p.apvts, "dlyEqFreq",   "EQ FREQ",   "Hz",    MoonsongPalette::moonWhite, true),
      dlyEqGainKnob  (p.apvts, "dlyEqGain",   "EQ GAIN",   "dB",    MoonsongPalette::moonWhite, true),
      dlySatKnob     (p.apvts, "dlySat",      "SATURATE",  "%",     MoonsongPalette::paleBlue,  true),
      revHpfKnob     (p.apvts, "revHpf",      "HPF",       "Hz",    MoonsongPalette::paleBlue,  true),
      revLpfKnob     (p.apvts, "revLpf",      "LPF",       "Hz",    MoonsongPalette::paleBlue,  true),
      revEqFreqKnob  (p.apvts, "revEqFreq",   "EQ FREQ",   "Hz",    MoonsongPalette::moonWhite, true),
      revEqGainKnob  (p.apvts, "revEqGain",   "EQ GAIN",   "dB",    MoonsongPalette::moonWhite, true),
      revSatKnob     (p.apvts, "revSat",      "SATURATE",  "%",     MoonsongPalette::paleBlue,  true)
{
    setLookAndFeel (&moonsongLnf);

    addAndMakeVisible (mixKnob);

    // Delay controls
    dlyModeBox.addItemList ({ "Ping-Pong", "Dual-Mono", "Mono" }, 1);
    dlyModeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "dlyMode", dlyModeBox);
    addAndMakeVisible (dlyModeBox);

    dlySyncAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "dlySync", dlySyncBtn);
    addAndMakeVisible (dlySyncBtn);

    multiTapAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "dlyMultiTap", multiTapBtn);
    addAndMakeVisible (multiTapBtn);

    dlyDivLBox.addItemList (kDivItems, 1);
    dlyDivRBox.addItemList (kDivItems, 1);
    dlyDivLAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "dlyDivL", dlyDivLBox);
    dlyDivRAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "dlyDivR", dlyDivRBox);
    addAndMakeVisible (dlyDivLBox);
    addAndMakeVisible (dlyDivRBox);

    for (auto* k : { &dlyTimeLKnob, &dlyTimeRKnob, &dlyFeedbackKnob, &dlyLevelKnob })
        addAndMakeVisible (k);

    // 5-Tap Multi-Tap Matrix
    for (int t = 0; t < 5; ++t)
    {
        const auto idx = juce::String (t + 1);
        auto& tc = tapControls[static_cast<size_t> (t)];
        tc.enableBtn.setButtonText ("TAP " + idx);
        tc.enableAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "tap" + idx + "On", tc.enableBtn);
        addChildComponent (tc.enableBtn);

        tc.timeKnob = std::make_unique<MoonsongKnob> (p.apvts, "tap" + idx + "Time", "TIME", "ms", MoonsongPalette::moonWhite, true);
        addChildComponent (*tc.timeKnob);

        tc.divBox.addItemList (kDivItems, 1);
        tc.divAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "tap" + idx + "Div", tc.divBox);
        addChildComponent (tc.divBox);

        tc.panKnob = std::make_unique<MoonsongKnob> (p.apvts, "tap" + idx + "Pan", "PAN", "PAN", MoonsongPalette::paleBlue, true);
        addChildComponent (*tc.panKnob);

        tc.gainKnob = std::make_unique<MoonsongKnob> (p.apvts, "tap" + idx + "Gain", "LVL", "%", MoonsongPalette::moonWhite, true);
        addChildComponent (*tc.gainKnob);
    }

    // Reverb controls
    for (auto* k : { &revSizeKnob, &revDecayKnob, &revPreDelayKnob, &revDampKnob, &revLevelKnob })
        addAndMakeVisible (k);

    // Left Drawer (Optical Comp)
    compOnAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "compOn", compOnBtn);
    addChildComponent (compOnBtn);
    for (auto* k : { &compPeakKnob, &compGainKnob, &compMixKnob })
        addChildComponent (k);

    // Right Drawer (Eclipse Mode Slider + Post Chorus)
    eclipseSlider.setSliderStyle (juce::Slider::LinearVertical);
    eclipseSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    eclipseSlider.onValueChange = [this]
    {
        eclipseValLabel.setText (juce::String (static_cast<int> (std::round (eclipseSlider.getValue()))) + "%",
                                 juce::dontSendNotification);
    };
    eclipseAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "eclipse", eclipseSlider);
    addChildComponent (eclipseSlider);

    eclipseLabel.setText ("ECLIPSE", juce::dontSendNotification);
    eclipseLabel.setJustificationType (juce::Justification::centred);
    eclipseLabel.setFont (MoonsongLookAndFeel::getInterFont (10.0f, true));
    eclipseLabel.setColour (juce::Label::textColourId, MoonsongPalette::paleBlue);
    addChildComponent (eclipseLabel);

    eclipseValLabel.setText ("0%", juce::dontSendNotification);
    eclipseValLabel.setJustificationType (juce::Justification::centred);
    eclipseValLabel.setFont (MoonsongLookAndFeel::getInterFont (10.5f, false));
    eclipseValLabel.setColour (juce::Label::textColourId, MoonsongPalette::moonWhite);
    addChildComponent (eclipseValLabel);

    chorusOnAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, "chorusOn", chorusOnBtn);
    addChildComponent (chorusOnBtn);
    for (auto* k : { &chorusRateKnob, &chorusDepthKnob, &chorusMixKnob })
        addChildComponent (k);

    // Bottom Drawer (Pre/Post Tone + Sat)
    dlyTonePosBox.addItemList ({ "PRE-FX", "POST-FX" }, 1);
    dlyTonePosAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "dlyTonePos", dlyTonePosBox);
    addChildComponent (dlyTonePosBox);

    dlyEqTypeBox.addItemList ({ "Broad Bell", "High Shelf" }, 1);
    dlyEqTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "dlyEqType", dlyEqTypeBox);
    addChildComponent (dlyEqTypeBox);

    revTonePosBox.addItemList ({ "PRE-FX", "POST-FX" }, 1);
    revTonePosAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "revTonePos", revTonePosBox);
    addChildComponent (revTonePosBox);

    revEqTypeBox.addItemList ({ "Broad Bell", "High Shelf" }, 1);
    revEqTypeAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (p.apvts, "revEqType", revEqTypeBox);
    addChildComponent (revEqTypeBox);

    for (auto* k : { &dlyHpfKnob, &dlyLpfKnob, &dlyEqFreqKnob, &dlyEqGainKnob, &dlySatKnob,
                     &revHpfKnob, &revLpfKnob, &revEqFreqKnob, &revEqGainKnob, &revSatKnob })
    {
        addChildComponent (k);
    }

    setResizable(true, true);
    setResizeLimits(920, 520, 1600, 960);
    setSize(1040, 580);
    updateVisibilityStates();
    startTimerHz (60);
}

MoonsongAudioProcessorEditor::~MoonsongAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void MoonsongAudioProcessorEditor::timerCallback()
{
    const float rawIn = audioProcessor.meterInDb.load();
    const float rawGr = audioProcessor.meterGrDb.load();
    smoothInDb = (rawIn > smoothInDb) ? rawIn : (smoothInDb * 0.85f + rawIn * 0.15f);
    smoothGrDb = (rawGr > smoothGrDb) ? rawGr : (smoothGrDb * 0.86f + rawGr * 0.14f);

    // Advance shimmering ocean water phase (subtly modulated by post chorus rate)
    const float cRate = audioProcessor.apvts.getRawParameterValue ("chorusRate")->load();
    waterPhase += 0.032f + cRate * 0.012f;
    if (waterPhase > 1000.0f)
        waterPhase -= 1000.0f;

    // Smooth drawer open/close animation
    const float targetL = leftDrawerOpen   ? 1.0f : 0.0f;
    const float targetR = rightDrawerOpen  ? 1.0f : 0.0f;
    const float targetB = bottomDrawerOpen ? 1.0f : 0.0f;

    const bool animChanged = (std::abs (leftAnim - targetL) > 0.005f)
                          || (std::abs (rightAnim - targetR) > 0.005f)
                          || (std::abs (bottomAnim - targetB) > 0.005f);

    leftAnim   += (targetL - leftAnim)   * 0.24f;
    rightAnim  += (targetR - rightAnim)  * 0.24f;
    bottomAnim += (targetB - bottomAnim) * 0.24f;

    if (animChanged)
    {
        if (std::abs (leftAnim - targetL)   < 0.005f) leftAnim   = targetL;
        if (std::abs (rightAnim - targetR)  < 0.005f) rightAnim  = targetR;
        if (std::abs (bottomAnim - targetB) < 0.005f) bottomAnim = targetB;
        resized();
    }

    updateVisibilityStates();
    repaint();
}

void MoonsongAudioProcessorEditor::updateVisibilityStates()
{
    const bool isMultiTap = audioProcessor.apvts.getRawParameterValue ("dlyMultiTap")->load() > 0.5f;
    const bool isSync     = audioProcessor.apvts.getRawParameterValue ("dlySync")->load() > 0.5f;

    const bool layoutChanged = (isMultiTap != lastMultiTapState) || (isSync != lastSyncState);
    lastMultiTapState = isMultiTap;
    lastSyncState     = isSync;

    dlyModeBox.setVisible (! isMultiTap);
    dlyTimeLKnob.setVisible (! isMultiTap && ! isSync);
    dlyTimeRKnob.setVisible (! isMultiTap && ! isSync);
    dlyDivLBox.setVisible (! isMultiTap && isSync);
    dlyDivRBox.setVisible (! isMultiTap && isSync);
    dlyFeedbackKnob.setVisible (true);
    dlyLevelKnob.setVisible (true);

    for (int t = 0; t < 5; ++t)
    {
        auto& tc = tapControls[static_cast<size_t> (t)];
        tc.enableBtn.setVisible (isMultiTap);
        tc.timeKnob->setVisible (isMultiTap && ! isSync);
        tc.divBox.setVisible    (isMultiTap && isSync);
        tc.panKnob->setVisible  (isMultiTap);
        tc.gainKnob->setVisible (isMultiTap);
    }

    const bool showLeft = leftAnim > 0.65f;
    compOnBtn.setVisible (showLeft);
    compPeakKnob.setVisible (showLeft);
    compGainKnob.setVisible (showLeft);
    compMixKnob.setVisible (showLeft);

    const bool showRight = rightAnim > 0.65f;
    eclipseSlider.setVisible (showRight);
    eclipseLabel.setVisible (showRight);
    eclipseValLabel.setVisible (showRight);
    chorusOnBtn.setVisible (showRight);
    chorusRateKnob.setVisible (showRight);
    chorusDepthKnob.setVisible (showRight);
    chorusMixKnob.setVisible (showRight);

    const bool showBottom = bottomAnim > 0.65f;
    for (auto* c : std::initializer_list<juce::Component*> {
        &dlyTonePosBox, &dlyHpfKnob, &dlyLpfKnob, &dlyEqTypeBox, &dlyEqFreqKnob, &dlyEqGainKnob, &dlySatKnob,
        &revTonePosBox, &revHpfKnob, &revLpfKnob, &revEqTypeBox, &revEqFreqKnob, &revEqGainKnob, &revSatKnob })
    {
        c->setVisible (showBottom);
    }

    if (layoutChanged)
        resized();
}

void MoonsongAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    const auto pt = e.getPosition();
    if (leftTabBounds.contains (pt))
    {
        leftDrawerOpen = ! leftDrawerOpen;
        return;
    }
    if (rightTabBounds.contains (pt))
    {
        rightDrawerOpen = ! rightDrawerOpen;
        return;
    }
    if (bottomTabBounds.contains (pt))
    {
        bottomDrawerOpen = ! bottomDrawerOpen;
        return;
    }
}

void MoonsongAudioProcessorEditor::resized()
{
    auto b = getLocalBounds().reduced (10);

    // Header strip
    auto header = b.removeFromTop (44);
    mixKnob.setBounds (header.removeFromRight (74));
    b.removeFromTop (6);

    // Bottom Drawer + Tab
    const int bottomH = static_cast<int> (std::round (bottomAnim * 138.0f));
    if (bottomH > 0)
    {
        bottomDrawerArea = b.removeFromBottom (bottomH);
        b.removeFromBottom (4);
    }
    else
    {
        bottomDrawerArea = {};
    }
    bottomTabBounds = b.removeFromBottom (22).withSizeKeepingCentre (240, 22);
    b.removeFromBottom (6);

    // Left Drawer + Tab
    const int leftW = static_cast<int> (std::round (leftAnim * 116.0f));
    if (leftW > 0)
    {
        leftDrawerArea = b.removeFromLeft (leftW);
        b.removeFromLeft (4);
    }
    else
    {
        leftDrawerArea = {};
    }
    leftTabBounds = b.removeFromLeft (22).withSizeKeepingCentre (22, 160);
    b.removeFromLeft (6);

    // Right Drawer + Tab
    const int rightW = static_cast<int> (std::round (rightAnim * 158.0f));
    if (rightW > 0)
    {
        rightDrawerArea = b.removeFromRight (rightW);
        b.removeFromRight (4);
    }
    else
    {
        rightDrawerArea = {};
    }
    rightTabBounds = b.removeFromRight (22).withSizeKeepingCentre (22, 170);
    b.removeFromRight (6);

    // Center Workspace: Slightly smaller Moon/Ocean display on top, Delay + Reverb engines below
    const int oceanH = static_cast<int> (b.getHeight() * 0.41f);
    oceanArea = b.removeFromTop (oceanH);
    b.removeFromTop (8);

    const int dlyW = static_cast<int> (b.getWidth() * 0.56f);
    delayCardArea = b.removeFromLeft (dlyW);
    b.removeFromLeft (8);
    reverbCardArea = b;

    // Layout Delay Card
    {
        auto d = delayCardArea.reduced (10, 8);
        auto topRow = d.removeFromTop (24);
        topRow.removeFromLeft (64); // Title space
        multiTapBtn.setBounds (topRow.removeFromRight (78).reduced (1));
        topRow.removeFromRight (6);
        dlySyncBtn.setBounds (topRow.removeFromRight (52).reduced (1));
        topRow.removeFromRight (6);
        if (dlyModeBox.isVisible())
            dlyModeBox.setBounds (topRow.removeFromRight (96).reduced (1));

        d.removeFromTop (6);
        const bool isMultiTap = audioProcessor.apvts.getRawParameterValue ("dlyMultiTap")->load() > 0.5f;

        if (! isMultiTap)
        {
            const int colW = d.getWidth() / 4;
            auto c1 = d.removeFromLeft (colW).reduced (2);
            auto c2 = d.removeFromLeft (colW).reduced (2);
            auto c3 = d.removeFromLeft (colW).reduced (2);
            auto c4 = d.reduced (2);

            dlyTimeLKnob.setBounds (c1);
            dlyTimeRKnob.setBounds (c2);
            dlyDivLBox.setBounds (c1.withSizeKeepingCentre (c1.getWidth() - 6, 26));
            dlyDivRBox.setBounds (c2.withSizeKeepingCentre (c2.getWidth() - 6, 26));
            dlyFeedbackKnob.setBounds (c3);
            dlyLevelKnob.setBounds (c4);
        }
        else
        {
            // Morph into 5-Tap Panner Matrix + Global FB/Level on the right
            auto globalCol = d.removeFromRight (74);
            dlyFeedbackKnob.setBounds (globalCol.removeFromTop (globalCol.getHeight() / 2).reduced (1));
            dlyLevelKnob.setBounds (globalCol.reduced (1));
            d.removeFromRight (4);

            const int tapW = d.getWidth() / 5;
            for (int t = 0; t < 5; ++t)
            {
                auto col = (t < 4 ? d.removeFromLeft (tapW) : d).reduced (2, 0);
                auto& tc = tapControls[static_cast<size_t> (t)];
                tc.enableBtn.setBounds (col.removeFromTop (20));
                col.removeFromTop (3);
                auto timingZone = col.removeFromTop (48);
                tc.timeKnob->setBounds (timingZone);
                tc.divBox.setBounds (timingZone.withSizeKeepingCentre (timingZone.getWidth() - 2, 22));
                auto bottomZone = col;
                tc.panKnob->setBounds  (bottomZone.removeFromLeft (bottomZone.getWidth() / 2));
                tc.gainKnob->setBounds (bottomZone);
            }
        }
    }

    // Layout Reverb Card
    {
        auto r = reverbCardArea.reduced (10, 8);
        r.removeFromTop (24);
        r.removeFromTop (6);
        const int colW = r.getWidth() / 5;
        revSizeKnob.setBounds     (r.removeFromLeft (colW).reduced (1));
        revDecayKnob.setBounds    (r.removeFromLeft (colW).reduced (1));
        revPreDelayKnob.setBounds (r.removeFromLeft (colW).reduced (1));
        revDampKnob.setBounds     (r.removeFromLeft (colW).reduced (1));
        revLevelKnob.setBounds    (r.reduced (1));
    }

    // Layout Left Drawer (Optical Compressor)
    if (! leftDrawerArea.isEmpty())
    {
        auto l = leftDrawerArea.reduced (8, 8);
        l.removeFromTop (16);
        compOnBtn.setBounds (l.removeFromTop (22));
        l.removeFromTop (6);
        const int kh = l.getHeight() / 3;
        compPeakKnob.setBounds (l.removeFromTop (kh).reduced (2));
        compGainKnob.setBounds (l.removeFromTop (kh).reduced (2));
        compMixKnob.setBounds  (l.reduced (2));
    }

    // Layout Right Drawer (Eclipse Mode Slider + Post Chorus)
    if (! rightDrawerArea.isEmpty())
    {
        auto r = rightDrawerArea.reduced (8, 8);
        r.removeFromTop (16);
        auto eclCol = r.removeFromLeft (56);
        eclipseLabel.setBounds (eclCol.removeFromTop (16));
        eclipseValLabel.setBounds (eclCol.removeFromBottom (16));
        eclipseSlider.setBounds (eclCol.reduced (4, 2));

        r.removeFromLeft (6);
        chorusOnBtn.setBounds (r.removeFromTop (22));
        r.removeFromTop (4);
        const int kh = r.getHeight() / 3;
        chorusRateKnob.setBounds  (r.removeFromTop (kh).reduced (1));
        chorusDepthKnob.setBounds (r.removeFromTop (kh).reduced (1));
        chorusMixKnob.setBounds   (r.reduced (1));
    }

    // Layout Bottom Drawer (Delay & Reverb Pre/Post Tone & Saturation)
    if (! bottomDrawerArea.isEmpty())
    {
        auto bot = bottomDrawerArea.reduced (10, 6);
        auto leftHalf = bot.removeFromLeft (bot.getWidth() / 2 - 6);
        bot.removeFromLeft (12);
        auto rightHalf = bot;

        auto layoutToneStrip = [] (juce::Rectangle<int> area,
                                   juce::ComboBox& posBox, MoonsongKnob& hpf, MoonsongKnob& lpf,
                                   juce::ComboBox& eqType, MoonsongKnob& eqF, MoonsongKnob& eqG, MoonsongKnob& sat)
        {
            auto top = area.removeFromTop (22);
            top.removeFromLeft (110);
            eqType.setBounds (top.removeFromRight (88));
            top.removeFromRight (6);
            posBox.setBounds (top.removeFromRight (78));

            area.removeFromTop (4);
            const int kw = area.getWidth() / 5;
            hpf.setBounds (area.removeFromLeft (kw).reduced (1));
            lpf.setBounds (area.removeFromLeft (kw).reduced (1));
            eqF.setBounds (area.removeFromLeft (kw).reduced (1));
            eqG.setBounds (area.removeFromLeft (kw).reduced (1));
            sat.setBounds (area.reduced (1));
        };

        layoutToneStrip (leftHalf,  dlyTonePosBox, dlyHpfKnob, dlyLpfKnob, dlyEqTypeBox, dlyEqFreqKnob, dlyEqGainKnob, dlySatKnob);
        layoutToneStrip (rightHalf, revTonePosBox, revHpfKnob, revLpfKnob, revEqTypeBox, revEqFreqKnob, revEqGainKnob, revSatKnob);
    }
}

void MoonsongAudioProcessorEditor::drawDrawerTab (juce::Graphics& g, juce::Rectangle<int> bounds,
                                                  const juce::String& text, bool isOpen, bool vertical)
{
    auto b = bounds.toFloat();
    g.setColour (isOpen ? MoonsongPalette::paleBlue.withAlpha (0.24f) : MoonsongPalette::midnightPanel);
    g.fillRoundedRectangle (b, 5.0f);
    g.setColour (isOpen ? MoonsongPalette::paleBlue : MoonsongPalette::midnightBorder);
    g.drawRoundedRectangle (b, 5.0f, 1.0f);

    g.setColour (isOpen ? MoonsongPalette::moonWhite : MoonsongPalette::moonSilver);
    g.setFont (MoonsongLookAndFeel::getInterFont (9.5, true));

    if (vertical)
    {
        juce::Graphics::ScopedSaveState ss (g);
        g.addTransform (juce::AffineTransform::rotation (-juce::MathConstants<float>::halfPi,
                                                         b.getCentreX(), b.getCentreY()));
        g.drawText (text, b.withSizeKeepingCentre (b.getHeight(), b.getWidth()), juce::Justification::centred);
    }
    else
    {
        g.drawText (text, b, juce::Justification::centred);
    }
}

void MoonsongAudioProcessorEditor::drawOceanDisplay (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto b = bounds.toFloat();
    juce::Graphics::ScopedSaveState ss (g);
    juce::Path clip;
    clip.addRoundedRectangle (b, 8.0f);
    g.reduceClipRegion (clip);

    const float horizonY = b.getY() + b.getHeight() * 0.46f;

    // 1. Night Sky Gradient
    juce::ColourGradient skyGrad (juce::Colour::fromString ("FF040914"), b.getCentreX(), b.getY(),
                                  juce::Colour::fromString ("FF0E213D"), b.getCentreX(), horizonY, false);
    g.setGradientFill (skyGrad);
    g.fillRect (b.withBottom (horizonY));

    // Subtle stars
    g.setColour (MoonsongPalette::moonWhite.withAlpha (0.45f));
    constexpr float starCoords[14][2] = {
        { 0.12f, 0.18f }, { 0.21f, 0.34f }, { 0.33f, 0.14f }, { 0.41f, 0.29f },
        { 0.59f, 0.16f }, { 0.67f, 0.31f }, { 0.76f, 0.12f }, { 0.84f, 0.25f },
        { 0.91f, 0.19f }, { 0.08f, 0.36f }, { 0.28f, 0.24f }, { 0.72f, 0.38f },
        { 0.63f, 0.22f }, { 0.37f, 0.39f }
    };
    for (int i = 0; i < 14; ++i)
    {
        const float sx = b.getX() + starCoords[i][0] * b.getWidth();
        const float sy = b.getY() + starCoords[i][1] * (horizonY - b.getY());
        g.fillEllipse (sx, sy, (i % 3 == 0) ? 1.8f : 1.2f, (i % 3 == 0) ? 1.8f : 1.2f);
    }

    // 2. Full Moon & Dynamic Incoming Vocal Level Corona
    const float inNorm = juce::jlimit (0.0f, 1.0f, (smoothInDb + 54.0f) / 54.0f);
    const float moonX  = b.getCentreX();
    const float moonY  = b.getY() + (horizonY - b.getY()) * 0.44f;
    const float moonR  = 17.0f;
    const float haloR  = moonR + 12.0f + inNorm * 38.0f;

    juce::ColourGradient haloGrad (MoonsongPalette::moonWhite.withAlpha (0.34f + inNorm * 0.24f), moonX, moonY,
                                   MoonsongPalette::paleBlue.withAlpha (0.0f), moonX + haloR, moonY, true);
    haloGrad.addColour (0.45, MoonsongPalette::paleIce.withAlpha (0.14f + inNorm * 0.14f));
    g.setGradientFill (haloGrad);
    g.fillEllipse (moonX - haloR, moonY - haloR, haloR * 2.0f, haloR * 2.0f);

    // Moon disc
    g.setColour (MoonsongPalette::moonWhite);
    g.fillEllipse (moonX - moonR, moonY - moonR, moonR * 2.0f, moonR * 2.0f);

    // Subtle lunar mare texture
    g.setColour (MoonsongPalette::moonSilver.withAlpha (0.18f));
    g.fillEllipse (moonX - 7.0f, moonY - 5.0f, 8.0f, 6.0f);
    g.fillEllipse (moonX + 2.0f, moonY + 1.0f, 6.0f, 5.0f);

    // 3. Ocean Surface & Animated Moonlight Shimmer
    juce::ColourGradient seaGrad (juce::Colour::fromString ("FF081629"), b.getCentreX(), horizonY,
                                  juce::Colour::fromString ("FF040A14"), b.getCentreX(), b.getBottom(), false);
    g.setGradientFill (seaGrad);
    g.fillRect (b.withTop (horizonY));

    // Horizon line glow
    g.setColour (MoonsongPalette::paleBlue.withAlpha (0.28f));
    g.drawHorizontalLine (static_cast<int> (horizonY), b.getX(), b.getRight());

    const float eclipseNorm = audioProcessor.apvts.getRawParameterValue ("eclipse")->load() * 0.01f;
    const float seaH = b.getBottom() - horizonY;
    const int numRipples = static_cast<int> (seaH / 2.5f);

    for (int r = 0; r < numRipples; ++r)
    {
        const float t = static_cast<float> (r) / std::max (1, numRipples); // 0 at horizon -> 1 at foreground
        const float y = horizonY + 2.0f + t * (seaH - 3.0f);

        // Perspective widening + Eclipse stereo spread widening
        const float perspectiveW = (14.0f + t * 88.0f) * (1.0f + eclipseNorm * 0.55f);
        const float wave1 = std::sin (t * 26.0f - waterPhase * 1.4f);
        const float wave2 = std::cos (t * 43.0f + waterPhase * 0.9f);
        const float centerShift = (wave1 * 6.0f + wave2 * 4.0f) * (0.25f + t * 0.95f);

        const float shimmer = 0.5f + 0.5f * std::sin (t * 52.0f - waterPhase * 2.1f + wave2 * 2.0f);
        if (shimmer < 0.22f)
            continue;

        const float sliceW = perspectiveW * (0.35f + 0.65f * shimmer);
        const float sliceX = moonX + centerShift - sliceW * 0.5f;
        const float alpha  = (0.14f + 0.58f * shimmer) * (1.0f - t * 0.35f) * (0.75f + 0.25f * inNorm);

        g.setColour (MoonsongPalette::moonWhite.withAlpha (juce::jlimit (0.0f, 0.88f, alpha)));
        g.fillRoundedRectangle (sliceX, y, sliceW, 1.4f + t * 0.9f, 0.7f);

        // Secondary pale blue flank reflections when Eclipse Mode is raised
        if (eclipseNorm > 0.05f && r % 2 == 0)
        {
            const float flankOffset = perspectiveW * 0.62f;
            const float flankW = sliceW * 0.42f * eclipseNorm;
            g.setColour (MoonsongPalette::paleBlue.withAlpha (alpha * 0.55f * eclipseNorm));
            g.fillRoundedRectangle (moonX + centerShift - flankOffset - flankW * 0.5f, y, flankW, 1.3f, 0.6f);
            g.fillRoundedRectangle (moonX + centerShift + flankOffset - flankW * 0.5f, y, flankW, 1.3f, 0.6f);
        }
    }

    // Outer border
    g.setColour (MoonsongPalette::midnightBorder);
    g.drawRoundedRectangle (b, 8.0f, 1.1f);
}

void MoonsongAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (MoonsongPalette::midnightDeep);

    // Header
    auto header = getLocalBounds().reduced (10).removeFromTop (44);
    header.removeFromLeft(28);
    g.setColour (MoonsongPalette::moonWhite);
    g.setFont (MoonsongLookAndFeel::getGaramondFont (21.0f, true));
    g.drawText ("MOONSONG", header.removeFromLeft (145), juce::Justification::centredLeft);

    g.setColour (MoonsongPalette::paleBlue);
    g.setFont (MoonsongLookAndFeel::getInterFont (11.0f, false));
    g.drawText ("3D VOCAL DELAY & REVERB CONSOLE", header, juce::Justification::centredLeft);

    // Ocean & Moon Viewport
    drawOceanDisplay (g, oceanArea);

    // Delay Card
    g.setColour (MoonsongPalette::midnightPanel);
    g.fillRoundedRectangle (delayCardArea.toFloat(), 8.0f);
    g.setColour (MoonsongPalette::midnightBorder);
    g.drawRoundedRectangle (delayCardArea.toFloat(), 8.0f, 1.0f);
    g.setColour (MoonsongPalette::moonWhite);
    g.setFont (MoonsongLookAndFeel::getInterFont (11.5f, true));
    g.drawText ("DELAY", delayCardArea.reduced (12, 9).removeFromTop (20), juce::Justification::centredLeft);

    // Reverb Card
    g.setColour (MoonsongPalette::midnightPanel);
    g.fillRoundedRectangle (reverbCardArea.toFloat(), 8.0f);
    g.setColour (MoonsongPalette::midnightBorder);
    g.drawRoundedRectangle (reverbCardArea.toFloat(), 8.0f, 1.0f);
    g.setColour (MoonsongPalette::paleBlue);
    g.setFont (MoonsongLookAndFeel::getInterFont (11.5f, true));
    g.drawText ("REVERB", reverbCardArea.reduced (12, 9).removeFromTop (20), juce::Justification::centredLeft);

    // Drawer Tabs
    drawDrawerTab (g, leftTabBounds,   leftDrawerOpen   ? "COMP <" : "COMP >", leftDrawerOpen, true);
    drawDrawerTab (g, rightTabBounds,  rightDrawerOpen  ? "> MOD"  : "< MOD",  rightDrawerOpen, true);
    drawDrawerTab (g, bottomTabBounds, bottomDrawerOpen ? "v  TONE & SATURATION  v" : "^  TONE & SATURATION  ^",
                   bottomDrawerOpen, false);

    // Left Drawer Panel (Optical Comp + GR Meter)
    if (! leftDrawerArea.isEmpty())
    {
        auto b = leftDrawerArea.toFloat();
        g.setColour (MoonsongPalette::midnightPanel);
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (MoonsongPalette::paleBlue.withAlpha (0.5f));
        g.drawRoundedRectangle (b, 8.0f, 1.0f);

        g.setColour (MoonsongPalette::moonWhite);
        g.setFont (MoonsongLookAndFeel::getInterFont (10.0f, true));
        g.drawText ("OPTO COMP", leftDrawerArea.reduced (8, 5).removeFromTop (14), juce::Justification::centredLeft);

        if (leftAnim > 0.65f)
        {
            // Small GR indicator bar at top right of left drawer
            auto grBar = leftDrawerArea.reduced (8, 6).removeFromTop (8).removeFromRight (32).toFloat();
            g.setColour (MoonsongPalette::midnightDeep);
            g.fillRoundedRectangle (grBar, 2.0f);
            const float grNorm = juce::jlimit (0.0f, 1.0f, smoothGrDb / 18.0f);
            g.setColour (MoonsongPalette::paleBlue);
            g.fillRoundedRectangle (grBar.withWidth (grBar.getWidth() * grNorm), 2.0f);
        }
    }

    // Right Drawer Panel (Eclipse + Post Chorus)
    if (! rightDrawerArea.isEmpty())
    {
        auto b = rightDrawerArea.toFloat();
        g.setColour (MoonsongPalette::midnightPanel);
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (MoonsongPalette::paleBlue.withAlpha (0.5f));
        g.drawRoundedRectangle (b, 8.0f, 1.0f);

        g.setColour (MoonsongPalette::paleBlue);
        g.setFont (MoonsongLookAndFeel::getInterFont (10.0f, true));
        g.drawText ("SPACE & CHORUS", rightDrawerArea.reduced (8, 5).removeFromTop (14), juce::Justification::centred);
    }

    // Bottom Drawer Panel (Pre/Post Filters, Bell/Shelf EQ & Saturation)
    if (! bottomDrawerArea.isEmpty())
    {
        auto b = bottomDrawerArea.toFloat();
        g.setColour (MoonsongPalette::midnightPanel);
        g.fillRoundedRectangle (b, 8.0f);
        g.setColour (MoonsongPalette::paleBlue.withAlpha (0.5f));
        g.drawRoundedRectangle (b, 8.0f, 1.0f);

        auto inner = bottomDrawerArea.reduced (10, 7);
        auto leftHdr = inner.removeFromLeft (inner.getWidth() / 2 - 6).removeFromTop (18);
        inner.removeFromLeft (12);
        auto rightHdr = inner.removeFromTop (18);

        g.setColour (MoonsongPalette::moonWhite);
        g.setFont (MoonsongLookAndFeel::getInterFont (10.5f, true));
        g.drawText ("DELAY TONE & SAT", leftHdr, juce::Justification::centredLeft);

        g.setColour (MoonsongPalette::paleBlue);
        g.drawText ("REVERB TONE & SAT", rightHdr, juce::Justification::centredLeft);

        // Center divider
        g.setColour (MoonsongPalette::midnightBorder);
        g.drawVerticalLine (bottomDrawerArea.getCentreX(),
                            static_cast<float> (bottomDrawerArea.getY() + 8),
                            static_cast<float> (bottomDrawerArea.getBottom() - 8));
    }
}