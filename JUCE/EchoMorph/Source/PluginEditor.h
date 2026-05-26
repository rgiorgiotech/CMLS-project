#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpectrumAnalyzer.h"
#include "TiramisuLookAndFeel.h"

class TESTINGAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    TESTINGAudioProcessorEditor(TESTINGAudioProcessor&);
    ~TESTINGAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void pushAudioBufferToVisuals(const juce::AudioBuffer<float>& buffer);

private:
    void timerCallback() override;

    // Funzioni grafiche decorative
    void drawCreamPiping(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawCoffeeBeanBg(juce::Graphics& g, float cx, float cy, float r, float angle);
    void drawCreamSwirl(juce::Graphics& g, float cx, float cy, float r);

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // Look and Feel tiramisù
    TiramisuLookAndFeel laf;
    PitchBeanLookAndFeel pitchLaf;

    // Visualizer
    SpectrumAnalyzer spectrumAnalyzer;

    // Labels
    juce::Label wetLabel;
    juce::Label dryLabel;
    juce::Label timeLabel;
    juce::Label feedbackLabel;
    juce::Label dampingLabel;
    juce::Label driveLabel;
    juce::Label morphLabel;
    juce::Label pitchLabel;

    // Slider / knob
    juce::Slider morphSlider;
    juce::Slider wetSlider;
    juce::Slider drySlider;
    juce::Slider timeSlider;
    juce::Slider feedbackSlider;
    juce::Slider dampingSlider;
    juce::Slider driveSlider;
    juce::Slider pitchSlider;

    // Bottoni
    juce::ToggleButton pingPongButton;
    juce::ToggleButton morphOnButton;
    juce::ToggleButton driveOnButton;

    // Attachments slider
    std::unique_ptr<SliderAttachment> morphAttachment;
    std::unique_ptr<SliderAttachment> wetAttachment;
    std::unique_ptr<SliderAttachment> dryAttachment;
    std::unique_ptr<SliderAttachment> timeAttachment;
    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> dampingAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;
    std::unique_ptr<SliderAttachment> pitchAttachment;

    // Attachments bottoni
    std::unique_ptr<ButtonAttachment> pingPongAttachment;
    std::unique_ptr<ButtonAttachment> morphOnAttachment;
    std::unique_ptr<ButtonAttachment> driveOnAttachment;

    float displayEnvelope = 0.0f;
    float displayMorph = 0.0f;

    TESTINGAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TESTINGAudioProcessorEditor)
};