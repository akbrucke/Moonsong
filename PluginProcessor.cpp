#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <algorithm>

namespace
{
    const juce::StringArray kNoteDivisions {
        "1/16", "1/8T", "1/16D", "1/8", "1/4T", "1/8D", "1/4", "1/2T", "1/4D", "1/2", "1/1"
    };
}

MoonsongAudioProcessor::MoonsongAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

float MoonsongAudioProcessor::noteDivisionToBeats (int idx)
{
    constexpr float beats[11] = {
        0.25f,         // 1/16
        1.0f / 3.0f,   // 1/8T
        0.375f,        // 1/16D
        0.5f,          // 1/8
        2.0f / 3.0f,   // 1/4T
        0.75f,         // 1/8D
        1.0f,          // 1/4
        4.0f / 3.0f,   // 1/2T
        1.5f,          // 1/4D
        2.0f,          // 1/2
        4.0f           // 1/1
    };
    return beats[juce::jlimit (0, 10, idx)];
}

juce::AudioProcessorValueTreeState::ParameterLayout MoonsongAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    // --- Optical Compressor (Left Drawer, Pre-FX) ---
    p.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "compOn", 1 }, "Comp Bypass", true));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "compPeak", 1 }, "Peak Reduction",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 35.0f, "%"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "compGain", 1 }, "Comp Makeup",
        juce::NormalisableRange<float> (-6.0f, 18.0f, 0.1f), 2.0f, "dB"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "compMix", 1 }, "Comp Blend",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, "%"));

    // --- Eclipse Doubler & Post Chorus (Right Drawer) ---
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "eclipse", 1 }, "Eclipse Mode",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));
    p.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "chorusOn", 1 }, "Chorus Enable", true));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusRate", 1 }, "Chorus Rate",
        juce::NormalisableRange<float> (0.1f, 3.0f, 0.01f, 0.6f), 0.45f, "Hz"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusDepth", 1 }, "Chorus Depth",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 30.0f, "%"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "chorusMix", 1 }, "Chorus Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 25.0f, "%"));

    // --- Delay Core ---
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "dlyMode", 1 }, "Delay Mode",
        juce::StringArray { "Ping-Pong", "Dual-Mono", "Mono" }, 0));
    p.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "dlyMultiTap", 1 }, "Multi-Tap Mode", false));
    p.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { "dlySync", 1 }, "Delay Sync", true));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlyTimeL", 1 }, "Delay Time L",
        juce::NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.45f), 320.0f, "ms"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlyTimeR", 1 }, "Delay Time R",
        juce::NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.45f), 480.0f, "ms"));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "dlyDivL", 1 }, "Delay Div L", kNoteDivisions, 6)); // 1/4
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "dlyDivR", 1 }, "Delay Div R", kNoteDivisions, 5)); // 1/8D
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlyFeedback", 1 }, "Delay Feedback",
        juce::NormalisableRange<float> (0.0f, 92.0f, 0.1f), 38.0f, "%"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlyLevel", 1 }, "Delay Level",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 55.0f, "%"));

    // --- Multi-Tap Matrix (5 Taps) ---
    const float defTapTimes[5] = { 120.0f, 240.0f, 360.0f, 480.0f, 640.0f };
    const int   defTapDivs[5]  = { 0, 3, 5, 6, 8 }; // 1/16, 1/8, 1/8D, 1/4, 1/4D
    const float defTapPans[5]  = { -0.75f, 0.65f, -0.35f, 0.85f, 0.0f };
    const float defTapGains[5] = { 85.0f, 75.0f, 65.0f, 55.0f, 45.0f };

    for (int t = 0; t < 5; ++t)
    {
        const auto idx = juce::String (t + 1);
        p.push_back (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { "tap" + idx + "On", 1 }, "Tap " + idx + " On", t < 4));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "tap" + idx + "Time", 1 }, "Tap " + idx + " Time",
            juce::NormalisableRange<float> (10.0f, 2000.0f, 1.0f, 0.45f), defTapTimes[t], "ms"));
        p.push_back (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { "tap" + idx + "Div", 1 }, "Tap " + idx + " Div", kNoteDivisions, defTapDivs[t]));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "tap" + idx + "Pan", 1 }, "Tap " + idx + " Pan",
            juce::NormalisableRange<float> (-1.0f, 1.0f, 0.01f), defTapPans[t], ""));
        p.push_back (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "tap" + idx + "Gain", 1 }, "Tap " + idx + " Level",
            juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), defTapGains[t], "%"));
    }

    // --- Reverb Core ---
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revSize", 1 }, "Reverb Size",
        juce::NormalisableRange<float> (10.0f, 100.0f, 0.1f), 68.0f, "%"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revDecay", 1 }, "Reverb Decay",
        juce::NormalisableRange<float> (0.4f, 8.0f, 0.05f, 0.5f), 2.6f, "s"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revPreDelay", 1 }, "Reverb Pre-Delay",
        juce::NormalisableRange<float> (0.0f, 160.0f, 1.0f), 28.0f, "ms"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revDamp", 1 }, "Reverb Damping",
        juce::NormalisableRange<float> (1500.0f, 18000.0f, 10.0f, 0.5f), 6800.0f, "Hz"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revLevel", 1 }, "Reverb Level",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 60.0f, "%"));

    // --- Bottom Drawer: Delay Tone & Sat ---
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "dlyTonePos", 1 }, "Delay Tone Routing",
        juce::StringArray { "Pre-FX", "Post-FX" }, 1));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlyHpf", 1 }, "Delay HPF",
        juce::NormalisableRange<float> (20.0f, 1200.0f, 1.0f, 0.45f), 140.0f, "Hz"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlyLpf", 1 }, "Delay LPF",
        juce::NormalisableRange<float> (1000.0f, 20000.0f, 10.0f, 0.45f), 7500.0f, "Hz"));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "dlyEqType", 1 }, "Delay EQ Shape",
        juce::StringArray { "Broad Bell", "High Shelf" }, 0));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlyEqFreq", 1 }, "Delay EQ Freq",
        juce::NormalisableRange<float> (250.0f, 10000.0f, 5.0f, 0.45f), 1800.0f, "Hz"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlyEqGain", 1 }, "Delay EQ Gain",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 0.0f, "dB"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "dlySat", 1 }, "Delay Saturation",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 18.0f, "%"));

    // --- Bottom Drawer: Reverb Tone & Sat ---
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "revTonePos", 1 }, "Reverb Tone Routing",
        juce::StringArray { "Pre-FX", "Post-FX" }, 1));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revHpf", 1 }, "Reverb HPF",
        juce::NormalisableRange<float> (20.0f, 1200.0f, 1.0f, 0.45f), 180.0f, "Hz"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revLpf", 1 }, "Reverb LPF",
        juce::NormalisableRange<float> (1000.0f, 20000.0f, 10.0f, 0.45f), 9500.0f, "Hz"));
    p.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { "revEqType", 1 }, "Reverb EQ Shape",
        juce::StringArray { "Broad Bell", "High Shelf" }, 1));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revEqFreq", 1 }, "Reverb EQ Freq",
        juce::NormalisableRange<float> (250.0f, 10000.0f, 5.0f, 0.45f), 4500.0f, "Hz"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revEqGain", 1 }, "Reverb EQ Gain",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.1f), 1.5f, "dB"));
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "revSat", 1 }, "Reverb Saturation",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 12.0f, "%"));

    // --- Master Wet/Dry ---
    p.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "mix", 1 }, "Wet/Dry Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, "%"));

    return { p.begin(), p.end() };
}

