#pragma once
#include <JuceHeader.h>

/**
 * PitchShifter — pitch shifter granulare a doppia testina.
 *
 * Usa due read head con crossfade per evitare artefatti al reset.
 * pitchRatio = 1.0  → nessuno shift
 * pitchRatio = 2.0  → ottava sopra
 * pitchRatio = 0.5  → ottava sotto
 *
 * Chiamato su ogni canale separatamente in processBlock.
 */
class PitchShifter
{
public:
    void prepare(double sampleRate)
    {
        sr = sampleRate;
        bufferL.assign(bufSize, 0.0f);
        bufferR.assign(bufSize, 0.0f);
        writeHead = 0;
        readHead1 = 0.0f;
        readHead2 = (float)(bufSize / 2); // seconda testina sfasata di meta' buffer
        crossfade  = 0.0f;
    }

    // Imposta il pitch in semitoni (es. +7 = quinta, -12 = ottava sotto)
    void setPitchSemitones(float semitones)
    {
        pitchRatio = std::pow(2.0f, semitones / 12.0f);
    }

    // Imposta il fine tuning in cents (-100 = semitono sotto, +100 = semitono sopra)
    void setPitchCents(float cents)
    {
        pitchCentsRatio = std::pow(2.0f, cents / 1200.0f);
    }

    float getTotalRatio() const { return pitchRatio * pitchCentsRatio; }

    void process(juce::AudioBuffer<float>& buffer)
    {
        // Bypass se il pitch è neutro (entro 0.5 cents di tolleranza)
        float total = getTotalRatio();
        if (std::abs(total - 1.0f) < 0.0003f)
            return;

        int numSamples  = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        float* left  = numChannels > 0 ? buffer.getWritePointer(0) : nullptr;
        float* right = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            // Scrivi nel buffer circolare
            bufferL[writeHead] = left  ? left[i]  : 0.0f;
            bufferR[writeHead] = right ? right[i] : 0.0f;

            // Leggi con interpolazione lineare dalla testina 1
            float out1L = readInterp(bufferL, readHead1);
            float out1R = readInterp(bufferR, readHead1);

            // Leggi con interpolazione lineare dalla testina 2
            float out2L = readInterp(bufferL, readHead2);
            float out2R = readInterp(bufferR, readHead2);

            // Calcola il crossfade in base alla distanza dal write head
            float dist1 = std::fmod((writeHead - readHead1 + bufSize), (float)bufSize) / bufSize;
            float dist2 = std::fmod((writeHead - readHead2 + bufSize), (float)bufSize) / bufSize;

            // Fade sinusoidale — evita artefatti al punto di reset
            float fade1 = std::sin(juce::MathConstants<float>::pi * dist1);
            float fade2 = std::sin(juce::MathConstants<float>::pi * dist2);

            float norm = fade1 + fade2 + 0.0001f;
            fade1 /= norm;
            fade2 /= norm;

            if (left)  left[i]  = out1L * fade1 + out2L * fade2;
            if (right) right[i] = out1R * fade1 + out2R * fade2;

            // Avanza le testine di lettura al ritmo del pitch ratio
            readHead1 = std::fmod(readHead1 + total, (float)bufSize);
            readHead2 = std::fmod(readHead2 + total, (float)bufSize);

            // Avanza la testina di scrittura
            writeHead = (writeHead + 1) % bufSize;
        }
    }

private:
    static constexpr int bufSize = 8192; // ~170ms a 48kHz

    std::vector<float> bufferL, bufferR;
    int   writeHead     = 0;
    float readHead1     = 0.0f;
    float readHead2     = 0.0f;
    float crossfade     = 0.0f;
    float pitchRatio    = 1.0f; // semitoni coarse
    float pitchCentsRatio = 1.0f; // fine (joystick)
    double sr           = 48000.0;

    // Interpolazione lineare nel buffer circolare
    float readInterp(const std::vector<float>& buf, float pos) const
    {
        int i0 = (int)pos % bufSize;
        int i1 = (i0 + 1) % bufSize;
        float frac = pos - (int)pos;
        return buf[i0] * (1.0f - frac) + buf[i1] * frac;
    }
};