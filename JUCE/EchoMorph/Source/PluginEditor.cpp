#include "PluginProcessor.h"
#include "PluginEditor.h"

static const juce::Colour BG        { 0xff1c0e06 }; // espresso scuro
static const juce::Colour PANEL     { 0xff2e1608 }; // corpo caffè
static const juce::Colour CREAM     { 0xfffff8e7 }; // crema
static const juce::Colour MASCARP   { 0xfff0ddb0 }; // mascarpone
static const juce::Colour COFFEE    { 0xff3d2010 }; // caffè medio
static const juce::Colour ROAST     { 0xff5c3317 }; // tostato
static const juce::Colour CHOC      { 0xff140800 }; // cioccolato fondente
static const juce::Colour TEXT_COL  { 0xfff5e6c8 }; // testo crema

//==============================================================================
TESTINGAudioProcessorEditor::TESTINGAudioProcessorEditor(TESTINGAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Applica il look and feel tiramisù a tutti i componenti
    setLookAndFeel(&laf);

    // Labels
    for (auto* l : { &wetLabel, &dryLabel, &timeLabel, &feedbackLabel,
                     &dampingLabel, &driveLabel, &morphLabel, &pitchLabel })
    {
        l->setColour(juce::Label::textColourId, TEXT_COL);
        l->setJustificationType(juce::Justification::centred);
        l->setFont(juce::FontOptions(12.0f).withStyle("Bold"));
    }
    wetLabel.setText("Wet",                    juce::dontSendNotification);
    dryLabel.setText("Dry",                    juce::dontSendNotification);
    timeLabel.setText("Time ms",               juce::dontSendNotification);
    feedbackLabel.setText("Feedback",          juce::dontSendNotification);
    dampingLabel.setText("Damping",            juce::dontSendNotification);
    driveLabel.setText("Drive",                juce::dontSendNotification);
    morphLabel.setText("Espresso",              juce::dontSendNotification);
    pitchLabel.setText("Pitch",                juce::dontSendNotification);

    // Morph slider
    morphSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    morphSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    morphSlider.setColour(juce::Slider::textBoxTextColourId,       TEXT_COL);
    morphSlider.setColour(juce::Slider::textBoxBackgroundColourId, PANEL);
    morphSlider.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);

    // Knob style
    auto setupKnob = [](juce::Slider& s)
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 16);
        s.setColour(juce::Slider::textBoxTextColourId,         juce::Colour(0xfff5e6c8));
        s.setColour(juce::Slider::textBoxBackgroundColourId,   juce::Colour(0xff2e1608));
        s.setColour(juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    };
    setupKnob(wetSlider);
    setupKnob(drySlider);
    setupKnob(timeSlider);
    setupKnob(feedbackSlider);
    setupKnob(dampingSlider);
    setupKnob(driveSlider);

    // Pitch knob — LAF separato (chicco crema), ma con value box visibile come gli altri knob
    setupKnob(pitchSlider);
    pitchSlider.setLookAndFeel(&pitchLaf);
    pitchSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 16);
    pitchSlider.setNumDecimalPlacesToDisplay(3);
    pitchSlider.setColour(juce::Slider::textBoxTextColourId,       TEXT_COL);
    pitchSlider.setColour(juce::Slider::textBoxBackgroundColourId, PANEL);
    pitchSlider.setColour(juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);

    // Bottoni colorati stile jack audio
    pingPongButton.setButtonText("Ping Pong");
    pingPongButton.setColour(juce::ToggleButton::textColourId,         TEXT_COL);
    pingPongButton.setColour(juce::ToggleButton::tickColourId,         juce::Colour(0xff3399ff));
    pingPongButton.setColour(juce::ToggleButton::tickDisabledColourId, ROAST);

    morphOnButton.setButtonText("Morph ON");
    morphOnButton.setColour(juce::ToggleButton::textColourId,         TEXT_COL);
    morphOnButton.setColour(juce::ToggleButton::tickColourId,         juce::Colour(0xffee4444));
    morphOnButton.setColour(juce::ToggleButton::tickDisabledColourId, ROAST);

    driveOnButton.setButtonText("Drive ON");
    driveOnButton.setColour(juce::ToggleButton::textColourId,         TEXT_COL);
    driveOnButton.setColour(juce::ToggleButton::tickColourId,         juce::Colour(0xff33cc66));
    driveOnButton.setColour(juce::ToggleButton::tickDisabledColourId, ROAST);

    addAndMakeVisible(morphSlider);    addAndMakeVisible(morphLabel);
    addAndMakeVisible(wetSlider);      addAndMakeVisible(wetLabel);
    addAndMakeVisible(drySlider);      addAndMakeVisible(dryLabel);
    addAndMakeVisible(timeSlider);     addAndMakeVisible(timeLabel);
    addAndMakeVisible(feedbackSlider); addAndMakeVisible(feedbackLabel);
    addAndMakeVisible(dampingSlider);  addAndMakeVisible(dampingLabel);
    addAndMakeVisible(driveSlider);    addAndMakeVisible(driveLabel);
    addAndMakeVisible(pitchSlider);    addAndMakeVisible(pitchLabel);
    addAndMakeVisible(pingPongButton);
    addAndMakeVisible(morphOnButton);
    addAndMakeVisible(driveOnButton);
    addAndMakeVisible(spectrumAnalyzer);

    morphOnAttachment  = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "MORPHON",   morphOnButton);
    wetAttachment      = std::make_unique<SliderAttachment>(audioProcessor.apvts, "WET",        wetSlider);
    dryAttachment      = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DRY",        drySlider);
    timeAttachment     = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DELAYTIME",  timeSlider);
    feedbackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FEEDBACK",   feedbackSlider);
    dampingAttachment  = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DAMPING",    dampingSlider);
    morphAttachment    = std::make_unique<SliderAttachment>(audioProcessor.apvts, "MORPH",      morphSlider);
    pingPongAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "PINGPONG",   pingPongButton);
    driveOnAttachment  = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "DRIVEON",    driveOnButton);
    driveAttachment    = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DRIVE",      driveSlider);
    pitchAttachment    = std::make_unique<SliderAttachment>(audioProcessor.apvts, "PITCH",      pitchSlider);

    startTimerHz(30);
    setSize(720, 680);
}

