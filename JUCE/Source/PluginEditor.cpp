/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TESTINGAudioProcessorEditor::TESTINGAudioProcessorEditor (TESTINGAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    wetSlider.setRange(0.0, 1.0, 0.01);
    wetSlider.setValue(0.5);
    wetSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    //wetSlider.addListener(this);
    wetLabel.setText("Wet Level", juce::dontSendNotification);

    drySlider.setRange(0.0, 1.0, 0.01);
    drySlider.setValue(0.5);
    drySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    //drySlider.addListener(this);
    dryLabel.setText("Dry Level", juce::dontSendNotification);

    timeSlider.setRange(1.0, 2000.0, 1.0);
    timeSlider.setValue(350.0);
    timeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    //timeSlider.addListener(this);
    timeLabel.setText("Time ms", juce::dontSendNotification);

    feedbackSlider.setRange(0.0, 0.95, 0.01);
    feedbackSlider.setValue(0.35);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    //feedbackSlider.addListener(this);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    dampingSlider.setRange(500.0, 20000.0, 100.0);
    dampingSlider.setValue(8000.0);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    dampingLabel.setText("Damping Hz", juce::dontSendNotification);
    driveOnButton.setButtonText("Drive ON");
    addAndMakeVisible(driveOnButton);

    driveSlider.setRange(1.0, 20.0, 0.1);
    driveSlider.setValue(1.0);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    driveLabel.setText("Drive", juce::dontSendNotification);

    addAndMakeVisible(driveSlider);
    addAndMakeVisible(driveLabel);
    pingPongButton.setButtonText("Ping-Pong (audible with headset)");
    //pingPongButton.addListener(this);
    morphSlider.setRange(0.0, 1.0, 0.01);
    morphSlider.setValue(0.0);
    morphSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    morphLabel.setText("Morph Ambient-Glitch", juce::dontSendNotification);
    morphOnButton.setButtonText("Enable Morph/Glitch");
    addAndMakeVisible(morphSlider);
    addAndMakeVisible(morphLabel);

    addAndMakeVisible(wetSlider);
    addAndMakeVisible(wetLabel);

    addAndMakeVisible(drySlider);
    addAndMakeVisible(dryLabel);

    addAndMakeVisible(timeSlider);
    addAndMakeVisible(timeLabel);

    addAndMakeVisible(feedbackSlider);
    addAndMakeVisible(dampingSlider);
    addAndMakeVisible(dampingLabel);
    addAndMakeVisible(feedbackLabel);
    addAndMakeVisible(pingPongButton);
    
    addAndMakeVisible(morphOnButton);

    morphOnAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts,
        "MORPHON",
        morphOnButton
    );

    wetAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts,
        "WET",
        wetSlider
    );

    dryAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts,
        "DRY",
        drySlider
    );

    timeAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts,
        "DELAYTIME",
        timeSlider
    );

    feedbackAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts,
        "FEEDBACK",
        feedbackSlider
    );

    pingPongAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts,
        "PINGPONG",
        pingPongButton
    );
    dampingAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts,
        "DAMPING",
        dampingSlider
    );
    morphAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts,
        "MORPH",
        morphSlider
    );
    driveOnAttachment = std::make_unique<ButtonAttachment>(
        audioProcessor.apvts,
        "DRIVEON",
        driveOnButton
    );

    driveAttachment = std::make_unique<SliderAttachment>(
        audioProcessor.apvts,
        "DRIVE",
        driveSlider
    );

    setSize(500, 420);
}

TESTINGAudioProcessorEditor::~TESTINGAudioProcessorEditor()
{
}

//==============================================================================
void TESTINGAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void TESTINGAudioProcessorEditor::resized()
{
    morphLabel.setBounds(10, 10, 150, 20);
    morphSlider.setBounds(170, 10, getWidth() - 180, 20);
    wetLabel.setBounds(10, 50, 90, 20);
    wetSlider.setBounds(100, 50, getWidth() - 110, 20);

    dryLabel.setBounds(10, 90, 90, 20);
    drySlider.setBounds(100, 90, getWidth() - 110, 20);

    timeLabel.setBounds(10, 130, 90, 20);
    timeSlider.setBounds(100, 130, getWidth() - 110, 20);

    feedbackLabel.setBounds(10, 170, 90, 20);
    feedbackSlider.setBounds(100, 170, getWidth() - 110, 20);

    dampingLabel.setBounds(10, 210, 90, 20);
    dampingSlider.setBounds(100, 210, getWidth() - 110, 20);

    pingPongButton.setBounds(100, 250, 260, 30);
    morphOnButton.setBounds(100, 285, 200, 30);
    driveOnButton.setBounds(100, 325, 120, 30);

    driveLabel.setBounds(10, 360, 90, 20);
    driveSlider.setBounds(100, 360, getWidth() - 110, 20);

    
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}