void MoonsongAudioProcessor::ToneSatChain::prepare (const juce::dsp::ProcessSpec& spec)
{
    hpfL.prepare (spec); hpfR.prepare (spec);
    lpfL.prepare (spec); lpfR.prepare (spec);
    eqL.prepare (spec);  eqR.prepare (spec);

    hpfL.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    hpfR.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    lpfL.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    lpfR.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    reset();
}

void MoonsongAudioProcessor::ToneSatChain::reset()
{
    hpfL.reset(); hpfR.reset();
    lpfL.reset(); lpfR.reset();
    eqL.reset();  eqR.reset();
    lastHpf = lastLpf = lastEqFreq = -1.0f;
    lastEqGain = -999.0f;
    lastEqType = -1;
}

void MoonsongAudioProcessor::ToneSatChain::update (double sr, float hpfHz, float lpfHz,
                                                   float eqFreq, float eqGainDb, int eqType)
{
    if (std::abs (hpfHz - lastHpf) > 0.2f)
    {
        lastHpf = hpfHz;
        hpfL.setCutoffFrequency (hpfHz);
        hpfR.setCutoffFrequency (hpfHz);
    }
    if (std::abs (lpfHz - lastLpf) > 0.5f)
    {
        lastLpf = lpfHz;
        lpfL.setCutoffFrequency (lpfHz);
        lpfR.setCutoffFrequency (lpfHz);
    }
    if (eqType != lastEqType || std::abs (eqFreq - lastEqFreq) > 0.5f || std::abs (eqGainDb - lastEqGain) > 0.02f)
    {
        lastEqType = eqType;
        lastEqFreq = eqFreq;
        lastEqGain = eqGainDb;
        const float gainFactor = juce::Decibels::decibelsToGain (eqGainDb);
        auto coeffs = (eqType == 0)
            ? juce::dsp::IIR::Coefficients<float>::makePeakFilter (sr, eqFreq, 0.72f, gainFactor)
            : juce::dsp::IIR::Coefficients<float>::makeHighShelf  (sr, eqFreq, 0.707f, gainFactor);
        *eqL.coefficients = *coeffs;
        *eqR.coefficients = *coeffs;
    }
}

