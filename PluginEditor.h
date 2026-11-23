#pragma once

#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"
#include <array>

class MoonsongLookAndFeel : public juce::LookAndFeel_V4

{
public:
    MoonsongLookAndFeel();

    static juce::Font getGaramondFont (float height, bool isBold = false);
    static juce::Font getInterFont (float height, bool isBold = false);
    
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

class MoonsongKnob : public juce::Component
{
public:
    MoonsongKnob (juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramID,
                  const juce::String& labelText,
                  const juce::String& unitText,
                  juce::Colour accentColour,
                  bool compact = false);

    void resized() override;
    void paint (juce::Graphics&) override {}
    void setUnit (const juce::String& newUnit) { unit = newUnit; updateValueText(); }

private:
    juce::Slider slider;
    juce::Label nameLabel;
    juce::Label valueLabel;
    juce::Colour accent;
    juce::String unit;
    bool isCompact;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    void updateValueText();
};

class MoonsongAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit MoonsongAudioProcessorEditor (MoonsongAudioProcessor&);
    ~MoonsongAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;

private:
    MoonsongAudioProcessor& audioProcessor;
    MoonsongLookAndFeel moonsongLnf;

    // Collapsible Drawer States & Animated Ratios (0.0 = collapsed, 1.0 = open)
    bool leftDrawerOpen   = false;
    bool rightDrawerOpen  = false;
    bool bottomDrawerOpen = false;
    float leftAnim   = 0.0f;
    float rightAnim  = 0.0f;
    float bottomAnim = 0.0f;

    juce::Rectangle<int> leftTabBounds;
    juce::Rectangle<int> rightTabBounds;
    juce::Rectangle<int> bottomTabBounds;

    // Header / Global
    MoonsongKnob mixKnob;

    // Central Delay Section (Standard vs Multi-Tap Morph)
    juce::ComboBox dlyModeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dlyModeAtt;

    juce::ToggleButton dlySyncBtn { "SYNC" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dlySyncAtt;

    juce::ToggleButton multiTapBtn { "MULTI-TAP" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> multiTapAtt;

    MoonsongKnob dlyTimeLKnob;
    MoonsongKnob dlyTimeRKnob;
    juce::ComboBox dlyDivLBox;
    juce::ComboBox dlyDivRBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dlyDivLAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dlyDivRAtt;
    MoonsongKnob dlyFeedbackKnob;
    MoonsongKnob dlyLevelKnob;

    // 5-Tap Multi-Tap Matrix Controls
    struct TapControls
    {
        juce::ToggleButton enableBtn;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAtt;
        std::unique_ptr<MoonsongKnob> timeKnob;
        juce::ComboBox divBox;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divAtt;
        std::unique_ptr<MoonsongKnob> panKnob;
        std::unique_ptr<MoonsongKnob> gainKnob;
    };
    std::array<TapControls, 5> tapControls;

    // Central Reverb Section
    MoonsongKnob revSizeKnob;
    MoonsongKnob revDecayKnob;
    MoonsongKnob revPreDelayKnob;
    MoonsongKnob revDampKnob;
    MoonsongKnob revLevelKnob;

    // Left Drawer: Optical Compressor
    juce::ToggleButton compOnBtn { "OPTO ACTIVE" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> compOnAtt;
    MoonsongKnob compPeakKnob;
    MoonsongKnob compGainKnob;
    MoonsongKnob compMixKnob;

    // Right Drawer: Eclipse Doubler + Post Stereo Chorus
    juce::Slider eclipseSlider;
    juce::Label  eclipseLabel;
    juce::Label  eclipseValLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> eclipseAtt;

    juce::ToggleButton chorusOnBtn { "CHORUS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> chorusOnAtt;
    MoonsongKnob chorusRateKnob;
    MoonsongKnob chorusDepthKnob;
    MoonsongKnob chorusMixKnob;

    // Bottom Drawer: Delay & Reverb Tone/Sat
    juce::ComboBox dlyTonePosBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dlyTonePosAtt;
    MoonsongKnob dlyHpfKnob;
    MoonsongKnob dlyLpfKnob;
    juce::ComboBox dlyEqTypeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> dlyEqTypeAtt;
    MoonsongKnob dlyEqFreqKnob;
    MoonsongKnob dlyEqGainKnob;
    MoonsongKnob dlySatKnob;

    juce::ComboBox revTonePosBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> revTonePosAtt;
    MoonsongKnob revHpfKnob;
    MoonsongKnob revLpfKnob;
    juce::ComboBox revEqTypeBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> revEqTypeAtt;
    MoonsongKnob revEqFreqKnob;
    MoonsongKnob revEqGainKnob;
    MoonsongKnob revSatKnob;

    // Layout Regions & Animation State
    juce::Rectangle<int> oceanArea;
    juce::Rectangle<int> delayCardArea;
    juce::Rectangle<int> reverbCardArea;
    juce::Rectangle<int> leftDrawerArea;
    juce::Rectangle<int> rightDrawerArea;
    juce::Rectangle<int> bottomDrawerArea;

    float smoothInDb  = -60.0f;
    float smoothGrDb  = 0.0f;
    float waterPhase  = 0.0f;

    bool lastMultiTapState = false;
    bool lastSyncState     = true;

    void timerCallback() override;
    void updateVisibilityStates();
    void drawOceanDisplay (juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawDrawerTab (juce::Graphics& g, juce::Rectangle<int> bounds, const juce::String& text, bool isOpen, bool vertical);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MoonsongAudioProcessorEditor)
};