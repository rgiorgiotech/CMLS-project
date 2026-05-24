/*
  ==============================================================================

    StereoDelay.h
    Created: 19 May 2026 10:33:22am
    Author:  neble

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class StereoDelay
{
public:
    void setDampingHz(float val);
    void prepare(double sampleRate, float maxDelaySeconds);

    void process(juce::AudioBuffer<float>& buffer);

    void setWet(float val);
    void setDry(float val);
    void setFeedback(float val);
    void setDelayMs(float val);
    void setPingPong(bool val);

private:
    float dampingHz = 8000.0f;

    juce::SmoothedValue<float> dampedL = 0.0f;
    juce::SmoothedValue<float> dampedR = 0.0f;

    float filterStateL = 0.0f;
    float filterStateR = 0.0f;

    juce::AudioSampleBuffer delayBuffer;

    double currentSampleRate = 44100.0;

    int writeIndex = 0;

    float wet = 0.35f;
    float dry = 0.70f;
    float feedback = 0.35f;
    float delayMs = 350.0f;

    bool pingPong = false;
};