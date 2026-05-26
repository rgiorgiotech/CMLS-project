#pragma once
#include <JuceHeader.h>

/**
 * PitchShifter 
 *
 *
 * pitchRatio = 1.0  → no shift
 * pitchRatio = 2.0  → ottava up
 * pitchRatio = 0.5  → octave down
 *
 * 
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
        readHead2 = (float)(bufSize / 2); 
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
        
        float total = getTotalRatio();
        if (std::abs(total - 1.0f) < 0.0003f)
            return;

        int numSamples  = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        float* left  = numChannels > 0 ? buffer.getWritePointer(0) : nullptr;
        float* right = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
           
            bufferL[writeHead] = left  ? left[i]  : 0.0f;
            bufferR[writeHead] = right ? right[i] : 0.0f;

           
            float out1L = readInterp(bufferL, readHead1);
            float out1R = readInterp(bufferR, readHead1);

           
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

           
            readHead1 = std::fmod(readHead1 + total, (float)bufSize);
            readHead2 = std::fmod(readHead2 + total, (float)bufSize);

          
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
    float pitchRatio    = 1.0f; 
    float pitchCentsRatio = 1.0f; 
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