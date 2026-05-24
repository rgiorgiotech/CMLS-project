#pragma once
#include <JuceHeader.h>

class Saturator
{
public:
    void setEnabled(bool shouldBeEnabled)
    {
        enabled = shouldBeEnabled;
    }

    void setDrive(float newDrive)
    {
        drive = juce::jlimit(1.0f, 20.0f, newDrive);
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (!enabled)
            return;

        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        const float driveNow = drive;
        const float compensation = 1.0f / std::tanh(driveNow);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* data = buffer.getWritePointer(ch);

            for (int i = 0; i < numSamples; ++i)
            {
                float x = data[i] * driveNow;
                data[i] = std::tanh(x) * compensation;
            }
        }
    }

private:
    bool enabled = false;
    float drive = 1.0f;
};