void MoonsongAudioProcessor::ToneSatChain::processSample (float& l, float& r, float satDrive, bool active)
{
    if (! active)
        return;

    l = hpfL.processSample (0, l);
    r = hpfR.processSample (1, r);

    l = lpfL.processSample (0, l);
    r = lpfR.processSample (1, r);

    if (std::abs (lastEqGain) > 0.02f)
    {
        l = eqL.processSample (l);
        r = eqR.processSample (r);
    }

    if (satDrive > 0.001f)
    {
        const float pre = 1.0f + satDrive * 4.5f;
        const float norm = 1.0f / std::tanh (pre);
        l = std::tanh (l * pre) * norm;
        r = std::tanh (r * pre) * norm;
    }
}

void MoonsongAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec monoSpec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 1 };
    juce::dsp::ProcessSpec stereoSpec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 2 };

    dlyTonePre.prepare (stereoSpec);
    dlyTonePost.prepare (stereoSpec);
    revTonePre.prepare (stereoSpec);
    revTonePost.prepare (stereoSpec);

    eclipseDelayL.prepare (monoSpec); eclipseDelayL.reset();
    eclipseDelayR.prepare (monoSpec); eclipseDelayR.reset();
    eclipsePhaseL = 0.0;
    eclipsePhaseR = 0.25;

    dlyLineL.prepare (monoSpec);     dlyLineL.reset();
    dlyLineR.prepare (monoSpec);     dlyLineR.reset();
    multiTapLine.prepare (monoSpec); multiTapLine.reset();
    dlyFbL = dlyFbR = multiTapFb = 0.0f;

    for (auto& d : revDiffusers) { d.prepare (monoSpec); d.reset(); }
    for (auto& t : revTanks)     { t.prepare (monoSpec); t.reset(); }
    revPreDelayL.prepare (monoSpec); revPreDelayL.reset();
    revPreDelayR.prepare (monoSpec); revPreDelayR.reset();
    revLpState.fill (0.0f);
    revHpState.fill (0.0f);

    chorusLineL.prepare (monoSpec); chorusLineL.reset();
    chorusLineR.prepare (monoSpec); chorusLineR.reset();

    optoEnvFast = optoEnvSlow = optoGrDb = 0.0f;

    const double ramp = 0.025;
    mixSmoothed.reset (sampleRate, ramp);
    eclipseSmoothed.reset (sampleRate, ramp);
    dlyLevelSmoothed.reset (sampleRate, ramp);
    revLevelSmoothed.reset (sampleRate, ramp);
    dlyTimeLSmoothed.reset (sampleRate, 0.06);
    dlyTimeRSmoothed.reset (sampleRate, 0.06);
    for (auto& s : tapDelaySmoothed)
        s.reset (sampleRate, 0.06);
}

void MoonsongAudioProcessor::releaseResources() {}

bool MoonsongAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono()
        || layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo();
}

void MoonsongAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numInCh    = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numInCh == 0 || numSamples == 0)
        return;

    // Host BPM for sync
    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto hostBpm = pos->getBpm())
                if (*hostBpm > 20.0 && *hostBpm < 400.0)
                    bpm = *hostBpm;

    const double secPerBeat = 60.0 / bpm;

    // Fetch parameters
    const bool  compOn      = apvts.getRawParameterValue ("compOn")->load() > 0.5f;
    const float compPeak    = apvts.getRawParameterValue ("compPeak")->load() * 0.01f;
    const float compMakeup  = juce::Decibels::decibelsToGain (apvts.getRawParameterValue ("compGain")->load());
    const float compMix     = apvts.getRawParameterValue ("compMix")->load() * 0.01f;

    const float eclipseAmt  = apvts.getRawParameterValue ("eclipse")->load() * 0.01f;
    const bool  chorusOn    = apvts.getRawParameterValue ("chorusOn")->load() > 0.5f;
    const float chorusRate  = apvts.getRawParameterValue ("chorusRate")->load();
    const float chorusDepth = apvts.getRawParameterValue ("chorusDepth")->load() * 0.01f;
    const float chorusMix   = apvts.getRawParameterValue ("chorusMix")->load() * 0.01f;

    const int   dlyMode     = static_cast<int> (apvts.getRawParameterValue ("dlyMode")->load());
    const bool  multiTapOn  = apvts.getRawParameterValue ("dlyMultiTap")->load() > 0.5f;
    const bool  dlySync     = apvts.getRawParameterValue ("dlySync")->load() > 0.5f;
    const float dlyFbGain   = apvts.getRawParameterValue ("dlyFeedback")->load() * 0.01f;
    const float dlyLevel    = apvts.getRawParameterValue ("dlyLevel")->load() * 0.01f;

    float targetDlyMsL = apvts.getRawParameterValue ("dlyTimeL")->load();
    float targetDlyMsR = apvts.getRawParameterValue ("dlyTimeR")->load();
    if (dlySync)
    {
        const int divL = static_cast<int> (apvts.getRawParameterValue ("dlyDivL")->load());
        const int divR = static_cast<int> (apvts.getRawParameterValue ("dlyDivR")->load());
        targetDlyMsL = static_cast<float> (noteDivisionToBeats (divL) * secPerBeat * 1000.0);
        targetDlyMsR = static_cast<float> (noteDivisionToBeats (divR) * secPerBeat * 1000.0);
    }
    const float maxDelayMs = (static_cast<float> (kMaxDelaySamples - 16) / static_cast<float> (currentSampleRate)) * 1000.0f;
    dlyTimeLSmoothed.setTargetValue (juce::jlimit (10.0f, maxDelayMs, targetDlyMsL));
    dlyTimeRSmoothed.setTargetValue (juce::jlimit (10.0f, maxDelayMs, targetDlyMsR));

    struct TapConfig { bool on; float panL; float panR; float gain; };
    std::array<TapConfig, 5> taps {};
    for (int t = 0; t < 5; ++t)
    {
        const auto idx = juce::String (t + 1);
        taps[static_cast<size_t> (t)].on   = apvts.getRawParameterValue ("tap" + idx + "On")->load() > 0.5f;
        taps[static_cast<size_t> (t)].gain = apvts.getRawParameterValue ("tap" + idx + "Gain")->load() * 0.01f;
        const float pan = apvts.getRawParameterValue ("tap" + idx + "Pan")->load();
        const float angle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        taps[static_cast<size_t> (t)].panL = std::cos (angle);
        taps[static_cast<size_t> (t)].panR = std::sin (angle);

        float tMs = apvts.getRawParameterValue ("tap" + idx + "Time")->load();
        if (dlySync)
        {
            const int div = static_cast<int> (apvts.getRawParameterValue ("tap" + idx + "Div")->load());
            tMs = static_cast<float> (noteDivisionToBeats (div) * secPerBeat * 1000.0);
        }
        tapDelaySmoothed[static_cast<size_t> (t)].setTargetValue (juce::jlimit (10.0f, maxDelayMs, tMs));
    }

    const float revSize     = apvts.getRawParameterValue ("revSize")->load() * 0.01f;
    const float revDecaySec = apvts.getRawParameterValue ("revDecay")->load();
    const float revPreMs    = apvts.getRawParameterValue ("revPreDelay")->load();
    const float revDampHz   = apvts.getRawParameterValue ("revDamp")->load();
    const float revLevel    = apvts.getRawParameterValue ("revLevel")->load() * 0.01f;

    const int   dlyTonePos  = static_cast<int> (apvts.getRawParameterValue ("dlyTonePos")->load());
    const float dlySat      = apvts.getRawParameterValue ("dlySat")->load() * 0.01f;
    dlyTonePre.update  (currentSampleRate,
                        apvts.getRawParameterValue ("dlyHpf")->load(),
                        apvts.getRawParameterValue ("dlyLpf")->load(),
                        apvts.getRawParameterValue ("dlyEqFreq")->load(),
                        apvts.getRawParameterValue ("dlyEqGain")->load(),
                        static_cast<int> (apvts.getRawParameterValue ("dlyEqType")->load()));
    dlyTonePost.update (currentSampleRate,
                        apvts.getRawParameterValue ("dlyHpf")->load(),
                        apvts.getRawParameterValue ("dlyLpf")->load(),
                        apvts.getRawParameterValue ("dlyEqFreq")->load(),
                        apvts.getRawParameterValue ("dlyEqGain")->load(),
                        static_cast<int> (apvts.getRawParameterValue ("dlyEqType")->load()));

    const int   revTonePos  = static_cast<int> (apvts.getRawParameterValue ("revTonePos")->load());
    const float revSat      = apvts.getRawParameterValue ("revSat")->load() * 0.01f;
    revTonePre.update  (currentSampleRate,
                        apvts.getRawParameterValue ("revHpf")->load(),
                        apvts.getRawParameterValue ("revLpf")->load(),
                        apvts.getRawParameterValue ("revEqFreq")->load(),
                        apvts.getRawParameterValue ("revEqGain")->load(),
                        static_cast<int> (apvts.getRawParameterValue ("revEqType")->load()));
    revTonePost.update (currentSampleRate,
                        apvts.getRawParameterValue ("revHpf")->load(),
                        apvts.getRawParameterValue ("revLpf")->load(),
                        apvts.getRawParameterValue ("revEqFreq")->load(),
                        apvts.getRawParameterValue ("revEqGain")->load(),
                        static_cast<int> (apvts.getRawParameterValue ("revEqType")->load()));

    mixSmoothed.setTargetValue (apvts.getRawParameterValue ("mix")->load() * 0.01f);
    eclipseSmoothed.setTargetValue (eclipseAmt);
    dlyLevelSmoothed.setTargetValue (dlyLevel);
    revLevelSmoothed.setTargetValue (revLevel);

    // Precompute FDN constants
    constexpr float baseTankMs[kFdnChannels] = { 29.7f, 34.3f, 39.1f, 43.7f, 48.9f, 54.1f, 59.3f, 64.7f };
    constexpr float diffuserMs[4]            = { 4.7f, 6.3f, 8.9f, 11.7f };
    const float sizeScale = 0.55f + revSize * 0.85f;
    const float dampCoeff = std::exp (-2.0f * juce::MathConstants<float>::pi * revDampHz / static_cast<float> (currentSampleRate));
    // Built-in 135 Hz structural low-cut inside FDN feedback loop to prevent low-mid vocal mud
    const float mudHpCoeff = std::exp (-2.0f * juce::MathConstants<float>::pi * 135.0f / static_cast<float> (currentSampleRate));
    const float preDelaySamples = juce::jlimit (1.0f, 11000.0f, (revPreMs * 0.001f) * static_cast<float> (currentSampleRate));

    // Optical compressor time constants
    const float optoAtk  = std::exp (-1.0f / (0.010f * static_cast<float> (currentSampleRate)));
    const float optoRelF = std::exp (-1.0f / (0.060f * static_cast<float> (currentSampleRate)));
    const float optoRelS = std::exp (-1.0f / (0.650f * static_cast<float> (currentSampleRate)));
    const float optoThreshDb = -6.0f - compPeak * 34.0f; // -6 dB down to -40 dB

    const double eclipseIncL = 0.37 / currentSampleRate;
    const double eclipseIncR = 0.43 / currentSampleRate;
    const double chorusInc   = static_cast<double> (chorusRate) / currentSampleRate;
    const double revModInc   = 0.55 / currentSampleRate;

    float* chL = buffer.getWritePointer (0);
    float* chR = (numInCh > 1) ? buffer.getWritePointer (1) : chL;

    float blockInPeak  = 0.0f;
    float blockOutPeak = 0.0f;
    float maxGrBlock   = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float dryL = chL[i];
        const float dryR = (numInCh > 1) ? chR[i] : dryL;
        blockInPeak = std::max (blockInPeak, std::max (std::abs (dryL), std::abs (dryR)));

        // 1. Optical Compressor (Pre-FX)
        float fxInL = dryL;
        float fxInR = dryR;
        if (compOn)
        {
            const float det = std::max (std::abs (dryL), std::abs (dryR));
            if (det > optoEnvFast)
                optoEnvFast = optoAtk * optoEnvFast + (1.0f - optoAtk) * det;
            else
                optoEnvFast = optoRelF * optoEnvFast + (1.0f - optoRelF) * det;

            if (optoEnvFast > optoEnvSlow)
                optoEnvSlow = optoAtk * optoEnvSlow + (1.0f - optoAtk) * optoEnvFast;
            else
                optoEnvSlow = optoRelS * optoEnvSlow + (1.0f - optoRelS) * optoEnvFast;

            const float envLevel = 0.45f * optoEnvFast + 0.55f * optoEnvSlow;
            const float envDb    = juce::Decibels::gainToDecibels (envLevel + 1.0e-7f);
            float targetGr = 0.0f;
            if (envDb > optoThreshDb)
                targetGr = (envDb - optoThreshDb) * (1.0f - 1.0f / (1.0f + compPeak * 4.5f));

            optoGrDb = 0.92f * optoGrDb + 0.08f * targetGr;
            maxGrBlock = std::max (maxGrBlock, optoGrDb);

            const float compGain = juce::Decibels::decibelsToGain (-optoGrDb) * compMakeup;
            const float compL = dryL * compGain;
            const float compR = dryR * compGain;
            fxInL = dryL + compMix * (compL - dryL);
            fxInR = dryR + compMix * (compR - dryR);
        }
        else
        {
            optoGrDb *= 0.95f;
        }

        // 2. Eclipse Mode Stereo Doubler (Post-Comp, Pre-FX)
        const float ecl = eclipseSmoothed.getNextValue();
        eclipseDelayL.pushSample (0, fxInL);
        eclipseDelayR.pushSample (0, fxInR);

        const float modL = static_cast<float> (std::sin (eclipsePhaseL * juce::MathConstants<double>::twoPi));
        const float modR = static_cast<float> (std::sin (eclipsePhaseR * juce::MathConstants<double>::twoPi));
        eclipsePhaseL += eclipseIncL; if (eclipsePhaseL >= 1.0) eclipsePhaseL -= 1.0;
        eclipsePhaseR += eclipseIncR; if (eclipsePhaseR >= 1.0) eclipsePhaseR -= 1.0;

        if (ecl > 0.001f)
        {
            const float dSmpL = ((11.0f + 2.8f * modL) * 0.001f) * static_cast<float> (currentSampleRate);
            const float dSmpR = ((21.0f + 3.4f * modR) * 0.001f) * static_cast<float> (currentSampleRate);
            const float dblL = eclipseDelayL.popSample (0, dSmpL);
            const float dblR = eclipseDelayR.popSample (0, dSmpR);

            // Subtle stereo spread + Haas doubling
            const float wideL = 0.72f * fxInL + 0.58f * dblR - 0.15f * dblL;
            const float wideR = 0.72f * fxInR + 0.58f * dblL - 0.15f * dblR;
            fxInL = fxInL + ecl * (wideL - fxInL);
            fxInR = fxInR + ecl * (wideR - fxInR);
        }

        // 3. Delay Path
        float dlyInL = fxInL;
        float dlyInR = fxInR;
        dlyTonePre.processSample (dlyInL, dlyInR, dlySat, dlyTonePos == 0);

        const float curDlySmpL = (dlyTimeLSmoothed.getNextValue() * 0.001f) * static_cast<float> (currentSampleRate);
        const float curDlySmpR = (dlyTimeRSmoothed.getNextValue() * 0.001f) * static_cast<float> (currentSampleRate);

        float dlyOutL = 0.0f;
        float dlyOutR = 0.0f;

        if (multiTapOn)
        {
            const float monoIn = 0.5f * (dlyInL + dlyInR);
            multiTapLine.pushSample (0, std::tanh (monoIn + multiTapFb * dlyFbGain));

            float tapSumL = 0.0f;
            float tapSumR = 0.0f;
            float maxTapSmp = 10.0f;

            for (int t = 0; t < 5; ++t)
            {
                const float tSmp = (tapDelaySmoothed[static_cast<size_t> (t)].getNextValue() * 0.001f)
                                   * static_cast<float> (currentSampleRate);
                maxTapSmp = std::max (maxTapSmp, tSmp);
                const auto& cfg = taps[static_cast<size_t> (t)];
                if (cfg.on)
                {
                    const float tapSig = multiTapLine.popSample (0, tSmp, false) * cfg.gain;
                    tapSumL += tapSig * cfg.panL;
                    tapSumR += tapSig * cfg.panR;
                }
            }
            // Feedback taken from the longest tap position
            multiTapFb = multiTapLine.popSample (0, maxTapSmp, true);
            dlyOutL = tapSumL * 0.65f;
            dlyOutR = tapSumR * 0.65f;
        }
        else
        {
            // Advance tap smoothers so they stay warm if toggled
            for (int t = 0; t < 5; ++t)
                tapDelaySmoothed[static_cast<size_t> (t)].getNextValue();

            if (dlyMode == 0) // Ping-Pong
            {
                const float monoSeed = 0.5f * (dlyInL + dlyInR);
                dlyLineL.pushSample (0, std::tanh (monoSeed + dlyFbR * dlyFbGain));
                dlyLineR.pushSample (0, std::tanh (dlyFbL * dlyFbGain));
                dlyOutL = dlyLineL.popSample (0, curDlySmpL);
                dlyOutR = dlyLineR.popSample (0, curDlySmpR);
                dlyFbL = dlyOutL;
                dlyFbR = dlyOutR;
            }
            else if (dlyMode == 1) // Dual-Mono
            {
                dlyLineL.pushSample (0, std::tanh (dlyInL + dlyFbL * dlyFbGain));
                dlyLineR.pushSample (0, std::tanh (dlyInR + dlyFbR * dlyFbGain));
                dlyOutL = dlyLineL.popSample (0, curDlySmpL);
                dlyOutR = dlyLineR.popSample (0, curDlySmpR);
                dlyFbL = dlyOutL;
                dlyFbR = dlyOutR;
            }
            else // Mono
            {
                const float monoSeed = 0.5f * (dlyInL + dlyInR);
                dlyLineL.pushSample (0, std::tanh (monoSeed + dlyFbL * dlyFbGain));
                dlyOutL = dlyLineL.popSample (0, curDlySmpL);
                dlyOutR = dlyOutL;
                dlyFbL = dlyOutL;
                dlyFbR = dlyOutL;
            }
        }

        dlyTonePost.processSample (dlyOutL, dlyOutR, dlySat, dlyTonePos == 1);
        const float curDlyLvl = dlyLevelSmoothed.getNextValue();
        dlyOutL *= curDlyLvl;
        dlyOutR *= curDlyLvl;

        // 4. Lush Mud-Free Reverb Path
        float revInL = fxInL;
        float revInR = fxInR;
        revTonePre.processSample (revInL, revInR, revSat, revTonePos == 0);

        revPreDelayL.pushSample (0, revInL);
        revPreDelayR.pushSample (0, revInR);
        float diffL = revPreDelayL.popSample (0, preDelaySamples);
        float diffR = revPreDelayR.popSample (0, preDelaySamples);

        // 4-stage series all-pass diffusion
        for (int d = 0; d < 4; ++d)
        {
            const float dSmp = (diffuserMs[d] * 0.001f) * static_cast<float> (currentSampleRate);
            const float inSig = (d % 2 == 0) ? diffL : diffR;
            const float bufOut = revDiffusers[static_cast<size_t> (d)].popSample (0, dSmp);
            const float v = inSig + 0.52f * bufOut;
            revDiffusers[static_cast<size_t> (d)].pushSample (0, v);
            const float apOut = bufOut - 0.52f * v;
            if (d % 2 == 0) diffL = apOut;
            else            diffR = apOut;
        }

        // Read 8 FDN tanks with gentle modulation
        revModPhase += revModInc;
        if (revModPhase >= 1.0) revModPhase -= 1.0;
        const float modSin = static_cast<float> (std::sin (revModPhase * juce::MathConstants<double>::twoPi));
        const float modCos = static_cast<float> (std::cos (revModPhase * juce::MathConstants<double>::twoPi));

        std::array<float, kFdnChannels> tankOut {};
        float sumTanks = 0.0f;
        for (int c = 0; c < kFdnChannels; ++c)
        {
            const float modOffset = ((c % 2 == 0) ? modSin : modCos) * (0.45f * 0.001f * static_cast<float> (currentSampleRate));
            const float tSmp = (baseTankMs[c] * sizeScale * 0.001f) * static_cast<float> (currentSampleRate) + modOffset;
            float s = revTanks[static_cast<size_t> (c)].popSample (0, std::max (8.0f, tSmp));

            // RT60 per-channel feedback gain
            const float delaySec = (baseTankMs[c] * sizeScale) * 0.001f;
            const float g = std::pow (10.0f, (-3.0f * delaySec) / std::max (0.25f, revDecaySec));
            s *= juce::jlimit (0.0f, 0.985f, g);

            // High-frequency damping
            revLpState[static_cast<size_t> (c)] = dampCoeff * revLpState[static_cast<size_t> (c)] + (1.0f - dampCoeff) * s;
            s = revLpState[static_cast<size_t> (c)];

            // Anti-mud structural high-pass inside feedback loop
            revHpState[static_cast<size_t> (c)] = mudHpCoeff * revHpState[static_cast<size_t> (c)] + (1.0f - mudHpCoeff) * s;
            s -= revHpState[static_cast<size_t> (c)];

            tankOut[static_cast<size_t> (c)] = s;
            sumTanks += s;
        }

        // Householder feedback mixing matrix: H = I - (2/N) * 11^T
        const float householderScale = 2.0f / static_cast<float> (kFdnChannels);
        for (int c = 0; c < kFdnChannels; ++c)
        {
            const float inject = (c % 2 == 0) ? diffL : diffR;
            const float fb = tankOut[static_cast<size_t> (c)] - householderScale * sumTanks;
            revTanks[static_cast<size_t> (c)].pushSample (0, inject * 0.35f + fb);
        }

        float revOutL = (tankOut[0] + tankOut[2] + tankOut[4] + tankOut[6]) * 0.45f;
        float revOutR = (tankOut[1] + tankOut[3] + tankOut[5] + tankOut[7]) * 0.45f;

        revTonePost.processSample (revOutL, revOutR, revSat, revTonePos == 1);
        const float curRevLvl = revLevelSmoothed.getNextValue();
        revOutL *= curRevLvl;
        revOutR *= curRevLvl;

        // 5. Sum Wet Engines + Subtle Post Stereo Chorus
        float wetL = dlyOutL + revOutL;
        float wetR = dlyOutR + revOutR;

        chorusLineL.pushSample (0, wetL);
        chorusLineR.pushSample (0, wetR);
        chorusPhase += chorusInc;
        if (chorusPhase >= 1.0) chorusPhase -= 1.0;

        if (chorusOn && chorusMix > 0.001f)
        {
            const float cSin = static_cast<float> (std::sin (chorusPhase * juce::MathConstants<double>::twoPi));
            const float cCos = static_cast<float> (std::cos (chorusPhase * juce::MathConstants<double>::twoPi));
            const float depthMs = chorusDepth * 3.2f;
            const float cSmpL = ((6.5f + depthMs * cSin) * 0.001f) * static_cast<float> (currentSampleRate);
            const float cSmpR = ((7.2f + depthMs * cCos) * 0.001f) * static_cast<float> (currentSampleRate);

            const float chWetL = chorusLineL.popSample (0, cSmpL);
            const float chWetR = chorusLineR.popSample (0, cSmpR);
            wetL += chorusMix * 0.55f * chWetR;
            wetR += chorusMix * 0.55f * chWetL;
        }

        // 6. Global Wet/Dry Mix
        const float m = mixSmoothed.getNextValue();
        const float outL = dryL * (1.0f - m) + wetL;
        const float outR = dryR * (1.0f - m) + wetR;

        chL[i] = outL;
        if (numInCh > 1)
            chR[i] = outR;

        blockOutPeak = std::max (blockOutPeak, std::max (std::abs (outL), std::abs (outR)));
    }

    meterInDb.store  (juce::Decibels::gainToDecibels (blockInPeak, -100.0f));
    meterOutDb.store (juce::Decibels::gainToDecibels (blockOutPeak, -100.0f));
    meterGrDb.store  (maxGrBlock);
}

juce::AudioProcessorEditor* MoonsongAudioProcessor::createEditor()
{
    return new MoonsongAudioProcessorEditor (*this);
}

void MoonsongAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MoonsongAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MoonsongAudioProcessor();
}