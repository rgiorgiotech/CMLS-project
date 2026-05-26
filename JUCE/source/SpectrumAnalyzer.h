#pragma once
#include <JuceHeader.h>

/**
 * SpectrumAnalyzer

 *
 * IMPORTANTE: requires JUCE juce_dsp module.
 */
class SpectrumAnalyzer : public juce::Component,
                         private juce::Timer
{
public:
    SpectrumAnalyzer()
        : fft(fftOrder),
          window(fftSize, juce::dsp::WindowingFunction<float>::hann)
    {
        setOpaque(true);
        startTimerHz(30);
    }

    ~SpectrumAnalyzer() override
    {
        stopTimer();
    }

    void pushSamples(const float* samples, int numSamples)
    {
        if (samples == nullptr || numSamples <= 0)
            return;

        for (int i = 0; i < numSamples; ++i)
            pushNextSampleIntoFifo(samples[i]);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        juce::ColourGradient bg(
            juce::Colour(0xff080810), 0.0f, 0.0f,
            juce::Colour(0xff151527), 0.0f, bounds.getBottom(),
            false);
        g.setGradientFill(bg);
        g.fillRoundedRectangle(bounds, 10.0f);

        g.setColour(juce::Colour(0x44ffffff));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 10.0f, 1.0f);

        drawGrid(g, bounds);
        drawSpectrum(g, bounds);
        drawLabels(g, bounds);
    }

private:
    static constexpr int fftOrder = 11;
    static constexpr int fftSize  = 1 << fftOrder; // 2048
    static constexpr int scopeSize = 128;

    juce::dsp::FFT fft;
    juce::dsp::WindowingFunction<float> window;

    std::array<float, fftSize> fifo {};
    std::array<float, fftSize * 2> fftData {};
    std::array<float, scopeSize> scopeData {};

    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    void pushNextSampleIntoFifo(float sample)
    {
        if (fifoIndex == fftSize)
        {
            if (!nextFFTBlockReady)
            {
                std::fill(fftData.begin(), fftData.end(), 0.0f);
                std::copy(fifo.begin(), fifo.end(), fftData.begin());
                nextFFTBlockReady = true;
            }

            fifoIndex = 0;
        }

        fifo[(size_t) fifoIndex++] = sample;
    }

    void timerCallback() override
    {
        if (!nextFFTBlockReady)
            return;

        window.multiplyWithWindowingTable(fftData.data(), fftSize);
        fft.performFrequencyOnlyForwardTransform(fftData.data());

        auto mindB = -80.0f;
        auto maxdB = 0.0f;

        for (int i = 0; i < scopeSize; ++i)
        {
            // logaritmic map
            auto skewedProportionX = 1.0f - std::exp(std::log(1.0f - (float)i / (float)scopeSize) * 0.2f);
            auto fftDataIndex = juce::jlimit(0, fftSize / 2,
                (int) juce::jmap(skewedProportionX, 0.0f, 1.0f, 0.0f, (float) fftSize / 2.0f));

            auto level = juce::Decibels::gainToDecibels(fftData[(size_t) fftDataIndex])
                         - juce::Decibels::gainToDecibels((float) fftSize);

            auto normalized = juce::jmap(level, mindB, maxdB, 0.0f, 1.0f);
            normalized = juce::jlimit(0.0f, 1.0f, normalized);

            // Smoothing 
            scopeData[(size_t) i] = scopeData[(size_t) i] * 0.82f + normalized * 0.18f;
        }

        nextFFTBlockReady = false;
        repaint();
    }

    void drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setColour(juce::Colour(0xff24243a));

        for (int i = 1; i < 4; ++i)
        {
            auto y = bounds.getY() + bounds.getHeight() * (float)i / 4.0f;
            g.drawHorizontalLine((int)y, bounds.getX(), bounds.getRight());
        }

        for (int i = 1; i < 6; ++i)
        {
            auto x = bounds.getX() + bounds.getWidth() * (float)i / 6.0f;
            g.drawVerticalLine((int)x, bounds.getY(), bounds.getBottom());
        }
    }

    void drawSpectrum(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        juce::Path filled;
        juce::Path line;

        auto left   = bounds.getX() + 8.0f;
        auto right  = bounds.getRight() - 8.0f;
        auto top    = bounds.getY() + 8.0f;
        auto bottom = bounds.getBottom() - 8.0f;
        auto width  = right - left;
        auto height = bottom - top;

        filled.startNewSubPath(left, bottom);

        for (int i = 0; i < scopeSize; ++i)
        {
            auto x = left + width * (float)i / (float)(scopeSize - 1);
            auto y = juce::jmap(scopeData[(size_t) i], 0.0f, 1.0f, bottom, top);

            if (i == 0)
                line.startNewSubPath(x, y);
            else
                line.lineTo(x, y);

            filled.lineTo(x, y);
        }

        filled.lineTo(right, bottom);
        filled.closeSubPath();

        juce::ColourGradient fillGradient(
            juce::Colour(0xaa8f5cff), left, top,
            juce::Colour(0x201de6ff), left, bottom,
            false);
        g.setGradientFill(fillGradient);
        g.fillPath(filled);

        g.setColour(juce::Colour(0xffc9b6ff));
        g.strokePath(line, juce::PathStrokeType(2.0f));

        // Glow 
        g.setColour(juce::Colour(0x558f5cff));
        g.strokePath(line, juce::PathStrokeType(5.0f));
    }

    void drawLabels(juce::Graphics& g, juce::Rectangle<float> bounds)
    {
        g.setFont(juce::FontOptions(9.0f));
        g.setColour(juce::Colour(0xff74749a));

        g.drawText("LOW",  (int)bounds.getX() + 8,  (int)bounds.getBottom() - 16, 40, 12, juce::Justification::left);
        g.drawText("MID",  (int)bounds.getCentreX() - 20, (int)bounds.getBottom() - 16, 40, 12, juce::Justification::centred);
        g.drawText("HIGH", (int)bounds.getRight() - 48, (int)bounds.getBottom() - 16, 40, 12, juce::Justification::right);
    }
};
