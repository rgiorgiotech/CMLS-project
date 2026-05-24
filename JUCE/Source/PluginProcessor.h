/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "StereoDelay.h"
#include "EnvelopeFollower.h"
#include "DelayModulationEngine.h"
#include "Saturator.h"
//==============================================================================
/**
*/
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
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
 

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::AudioSampleBuffer dbuf;
    juce::OSCReceiver oscReceiver;
    void oscMessageReceived(const juce::OSCMessage& message) override;
    void oscBundleReceived(const juce::OSCBundle&) override {}

    /*int dw = 0;
    int dr = 1;

    float wet = 0.5f;
    float dry = 0.5f;
    int ds = 50000;*/
    StereoDelay stereoDelay;
    EnvelopeFollower envelopeFollower;
    DelayModulationEngine modulationEngine;
    Saturator saturator;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TESTINGAudioProcessor)
};
