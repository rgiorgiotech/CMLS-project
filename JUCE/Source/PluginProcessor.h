#pragma once

#include <JuceHeader.h>
#include "StereoDelay.h"
#include "EnvelopeFollower.h"
#include "DelayModulationEngine.h"
#include "Saturator.h"
#include "PitchShifter.h"

class TESTINGAudioProcessor : public juce::AudioProcessor,
                               private juce::OSCReceiver::Listener<>
{
public:
    void set_wet(float val);
    void set_dry(float val);
    void set_dampingHz(float val);
    void set_feedback(float val);
    void set_delayMs(float val);
    void set_pingPong(bool val);

    std::atomic<float> envelopeValue{ 0.0f };
    std::atomic<float> morphValue{ 0.0f };

    //==============================================================================
    TESTINGAudioProcessor();
    ~TESTINGAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    //==============================================================================
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

private:
    // OSC — riceve CC e pitch bend da SuperCollider sulla porta 9001
    void oscMessageReceived(const juce::OSCMessage& message) override;
    void oscBundleReceived(const juce::OSCBundle&) override {}

    juce::OSCReceiver     oscReceiver;
    StereoDelay           stereoDelay;
    EnvelopeFollower      envelopeFollower;
    DelayModulationEngine modulationEngine;
    Saturator             saturator;
    PitchShifter          pitchShifter;
    juce::AudioSampleBuffer dbuf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TESTINGAudioProcessor)
};