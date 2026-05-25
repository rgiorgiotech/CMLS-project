#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TESTINGAudioProcessorEditor::TESTINGAudioProcessorEditor(TESTINGAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    for (auto* label : { &wetLabel, &dryLabel, &timeLabel, &feedbackLabel,
                         &dampingLabel, &driveLabel, &morphLabel })
    {
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        label->setJustificationType(juce::Justification::centred);
        label->setFont(juce::FontOptions(14.0f).withStyle("Bold"));
    }

    wetLabel.setText("Wet", juce::dontSendNotification);
    dryLabel.setText("Dry", juce::dontSendNotification);
    timeLabel.setText("Time ms", juce::dontSendNotification);
    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    dampingLabel.setText("Damping", juce::dontSendNotification);
    driveLabel.setText("Drive", juce::dontSendNotification);
    morphLabel.setText("Morph Ambient / Glitch", juce::dontSendNotification);

    // Morph rimane slider orizzontale
    morphSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    morphSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 70, 22);
    morphSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffff9f1c));
    morphSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xffff9f1c));
    morphSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff303040));
    morphSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    morphSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff202028));
    morphSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    auto setupKnob = [](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 22);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffff9f1c));
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff363648));
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::white);
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff202028));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    };

    setupKnob(wetSlider);
    setupKnob(drySlider);
    setupKnob(timeSlider);
    setupKnob(feedbackSlider);
    setupKnob(dampingSlider);
    setupKnob(driveSlider);

    pingPongButton.setButtonText("Ping Pong");
    morphOnButton.setButtonText("Morph ON");
    driveOnButton.setButtonText("Drive ON");

    for (auto* button : { &pingPongButton, &morphOnButton, &driveOnButton })
    {
        button->setColour(juce::ToggleButton::textColourId, juce::Colours::white);
        button->setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffff9f1c));
        button->setColour(juce::ToggleButton::tickDisabledColourId, juce::Colour(0xff555560));
    }

    addAndMakeVisible(morphSlider);
    addAndMakeVisible(morphLabel);
    addAndMakeVisible(wetSlider);
    addAndMakeVisible(wetLabel);
    addAndMakeVisible(drySlider);
    addAndMakeVisible(dryLabel);
    addAndMakeVisible(timeSlider);
    addAndMakeVisible(timeLabel);
    addAndMakeVisible(feedbackSlider);
    addAndMakeVisible(feedbackLabel);
    addAndMakeVisible(dampingSlider);
    addAndMakeVisible(dampingLabel);
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(driveLabel);
    addAndMakeVisible(pingPongButton);
    addAndMakeVisible(morphOnButton);
    addAndMakeVisible(driveOnButton);
    addAndMakeVisible(spectrumAnalyzer);

    morphOnAttachment  = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "MORPHON",   morphOnButton);
    wetAttachment      = std::make_unique<SliderAttachment>(audioProcessor.apvts, "WET",       wetSlider);
    dryAttachment      = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DRY",       drySlider);
    timeAttachment     = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DELAYTIME", timeSlider);
    feedbackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FEEDBACK",  feedbackSlider);
    dampingAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DAMPING",   dampingSlider);
    morphAttachment    = std::make_unique<SliderAttachment>(audioProcessor.apvts, "MORPH",     morphSlider);
    pingPongAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "PINGPONG",  pingPongButton);
    driveOnAttachment  = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "DRIVEON",   driveOnButton);
    driveAttachment    = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DRIVE",     driveSlider);

    startTimerHz(30);
    setSize(620, 600);
}

TESTINGAudioProcessorEditor::~TESTINGAudioProcessorEditor()
{
    stopTimer();
}

void TESTINGAudioProcessorEditor::pushAudioBufferToVisuals(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() > 0)
        spectrumAnalyzer.pushSamples(buffer.getReadPointer(0), buffer.getNumSamples());
}

//==============================================================================
void TESTINGAudioProcessorEditor::timerCallback()
{
    displayEnvelope = audioProcessor.envelopeValue.load();
    displayMorph = audioProcessor.morphValue.load();
    repaint();
}

