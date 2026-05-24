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
    // Avvia il receiver OSC sulla porta 9001
    // SuperCollider mandera' i CC su questa porta via /cc ccNumber normalizedValue
    if (oscReceiver.connect(9001))
        oscReceiver.addListener(this);
}

TESTINGAudioProcessor::~TESTINGAudioProcessor()
{
    // Cleanup OSC
    oscReceiver.removeListener(this);
    oscReceiver.disconnect();
}

//==============================================================================
// Riceve i messaggi OSC da SuperCollider
// SC manda: /cc ccNumber normalizedValue (0.0 - 1.0)
void TESTINGAudioProcessor::oscMessageReceived(const juce::OSCMessage& message)
{
    // Filtra solo i messaggi /cc
    if (!message.getAddressPattern().matches("/cc")) return;

    // Controlla che ci siano almeno 2 argomenti: ccNumber e value
    if (message.size() < 2) return;

    int   cc  = message[0].getInt32();
    float val = juce::jlimit(0.0f, 1.0f, message[1].getFloat32());

    // Mappa ogni CC sul parametro corrispondente
    // I numeri CC dipendono dalla MPK — verificali con il debug in SC
switch (cc)
{
    case 1:  apvts.getParameter("MORPH")->setValueNotifyingHost(val);     break;
    case 2:  apvts.getParameter("WET")->setValueNotifyingHost(val);       break;
    case 3:  apvts.getParameter("DRY")->setValueNotifyingHost(val);       break;
    case 4:  apvts.getParameter("FEEDBACK")->setValueNotifyingHost(val);  break;
    case 5:  apvts.getParameter("DELAYTIME")->setValueNotifyingHost(val); break;
    case 6:  apvts.getParameter("DAMPING")->setValueNotifyingHost(val);   break;
    case 7:  apvts.getParameter("DRIVE")->setValueNotifyingHost(val);     break;
    default: break;
}
}

//==============================================================================
const juce::String TESTINGAudioProcessor::getName() const { return JucePlugin_Name; }

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

double TESTINGAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int    TESTINGAudioProcessor::getNumPrograms()              { return 1; }
int    TESTINGAudioProcessor::getCurrentProgram()           { return 0; }
void   TESTINGAudioProcessor::setCurrentProgram(int)        {}
const  juce::String TESTINGAudioProcessor::getProgramName(int) { return {}; }
void   TESTINGAudioProcessor::changeProgramName(int, const juce::String&) {}

//==============================================================================
void TESTINGAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    stereoDelay.prepare(sampleRate, 2.0f);
    envelopeFollower.prepare(sampleRate, 5.0f, 300.0f);
    modulationEngine.prepare(sampleRate);
}

void TESTINGAudioProcessor::releaseResources() {}

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
    // I CC arrivano ora via OSC da SC — non serve piu' leggere midiMessages
    juce::ignoreUnused(midiMessages);

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Calcola envelope follower sul segnale in ingresso
    float env = 0.0f;
    if (buffer.getNumChannels() > 0)
    {
        const float* input = buffer.getReadPointer(0);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            env = juce::jmax(env, envelopeFollower.process(input[i]));
    }
    envelopeValue.store(env);

    float morph = apvts.getRawParameterValue("MORPH")->load();
    morphValue.store(morph);

    // Scegli tra morph automatico o controllo manuale
    bool morphOn = apvts.getRawParameterValue("MORPHON")->load() > 0.5f;

    if (morphOn)
    {
        // Morph attivo — i parametri vengono calcolati dal motore di modulazione
        // I knob della MPK controllano MORPH via OSC, che guida tutto il resto
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
        // Morph disattivo — i parametri vengono letti direttamente dall'APVTS
        // I knob della MPK controllano ogni parametro individualmente via OSC
        stereoDelay.setWet(*apvts.getRawParameterValue("WET"));
        stereoDelay.setDry(*apvts.getRawParameterValue("DRY"));
        stereoDelay.setFeedback(*apvts.getRawParameterValue("FEEDBACK"));
        stereoDelay.setDelayMs(*apvts.getRawParameterValue("DELAYTIME"));
        stereoDelay.setDampingHz(*apvts.getRawParameterValue("DAMPING"));
        stereoDelay.setPingPong(
            apvts.getRawParameterValue("PINGPONG")->load() > 0.5f);
    }

    // Saturatore — controllabile via OSC con CC 76 (Drive) e CC 77 (Drive On)
    saturator.setEnabled(
        apvts.getRawParameterValue("DRIVEON")->load() > 0.5f);
    saturator.setDrive(
        apvts.getRawParameterValue("DRIVE")->load());

    saturator.process(buffer);
    stereoDelay.process(buffer);
}

//==============================================================================
bool TESTINGAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* TESTINGAudioProcessor::createEditor()
{
    return new TESTINGAudioProcessorEditor(*this);
}

void TESTINGAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void TESTINGAudioProcessor::setStateInformation(const void*, int)   {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TESTINGAudioProcessor();
}

void TESTINGAudioProcessor::set_wet(float val)      { stereoDelay.setWet(val); }
void TESTINGAudioProcessor::set_dry(float val)      { stereoDelay.setDry(val); }
void TESTINGAudioProcessor::set_dampingHz(float val){ stereoDelay.setDampingHz(val); }
void TESTINGAudioProcessor::set_feedback(float val) { stereoDelay.setFeedback(val); }
void TESTINGAudioProcessor::set_delayMs(float val)  { stereoDelay.setDelayMs(val); }
void TESTINGAudioProcessor::set_pingPong(bool val)  { stereoDelay.setPingPong(val); }

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout
TESTINGAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DAMPING",   "Damping",    500.0f,  20000.0f, 8000.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "WET",       "Wet",        0.0f,    1.0f,     0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DRY",       "Dry",        0.0f,    1.0f,     0.70f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "FEEDBACK",  "Feedback",   0.0f,    0.95f,    0.35f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "DRIVEON",   "Drive On",   false));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DRIVE",     "Drive",      1.0f,    20.0f,    1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "DELAYTIME", "Delay Time", 1.0f,    2000.0f,  350.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "MORPH",     "Morph",      0.0f,    1.0f,     0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "PINGPONG",  "Ping Pong",  false));
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "MORPHON",   "Morph On",   true));

    return { params.begin(), params.end() };
}