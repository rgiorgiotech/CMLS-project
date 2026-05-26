#pragma once
#include <JuceHeader.h>

 /**
 * LFOMatrix — low frequency oscillator with multiple sinewaves.
 * used to automaticaly model the delay parameters in time,
 *  creating evolving sounds without manual intervention.
 */
class LFOMatrix
{
public:
    enum Shape { Sine, Triangle, SampleAndHold };

    void prepare(double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0f;
    }

    /**
     * Restituisce un valore tra -1.0 e +1.0.
     * Chiama questo una volta per blocco audio, non per campione.
     */
    float tick()
    {
        float output = 0.0f;

        switch (shape)
        {
        case Sine:
            output = std::sin(phase * juce::MathConstants<float>::twoPi);
            break;

        case Triangle:
            // Onda triangolare: sale da -1 a +1 nella prima metà,
            // scende da +1 a -1 nella seconda metà
            output = (phase < 0.5f)
                ? (phase * 4.0f - 1.0f)
                : (3.0f - phase * 4.0f);
            break;

        case SampleAndHold:
            // Genera un nuovo valore casuale ogni ciclo
            if (phase < lastPhase)
                currentRandom = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            output = currentRandom;
            break;
        }

        lastPhase = phase;

        // Avanza la fase in base alla frequenza
        phase += frequency / (float)sr;
        if (phase >= 1.0f) phase -= 1.0f;

        return output;
    }

    // Normalizza l'output da -1/+1 a 0/1 per mapparlo su parametri positivi
    float tickNormalized() { return (tick() + 1.0f) * 0.5f; }

    float frequency = 0.5f; // Hz
    Shape shape = Sine;
    float depth = 0.3f; // 0.0 = nessuna modulazione, 1.0 = modulazione massima

private:
    double sr = 48000.0;
    float  phase = 0.0f;
    float  lastPhase = 0.0f;
    float  currentRandom = 0.0f;
};