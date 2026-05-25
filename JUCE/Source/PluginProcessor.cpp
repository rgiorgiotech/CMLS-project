#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TESTINGAudioProcessor::TESTINGAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
        .withInput("Input",  juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
    apvts(*this, nullptr, "Parameters", createParameters())
#endif
{
    // SuperCollider manda i controlli OSC su questa porta:
    // /cc ccNumber normalizedValue
    if (oscReceiver.connect(9001))
        oscReceiver.addListener(this);
}

TESTINGAudioProcessor::~TESTINGAudioProcessor()
{
    oscReceiver.removeListener(this);
    oscReceiver.disconnect();
}

//==============================================================================
void TESTINGAudioProcessor::oscMessageReceived(const juce::OSCMessage& message)
{
    if (!message.getAddressPattern().matches("/cc"))
        return;

    if (message.size() < 2)
        return;

    if (!message[0].isInt32() || !message[1].isFloat32())
        return;

    const int cc = message[0].getInt32();
    const float val = juce::jlimit(0.0f, 1.0f, message[1].getFloat32());

    auto setParam = [&] (const juce::String& paramID)
    {
        if (auto* p = apvts.getParameter(paramID))
            p->setValueNotifyingHost(val);
    };

    switch (cc)
    {
        case 1: setParam("MORPH");     break;
        case 2: setParam("WET");       break;
        case 3: setParam("DRY");       break;
        case 4: setParam("FEEDBACK");  break;
        case 5: setParam("DELAYTIME"); break;
        case 6: setParam("DAMPING");   break;
        case 7: setParam("DRIVE");     break;
        default: break;
    }
}

//==============================================================================
const juce::String TESTINGAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TESTINGAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool TESTINGAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool TESTINGAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double TESTINGAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TESTINGAudioProcessor::getNumPrograms()
{
    return 1;
}

int TESTINGAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TESTINGAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String TESTINGAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void TESTINGAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void TESTINGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    stereoDelay.prepare(sampleRate, 2.0f);
    envelopeFollower.prepare(sampleRate, 5.0f, 300.0f);
    modulationEngine.prepare(sampleRate);
}

void TESTINGAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TESTINGAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

//==============================================================================
void TESTINGAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels  = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    float env = 0.0f;

    if (buffer.getNumChannels() > 0)
    {
        const float* input = buffer.getReadPointer(0);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            env = juce::jmax(env, envelopeFollower.process(input[i]));
    }

    envelopeValue.store(env);

    const float morph = apvts.getRawParameterValue("MORPH")->load();
    morphValue.store(morph);

    const bool morphOn = apvts.getRawParameterValue("MORPHON")->load() > 0.5f;

    if (morphOn)
    {
        auto modParams = modulationEngine.compute(morph, env);

        stereoDelay.setWet(modParams.wet);
        stereoDelay.setDry(modParams.dry);
        stereoDelay.setFeedback(modParams.feedback);
        stereoDelay.setDelayMs(modParams.delayMs);
        stereoDelay.setDampingHz(modParams.dampingHz);
        stereoDelay.setPingPong(modParams.pingPong);
    }
    else
    {
        stereoDelay.setWet(apvts.getRawParameterValue("WET")->load());
        stereoDelay.setDry(apvts.getRawParameterValue("DRY")->load());
        stereoDelay.setFeedback(apvts.getRawParameterValue("FEEDBACK")->load());
        stereoDelay.setDelayMs(apvts.getRawParameterValue("DELAYTIME")->load());
        stereoDelay.setDampingHz(apvts.getRawParameterValue("DAMPING")->load());
        stereoDelay.setPingPong(apvts.getRawParameterValue("PINGPONG")->load() > 0.5f);
    }

    saturator.setEnabled(apvts.getRawParameterValue("DRIVEON")->load() > 0.5f);
    saturator.setDrive(apvts.getRawParameterValue("DRIVE")->load());

    saturator.process(buffer);
    stereoDelay.process(buffer);

    // Alimenta lo spectrum analyzer con il segnale già processato.
    if (auto* editor = dynamic_cast<TESTINGAudioProcessorEditor*>(getActiveEditor()))
        editor->pushAudioBufferToVisuals(buffer);
}

//==============================================================================
bool TESTINGAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TESTINGAudioProcessor::createEditor()
{
    return new TESTINGAudioProcessorEditor(*this);
}

void TESTINGAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ignoreUnused(destData);
}

void TESTINGAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TESTINGAudioProcessor();
}

//==============================================================================
void TESTINGAudioProcessor::set_wet(float val)       { stereoDelay.setWet(val); }
void TESTINGAudioProcessor::set_dry(float val)       { stereoDelay.setDry(val); }
void TESTINGAudioProcessor::set_dampingHz(float val) { stereoDelay.setDampingHz(val); }
void TESTINGAudioProcessor::set_feedback(float val)  { stereoDelay.setFeedback(val); }
void TESTINGAudioProcessor::set_delayMs(float val)   { stereoDelay.setDelayMs(val); }
void TESTINGAudioProcessor::set_pingPong(bool val)   { stereoDelay.setPingPong(val); }

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
TESTINGAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DAMPING",
        "Damping",
        juce::NormalisableRange<float>(500.0f, 20000.0f, 1.0f, 0.35f),
        8000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "WET",
        "Wet",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.35f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DRY",
        "Dry",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.70f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FEEDBACK",
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.001f),
        0.35f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "DRIVEON",
        "Drive On",
        false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DRIVE",
        "Drive",
        juce::NormalisableRange<float>(1.0f, 20.0f, 0.01f),
        1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DELAYTIME",
        "Delay Time",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f, 0.45f),
        350.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "MORPH",
        "Morph",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "PINGPONG",
        "Ping Pong",
        false));

    // Di default OFF: così i knob manuali non vengono sovrascritti dal morph.
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "MORPHON",
        "Morph On",
        false));

    return { params.begin(), params.end() };
}
