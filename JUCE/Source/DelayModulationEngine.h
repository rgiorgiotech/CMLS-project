#pragma once
#include <JuceHeader.h>
#include "LFOMatrix.h"

struct DelayParameters
{
    float wet = 0.35f;
    float dry = 0.70f;
    float feedback = 0.35f;
    float delayMs = 350.0f;
    float dampingHz = 8000.0f;
    bool pingPong = false;
};

class DelayModulationEngine
{
public:
    void prepare(double sampleRate)
    {
        lfoDelay.prepare(sampleRate);
        lfoDelay.frequency = 0.2f;
        lfoDelay.shape = LFOMatrix::Sine;
        lfoDelay.depth = 0.25f;

        lfoDamping.prepare(sampleRate);
        lfoDamping.frequency = 0.4f;
        lfoDamping.shape = LFOMatrix::Triangle;
        lfoDamping.depth = 0.2f;
    }

    DelayParameters compute(float morph, float env)
    {
        DelayParameters p;

        auto lerp = [](float a, float b, float t)
            {
                return a + t * (b - a);
            };

        p.delayMs = lerp(700.0f, 25.0f, morph);
        p.feedback = lerp(0.30f, 0.93f, morph);
        p.dampingHz = lerp(1200.0f, 14000.0f, morph);
        p.wet = lerp(0.35f, 0.95f, morph);
        p.dry = lerp(0.80f, 0.20f, morph);

        float delayLfo = lfoDelay.tickNormalized();
        float delayDepth = lerp(0.05f, 0.85f, morph);
        p.delayMs *= 1.0f + delayLfo * delayDepth - delayDepth * 0.5f;

        float dampingLfo = lfoDamping.tickNormalized();
        float dampingDepth = lerp(0.10f, 0.50f, morph);
        p.dampingHz *= 1.0f + dampingLfo * dampingDepth;

        p.feedback += env * lerp(0.0f, 0.15f, morph);

        p.delayMs = juce::jlimit(1.0f, 2000.0f, p.delayMs);
        p.feedback = juce::jlimit(0.0f, 0.94f, p.feedback);
        p.dampingHz = juce::jlimit(500.0f, 20000.0f, p.dampingHz);

        p.pingPong = morph > 0.45f;
        return p;
    }

private:
    LFOMatrix lfoDelay;
    LFOMatrix lfoDamping;
};