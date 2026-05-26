#pragma once
#include <JuceHeader.h>

 /**
 * EnvelopeFollower — tracks the amplitude of the signal in real time
 * gives back a value that is smoothed between o.o-1.0 
 * which represents how strong is the signal in taht moment
 * used to modulate the delay parameters in an organic way
 */

class EnvelopeFollower
{
public:
    void prepare(double sampleRate,
        float attackMs = 5.0f,
        float releaseMs = 300.0f)
    {
        sr = sampleRate;
        setAttackMs(attackMs);
        setReleaseMs(releaseMs);
        envelope = 0.0f;
    }

    float process(float inputSample)
    {
        float absInput = std::abs(inputSample);

        if (absInput > envelope)
            envelope = attackCoeff * envelope + (1.0f - attackCoeff) * absInput;
        else
            envelope = releaseCoeff * envelope + (1.0f - releaseCoeff) * absInput;

        return juce::jlimit(0.0f, 1.0f, envelope);
    }

    float getCurrentValue() const { return envelope; }

    void setAttackMs(float ms)
    {
        attackCoeff = std::exp(-1.0f / (float)(sr * ms / 1000.0f));
    }

    void setReleaseMs(float ms)
    {
        releaseCoeff = std::exp(-1.0f / (float)(sr * ms / 1000.0f));
    }

private:
    double sr = 48000.0;
    float  attackCoeff = 0.99f;
    float  releaseCoeff = 0.999f;
    float  envelope = 0.0f;
};