TESTINGAudioProcessorEditor::~TESTINGAudioProcessorEditor()
{
    stopTimer();
    pitchSlider.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void TESTINGAudioProcessorEditor::pushAudioBufferToVisuals(const juce::AudioBuffer<float>& b)
{
    if (b.getNumChannels() > 0)
        spectrumAnalyzer.pushSamples(b.getReadPointer(0), b.getNumSamples());
}

void TESTINGAudioProcessorEditor::timerCallback()
{
    displayEnvelope = audioProcessor.envelopeValue.load();
    displayMorph    = audioProcessor.morphValue.load();
    repaint();
}

//==============================================================================
// Bordo crema piped — serie di piccoli cerchi sovrapposti lungo il perimetro
void TESTINGAudioProcessorEditor::drawCreamPiping(juce::Graphics& g,
    juce::Rectangle<float> bounds)
{
    float blobR  = 7.0f;
    float step   = blobR * 1.55f;
    auto inner   = bounds.reduced(blobR * 0.4f);

    // Colori crema per i blob
    juce::Colour light = CREAM.withAlpha(0.82f);
    juce::Colour shadow = juce::Colour(0xff8b5a30).withAlpha(0.30f);

    // Funzione per disegnare un blob crema in una posizione
    auto blob = [&](float bx, float by)
    {
        // Ombra
        g.setColour(shadow);
        g.fillEllipse(bx - blobR + 1.5f, by - blobR * 0.7f + 1.5f,
                      blobR * 2.0f, blobR * 1.4f);
        // Corpo
        g.setColour(light);
        g.fillEllipse(bx - blobR, by - blobR * 0.7f,
                      blobR * 2.0f, blobR * 1.4f);
        // Highlight
        g.setColour(CREAM.withAlpha(0.55f));
        g.fillEllipse(bx - blobR * 0.4f, by - blobR * 0.55f,
                      blobR * 0.6f, blobR * 0.4f);
    };

    // Lato superiore
    float y = inner.getY();
    for (float bx = inner.getX(); bx <= inner.getRight(); bx += step)
        blob(bx, y);

    // Lato inferiore (solo sopra lo spectrum)
    float yb = inner.getY() + 490.0f;
    for (float bx = inner.getX(); bx <= inner.getRight(); bx += step)
        blob(bx, yb);

    // Lato sinistro
    for (float by = inner.getY() + step; by < yb; by += step)
        blob(inner.getX(), by);

    // Lato destro
    for (float by = inner.getY() + step; by < yb; by += step)
        blob(inner.getRight(), by);
}

// Chicco di caffè decorativo piccolo per il background
void TESTINGAudioProcessorEditor::drawCoffeeBeanBg(juce::Graphics& g,
    float cx, float cy, float r, float angle)
{
    juce::Graphics::ScopedSaveState ss(g);
    g.addTransform(juce::AffineTransform::rotation(angle, cx, cy));

    g.setColour(COFFEE.withAlpha(0.12f));
    g.fillEllipse(cx - r * 0.65f, cy - r, r * 1.3f, r * 2.0f);

    juce::Path groove;
    groove.startNewSubPath(cx, cy - r * 0.88f);
    groove.cubicTo(cx + r * 0.28f, cy - r * 0.3f,
                   cx + r * 0.28f, cy + r * 0.3f,
                   cx, cy + r * 0.88f);
    groove.cubicTo(cx - r * 0.28f, cy + r * 0.3f,
                   cx - r * 0.28f, cy - r * 0.3f,
                   cx, cy - r * 0.88f);
    g.setColour(CHOC.withAlpha(0.08f));
    g.strokePath(groove, juce::PathStrokeType(r * 0.12f));
}

// Ciuffo di crema montata
void TESTINGAudioProcessorEditor::drawCreamSwirl(juce::Graphics& g,
    float cx, float cy, float r)
{
    // Base tonda
    g.setColour(CREAM.withAlpha(0.75f));
    g.fillEllipse(cx - r, cy - r * 0.5f, r * 2.0f, r);

    // Spirale in cima — tre cerchi sovrapposti
    for (int i = 0; i < 3; ++i)
    {
        float sr = r * (0.75f - i * 0.20f);
        float sy = cy - r * 0.5f - i * r * 0.45f;
        g.setColour(CREAM.withAlpha(0.80f - i * 0.15f));
        g.fillEllipse(cx - sr, sy - sr * 0.6f, sr * 2.0f, sr * 1.2f);
        // Highlight
        g.setColour(juce::Colour(0xffffffff).withAlpha(0.25f - i * 0.06f));
        g.fillEllipse(cx - sr * 0.35f, sy - sr * 0.45f,
                      sr * 0.4f, sr * 0.25f);
    }

    // Punta
    g.setColour(CREAM.withAlpha(0.85f));
    float tipR = r * 0.18f;
    g.fillEllipse(cx - tipR, cy - r * 1.85f, tipR * 2, tipR * 2);
}

//==============================================================================
void TESTINGAudioProcessorEditor::paint(juce::Graphics& g)
{
    int W = getWidth();
    int H = getHeight();

    // -------------------------------------------------------
    // SFONDO — corpo strumento
    // -------------------------------------------------------
    g.fillAll(BG);

    // Pannello principale
    auto panel = getLocalBounds().toFloat().reduced(10.0f);
    juce::ColourGradient panelGrad(
        PANEL.withAlpha(0.97f), 0.0f, 0.0f,
        COFFEE.withAlpha(0.5f), 0.0f, (float)H,
        false);
    g.setGradientFill(panelGrad);
    g.fillRoundedRectangle(panel, 14.0f);

    // -------------------------------------------------------
    // CHICCHI DI CAFFÈ BACKGROUND (decorativi, bassissima opacità)
    // Solo nelle aree libere dai controlli
    // -------------------------------------------------------
    struct Bean { float x, y, r, a; };
    const Bean beans[] = {
        { 580, 130, 14, 0.3f },  { 590, 200, 11, 1.1f },
        { 570, 320, 13, -0.5f }, { 585, 390, 10, 0.8f },
        { 55,  440, 12, 0.2f },  { 120, 455, 11, -0.9f },
        { 420, 448, 13, 0.6f },  { 500, 440, 10, -0.3f },
        { 600, 460, 11, 1.0f },
    };
    for (auto& b : beans)
        drawCoffeeBeanBg(g, b.x, b.y, b.r, b.a);

    // -------------------------------------------------------
    // BORDO CREMA PIPED
    // -------------------------------------------------------
    drawCreamPiping(g, panel);

    // -------------------------------------------------------
    // CIUFFI CREMA agli angoli superiori
    // -------------------------------------------------------
    drawCreamSwirl(g, (float)W - 60.0f, 56.0f, 22.0f);
    drawCreamSwirl(g, (float)W - 30.0f, 48.0f, 15.0f);

    // -------------------------------------------------------
    // BORDO pannello
    // -------------------------------------------------------
    g.setColour(ROAST.withAlpha(0.45f));
    g.drawRoundedRectangle(panel, 14.0f, 1.5f);

    // -------------------------------------------------------
    // HEADER — titolo stampato direttamente sul plugin
    // -------------------------------------------------------
    // Piccolo chicco decorativo accanto al titolo
    drawCoffeeBeanBg(g, 34.0f, 38.0f, 10.0f, 0.2f);

    // Titolo elegante: niente targhetta, testo direttamente sul corpo del plugin
    g.setColour(CREAM);
    g.setFont(juce::FontOptions(34.0f).withStyle("Bold Italic"));
    g.drawText("Ladyfingers", 54, 12, 260, 34,
               juce::Justification::centredLeft);

    g.setColour(MASCARP.withAlpha(0.92f));
    g.setFont(juce::FontOptions(13.0f).withStyle("Italic"));
    g.drawText("Morph Delay", 56, 45, 180, 18,
               juce::Justification::centredLeft);

    g.setColour(TEXT_COL.withAlpha(0.40f));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("SuperCollider OSC - Tiramisu Edition",
               318, 28, 245, 14, juce::Justification::centredLeft);

    // Separatore
    g.setColour(ROAST.withAlpha(0.30f));
    g.drawLine(18.0f, 70.0f, (float)W - 18.0f, 70.0f, 1.0f);

    // -------------------------------------------------------
    // BARRA MORPH — label e cornice, la traccia è disegnata dal TiramisuLookAndFeel
    // -------------------------------------------------------
    auto morphFrame = juce::Rectangle<float>(28.0f, 82.0f, (float)W - 74.0f, 38.0f);
    g.setColour(CHOC.withAlpha(0.26f));
    g.fillRoundedRectangle(morphFrame, 8.0f);
    g.setColour(MASCARP.withAlpha(0.18f));
    g.drawRoundedRectangle(morphFrame, 8.0f, 1.0f);

    g.setFont(juce::FontOptions(10.0f).withStyle("Bold"));
    g.setColour(TEXT_COL.withAlpha(0.78f));
    g.drawText("ESPRESSO", 38, 86, 90, 14, juce::Justification::left);
    g.drawText("PANNA", W - 120, 86, 70, 14, juce::Justification::right);

    // Valore morph stampato direttamente sul background: niente textbox JUCE separata
    g.setColour(CREAM.withAlpha(0.94f));
    g.setFont(juce::FontOptions(18.0f).withStyle("Bold"));
    g.drawText(juce::String(displayMorph, 3), W - 118, 97, 80, 24,
               juce::Justification::centred);

    // -------------------------------------------------------
    // SEZIONE KNOB — sfondo rialzato sottile
    // -------------------------------------------------------
    g.setColour(CHOC.withAlpha(0.25f));
    g.fillRoundedRectangle(18.0f, 128.0f, (float)W - 70.0f, 350.0f, 12.0f);

    // -------------------------------------------------------
    // ENV METER — colonna crema
    // -------------------------------------------------------
    const int bX = W - 46;
    const int bY = 130;
    const int bH = 348;
    const int bW = 16;
    const int bF = (int)(juce::jlimit(0.0f, 1.0f, displayEnvelope) * bH);

    g.setColour(CHOC.withAlpha(0.95f));
    g.fillRoundedRectangle((float)bX, (float)bY, (float)bW, (float)bH, 3.5f);

    if (bF > 0)
    {
        juce::ColourGradient eG(
            ROAST.withAlpha(0.85f),  (float)bX, (float)(bY + bH - bF),
            MASCARP,                 (float)bX, (float)(bY + bH),
            false);
        g.setGradientFill(eG);
        g.fillRoundedRectangle((float)bX, (float)(bY + bH - bF),
                               (float)bW, (float)bF, 3.5f);
    }

    g.setFont(juce::FontOptions(8.0f));
    g.setColour(TEXT_COL.withAlpha(0.38f));
    g.drawText("ENV", bX - 2, bY + bH + 4, bW + 4, 12,
               juce::Justification::centred);

    // -------------------------------------------------------
    // SEPARATORE + label spectrum
    // -------------------------------------------------------
    g.setColour(ROAST.withAlpha(0.28f));
    g.drawLine(22.0f, 504.0f, (float)W - 22.0f, 504.0f, 1.0f);
    g.setFont(juce::FontOptions(10.0f).withStyle("Bold Italic"));
    g.setColour(MASCARP.withAlpha(0.60f));
    g.drawText("✦ Output Spectrum", 26, 508, 190, 13,
               juce::Justification::left);
}

//==============================================================================
void TESTINGAudioProcessorEditor::resized()
{
    // Layout leggermente più largo e arioso:
    // - la barra Morph non usa più il textbox JUCE;
    // - il valore Morph viene disegnato in paint() direttamente sul pannello;
    // - i knob hanno più spazio tra label e ring circolare.

    // Morph slider — dentro la cornice disegnata in paint()
    morphLabel.setBounds(60, 100, 96, 18);
    morphSlider.setBounds(170, 94, getWidth() - 300, 30);

    const int knobSize    = 104;
    const int labelHeight = 22;
    const int cellWidth   = 138;
    const int cellHeight  = 152;
    const int startX      = 38;
    const int startY      = 145;

    auto place = [&](juce::Slider& s, juce::Label& l, int x, int y)
    {
        // Label più larga e separata dal knob: non taglia più il ring.
        l.setBounds(x, y, cellWidth, labelHeight);
        l.setJustificationType(juce::Justification::centred);

        const int sliderX = x + (cellWidth - knobSize) / 2;
        const int sliderY = y + labelHeight + 14;
        s.setBounds(sliderX, sliderY, knobSize, knobSize);
    };

    // Fila 1
    place(wetSlider,      wetLabel,      startX + cellWidth * 0, startY);
    place(drySlider,      dryLabel,      startX + cellWidth * 1, startY);
    place(timeSlider,     timeLabel,     startX + cellWidth * 2, startY);
    place(feedbackSlider, feedbackLabel, startX + cellWidth * 3, startY);

    // Fila 2
    place(dampingSlider, dampingLabel, startX + cellWidth * 0, startY + cellHeight);
    place(driveSlider,   driveLabel,   startX + cellWidth * 1, startY + cellHeight);
    place(pitchSlider,   pitchLabel,   startX + cellWidth * 2, startY + cellHeight);

    // Bottoni a destra:
    // allineati alla riga del Pitch e posizionati sotto al Feedback.
    // Così non invadono il knob Feedback e partono dalla stessa zona verticale del Pitch.
    const int pitchRowY    = startY + cellHeight;
    const int pitchSliderY = pitchRowY + labelHeight + 14;

    const int buttonX = startX + cellWidth * 3 + 24;
    const int buttonY = pitchSliderY + 6;

    pingPongButton.setBounds(buttonX, buttonY,      150, 28);
    morphOnButton.setBounds( buttonX, buttonY + 42, 150, 28);
    driveOnButton.setBounds( buttonX, buttonY + 84, 150, 28);

    // Spectrum più largo e con margini coerenti
    spectrumAnalyzer.setBounds(22, 526, getWidth() - 44, 128);
}
