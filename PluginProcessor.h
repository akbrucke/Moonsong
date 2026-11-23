#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>

class MoonsongAudioProcessor : public juce::AudioProcessor
{
public:
    MoonsongAudioProcessor();
    ~MoonsongAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    // Telemetry for UI
    std::atomic<float> meterInDb  { -100.0f };
    std::atomic<float> meterOutDb { -100.0f };
    std::atomic<float> meterGrDb  { 0.0f };

private:
    double currentSampleRate = 48000.0;

    // 1. Optical Compressor (Pre-FX)
    float optoEnvFast = 0.0f;
    float optoEnvSlow = 0.0f;
    float optoGrDb    = 0.0f;

    // 2. Eclipse Mode Stereo Doubler (Post-Comp, Pre-FX)
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> eclipseDelayL { 4800 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> eclipseDelayR { 4800 };
    double eclipsePhaseL = 0.0;
    double eclipsePhaseR = 0.25;

    // 3. Tone + Saturation Chain (Dual-instantiated for clean Pre/Post routing)
    struct ToneSatChain
    {
        juce::dsp::StateVariableTPTFilter<float> hpfL, hpfR;
        juce::dsp::StateVariableTPTFilter<float> lpfL, lpfR;
        juce::dsp::IIR::Filter<float> eqL, eqR;
        float lastHpf = -1.0f, lastLpf = -1.0f;
        float lastEqFreq = -1.0f, lastEqGain = -999.0f;
        int   lastEqType = -1;

        void prepare (const juce::dsp::ProcessSpec& spec);
        void reset();
        void update (double sr, float hpfHz, float lpfHz, float eqFreq, float eqGainDb, int eqType);
        void processSample (float& l, float& r, float satDrive, bool active);
    };

    ToneSatChain dlyTonePre, dlyTonePost;
    ToneSatChain revTonePre, revTonePost;

    // 4. Delay Engine (Mono, Dual-Mono, Ping-Pong, and 5-Tap Matrix)
    static constexpr int kMaxDelaySamples = 48000 * 4;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> dlyLineL { kMaxDelaySamples };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> dlyLineR { kMaxDelaySamples };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> multiTapLine { kMaxDelaySamples };
    float dlyFbL = 0.0f, dlyFbR = 0.0f;
    float multiTapFb = 0.0f;
    std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>, 5> tapDelaySmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> dlyTimeLSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> dlyTimeRSmoothed;

    // 5. Lush Mud-Free FDN Reverb
    static constexpr int kFdnChannels = 8;
    std::array<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear>, 4> revDiffusers {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 4096 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 4096 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 4096 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> { 4096 }
    };
    std::array<juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd>, kFdnChannels> revTanks {
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> { 16384 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> { 16384 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> { 16384 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> { 16384 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> { 16384 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> { 16384 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> { 16384 },
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> { 16384 }
    };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> revPreDelayL { 12000 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> revPreDelayR { 12000 };
    std::array<float, kFdnChannels> revLpState {};
    std::array<float, kFdnChannels> revHpState {};
    double revModPhase = 0.0;

    // 6. Post-FX Stereo Chorus
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> chorusLineL { 4800 };
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> chorusLineR { 4800 };
    double chorusPhase = 0.0;

    // Global Parameter Smoothers
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> eclipseSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> dlyLevelSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> revLevelSmoothed;

    static float noteDivisionToBeats (int divisionIndex);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MoonsongAudioProcessor)
};