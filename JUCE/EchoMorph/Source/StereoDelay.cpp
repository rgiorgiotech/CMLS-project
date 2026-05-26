/*
  ==============================================================================
    StereoDelay.cpp
  ==============================================================================
*/

#include "StereoDelay.h"

void StereoDelay::prepare(double sampleRate, float maxDelaySeconds)
{
    currentSampleRate = sampleRate;

    const int maxDelaySamples =
        (int)(maxDelaySeconds * sampleRate);

    delayBuffer.setSize(2, maxDelaySamples);

    delayBuffer.clear();

    writeIndex = 0;
    filterStateL = 0.0f;
    filterStateR = 0.0f;
}

void StereoDelay::setWet(float val)
{
    wet = val;
}

void StereoDelay::setDry(float val)
{
    dry = val;
}

void StereoDelay::setFeedback(float val)
{
    feedback = val;
}

void StereoDelay::setDelayMs(float val)
{
    delayMs = val;
}

void StereoDelay::setPingPong(bool val)
{
    pingPong = val;
}

void StereoDelay::process(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();

    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0)
        return;

    const int delayBufferSize =
        delayBuffer.getNumSamples();

    const float dryNow = dry;

    const float wetNow = wet;

    const float feedbackNow =
        juce::jlimit(0.0f, 0.95f, feedback);

    const float delayMsNow =
        juce::jlimit(1.0f, 2000.0f, delayMs);

    const int delaySamples =
        juce::jlimit(
            1,
            delayBufferSize - 1,
            (int)((delayMsNow / 1000.0f)
                * (float)currentSampleRate)
        );

    int readIndex = writeIndex - delaySamples;

    if (readIndex < 0)
        readIndex += delayBufferSize;

    float* outL = buffer.getWritePointer(0);

    float* outR =
        (numChannels > 1)
        ? buffer.getWritePointer(1)
        : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        const float inL = outL[i];

        const float inR =
            (outR != nullptr)
            ? outR[i]
            : inL;

        const float delayedL =
            delayBuffer.getSample(0, readIndex);

        const float delayedR =
            delayBuffer.getSample(1, readIndex);

        float wetL;
        float wetR;

        if (pingPong)
        {
            wetL = delayedR;
            wetR = delayedL;
        }
        else
        {
            wetL = delayedL;
            wetR = delayedR;
        }

        outL[i] =
            dryNow * inL
            + wetNow * wetL;

        if (outR != nullptr)
        {
            outR[i] =
                dryNow * inR
                + wetNow * wetR;
        }

        float feedbackInputL;
        float feedbackInputR;

        if (pingPong)
        {
            // Mono-compatible ping-pong:
            // new input enters only the left delay line.
            // feedback alternates between left and right.
            feedbackInputL = inL + feedbackNow * delayedR;
            feedbackInputR = feedbackNow * delayedL;
        }
        else
        {
            feedbackInputL = inL + feedbackNow * delayedL;
            feedbackInputR = inR + feedbackNow * delayedR;
        }

        const float dampingNow =
            juce::jlimit(500.0f, 20000.0f, dampingHz);

        const float x =
            juce::jlimit(
                0.0f,
                1.0f,
                (float)(2.0 * juce::MathConstants<double>::pi * dampingNow / currentSampleRate)
            );

        filterStateL = filterStateL + x * (feedbackInputL - filterStateL);
        filterStateR = filterStateR + x * (feedbackInputR - filterStateR);

        delayBuffer.setSample(0, writeIndex, filterStateL);
        delayBuffer.setSample(1, writeIndex, filterStateR);

        writeIndex++;

        if (writeIndex >= delayBufferSize)
            writeIndex = 0;

        readIndex++;

        if (readIndex >= delayBufferSize)
            readIndex = 0;
    }
}
void StereoDelay::setDampingHz(float val)
{
    dampingHz = val;
}