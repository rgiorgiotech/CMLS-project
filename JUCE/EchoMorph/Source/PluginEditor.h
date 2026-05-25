#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "SpectrumAnalyzer.h"

class TESTINGAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    TESTINGAudioProcessorEditor(TESTINGAudioProcessor&);
    ~TESTINGAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Il processor manda qui il buffer audio già processato.
    void pushAudioBufferToVisuals(const juce::AudioBuffer<float>& buffer);

private:
    void timerCallback() override;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    SpectrumAnalyzer spectrumAnalyzer;

    juce::Label wetLabel, dryLabel, timeLabel, feedbackLabel;
    juce::Label dampingLabel, driveLabel, morphLabel;

    juce::Slider morphSlider;
    juce::Slider wetSlider, drySlider, timeSlider;
    juce::Slider feedbackSlider, dampingSlider, driveSlider;

    juce::ToggleButton pingPongButton, morphOnButton, driveOnButton;

    std::unique_ptr<SliderAttachment> morphAttachment;
    std::unique_ptr<SliderAttachment> wetAttachment;
    std::unique_ptr<SliderAttachment> dryAttachment;
    std::unique_ptr<SliderAttachment> timeAttachment;
    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> dampingAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;

    std::unique_ptr<ButtonAttachment> pingPongAttachment;
    std::unique_ptr<ButtonAttachment> morphOnAttachment;
    std::unique_ptr<ButtonAttachment> driveOnAttachment;

    float displayEnvelope = 0.0f;
    float displayMorph = 0.0f;

    TESTINGAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TESTINGAudioProcessorEditor)
};