//==============================================================================
void TESTINGAudioProcessorEditor::paint(juce::Graphics& g)
{
    juce::ColourGradient backgroundGradient(
        juce::Colour(0xff14141c), 0.0f, 0.0f,
        juce::Colour(0xff272735), 0.0f, (float)getHeight(),
        false);
    g.setGradientFill(backgroundGradient);
    g.fillAll();

    auto panel = getLocalBounds().toFloat().reduced(14.0f);
    g.setColour(juce::Colour(0xff20202a));
    g.fillRoundedRectangle(panel, 18.0f);
    g.setColour(juce::Colour(0x55ffffff));
    g.drawRoundedRectangle(panel, 18.0f, 1.0f);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(24.0f).withStyle("Bold"));
    g.drawText("SC Morph Delay", 30, 18, getWidth() - 60, 32,
               juce::Justification::centredLeft);

    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::FontOptions(13.0f));
    g.drawText("SuperCollider OSC controlled delay processor",
               32, 48, getWidth() - 64, 22,
               juce::Justification::centredLeft);

    // Mini indicatore Ambient -> Glitch
    auto morphBar = juce::Rectangle<float>(30.0f, 90.0f, (float)getWidth() - 78.0f, 6.0f);
    g.setColour(juce::Colour(0xff34344a));
    g.fillRoundedRectangle(morphBar, 3.0f);
    g.setColour(juce::Colour(0xffff9f1c));
    g.fillRoundedRectangle(morphBar.withWidth(morphBar.getWidth() * displayMorph), 3.0f);

    g.setFont(juce::FontOptions(10.0f));
    g.setColour(juce::Colour(0xff777792));
    g.drawText("AMBIENT", (int)morphBar.getX(), 98, 80, 14, juce::Justification::left);
    g.drawText("GLITCH", (int)morphBar.getRight() - 80, 98, 80, 14, juce::Justification::right);

    // ENV meter laterale
    const int barX = getWidth() - 36;
    const int barY = 125;
    const int barMaxH = 245;
    const int barW = 18;
    const int filledH = (int)(juce::jlimit(0.0f, 1.0f, displayEnvelope) * barMaxH);

    g.setColour(juce::Colour(0xff151522));
    g.fillRoundedRectangle((float)barX, (float)barY, (float)barW, (float)barMaxH, 4.0f);

    if (filledH > 0)
    {
        auto envColour = juce::Colour::fromHSV(
            0.25f - juce::jlimit(0.0f, 1.0f, displayEnvelope) * 0.25f,
            0.85f, 0.95f, 1.0f);

        g.setColour(envColour);
        g.fillRoundedRectangle((float)barX,
                               (float)(barY + barMaxH - filledH),
                               (float)barW,
                               (float)filledH,
                               4.0f);
    }

    g.setFont(juce::FontOptions(9.0f));
    g.setColour(juce::Colour(0xff777792));
    g.drawText("ENV", barX - 2, barY + barMaxH + 4, barW + 4, 12,
               juce::Justification::centred);

    g.setFont(juce::FontOptions(11.0f).withStyle("Bold"));
    g.setColour(juce::Colour(0xffc9b6ff));
    g.drawText("Output Spectrum", 30, 425, 180, 16, juce::Justification::left);
}

//==============================================================================
void TESTINGAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(28);
    area.removeFromTop(74);

    auto morphArea = area.removeFromTop(55);
    morphLabel.setBounds(morphArea.removeFromLeft(180));
    morphSlider.setBounds(morphArea.reduced(8, 12));

    area.removeFromTop(15);

    const int knobSize = 95;
    const int labelHeight = 22;
    const int cellWidth = 120;
    const int cellHeight = 135;

    auto placeKnob = [&](juce::Slider& slider, juce::Label& label, int x, int y)
    {
        label.setBounds(x, y, knobSize, labelHeight);
        slider.setBounds(x, y + labelHeight, knobSize, knobSize);
    };

    const int startX = 45;
    const int startY = 145;

    placeKnob(wetSlider,      wetLabel,      startX + cellWidth * 0, startY);
    placeKnob(drySlider,      dryLabel,      startX + cellWidth * 1, startY);
    placeKnob(timeSlider,     timeLabel,     startX + cellWidth * 2, startY);
    placeKnob(feedbackSlider, feedbackLabel, startX + cellWidth * 3, startY);

    placeKnob(dampingSlider, dampingLabel, startX + cellWidth * 1, startY + cellHeight);
    placeKnob(driveSlider,   driveLabel,   startX + cellWidth * 2, startY + cellHeight);

    const int buttonX = startX + cellWidth * 3;
    const int buttonY = startY + cellHeight + 20;
    pingPongButton.setBounds(buttonX, buttonY,      130, 24);
    morphOnButton.setBounds( buttonX, buttonY + 32, 130, 24);
    driveOnButton.setBounds( buttonX, buttonY + 64, 130, 24);

    spectrumAnalyzer.setBounds(28, 448, getWidth() - 56, 120);
}
