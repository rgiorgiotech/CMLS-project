/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class TESTINGAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    TESTINGAudioProcessorEditor (TESTINGAudioProcessor&);
    ~TESTINGAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    juce::Slider dampingSlider;
    juce::Label dampingLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    juce::Slider morphSlider;
    juce::Label morphLabel;

    std::unique_ptr<SliderAttachment> morphAttachment;
    std::unique_ptr<SliderAttachment> wetAttachment;
    std::unique_ptr<SliderAttachment> dryAttachment;
    std::unique_ptr<SliderAttachment> timeAttachment;
    std::unique_ptr<SliderAttachment> feedbackAttachment;
    std::unique_ptr<SliderAttachment> dampingAttachment;

    std::unique_ptr<ButtonAttachment> pingPongAttachment;

    juce::Slider feedbackSlider;
    juce::Label feedbackLabel;

    juce::ToggleButton pingPongButton;
    juce::Slider wetSlider;
    juce::Label wetLabel;
    juce::ToggleButton morphOnButton;
    std::unique_ptr<ButtonAttachment> morphOnAttachment;
    juce::Slider drySlider;
    juce::Label dryLabel;

    juce::Slider timeSlider;
    juce::Label timeLabel;
    juce::ToggleButton driveOnButton;
    juce::Slider driveSlider;
    juce::Label driveLabel;

    std::unique_ptr<ButtonAttachment> driveOnAttachment;
    std::unique_ptr<SliderAttachment> driveAttachment;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    TESTINGAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TESTINGAudioProcessorEditor)
};
