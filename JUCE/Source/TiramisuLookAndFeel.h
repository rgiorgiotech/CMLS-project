#pragma once
#include <JuceHeader.h>

/**
 * TiramisuLookAndFeel
 * Ridisegna i componenti JUCE con un tema pasticceria:
 * - Knob = chicco di caffè che ruota
 * - Pitch knob = variante crema con bordo scuro
 * - Toggle button = pallino colorato
 * - Slider = traccia morph con gradiente espresso -> crema
 *
 * Versione aggiornata:
 * - colori più contrastanti;
 * - knob più visibili sullo sfondo;
 * - ring/indicatore circolare del livello uguale per tutti i knob;
 * - pitch knob con lo stesso ring esterno degli altri knob.
 */
class TiramisuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    // Palette — più contrasto rispetto alla versione precedente
    static constexpr uint32_t C_ESPRESSO   = 0xff120602; // espresso quasi nero
    static constexpr uint32_t C_COFFEE     = 0xff4b2712; // caffè più visibile
    static constexpr uint32_t C_ROAST      = 0xff8a4f24; // tostato più caldo/chiaro
    static constexpr uint32_t C_CREAM      = 0xfffff8e7; // crema chiara
    static constexpr uint32_t C_MASCARPONE = 0xfff5deb3; // mascarpone
    static constexpr uint32_t C_CHOC_DARK  = 0xff080300; // cioccolato molto scuro

    bool isPitchKnob = false; // true solo per PitchBeanLookAndFeel

    //==========================================================================
    // COFFEE BEAN KNOB
    //==========================================================================
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int w, int h,
                          float sliderPos,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider&) override
    {
        const float cx = x + w * 0.5f;
        const float cy = y + h * 0.5f;
        const float r  = juce::jmin(w, h) * 0.38f;

        const float angle = rotaryStartAngle
                          + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        //==========================================================================
        // RING ESTERNO — identico per tutti, incluso il pitch knob
        //==========================================================================

        const float ringR = r * 1.34f;

        // Ring completo di background
        juce::Path baseRing;
        baseRing.addCentredArc(cx, cy,
                               ringR, ringR,
                               0.0f,
                               rotaryStartAngle,
                               rotaryEndAngle,
                               true);

        // Cerchio di base visibile anche quando il valore è basso
        g.setColour(juce::Colour(C_CREAM).withAlpha(0.25f));
        g.strokePath(baseRing,
                     juce::PathStrokeType(2.4f,
                                           juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));

        // Ring attivo/value
        juce::Path valueRing;
        valueRing.addCentredArc(cx, cy,
                                ringR, ringR,
                                0.0f,
                                rotaryStartAngle,
                                angle,
                                true);

        // Stesso colore per tutti: così il pitch ha lo stesso indicatore degli altri
        const juce::Colour activeRing = juce::Colour(C_CREAM);

        // Glow morbido
        g.setColour(activeRing.withAlpha(0.24f));
        g.strokePath(valueRing,
                     juce::PathStrokeType(6.2f,
                                           juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));

        // Linea principale del livello
        g.setColour(activeRing.withAlpha(0.98f));
        g.strokePath(valueRing,
                     juce::PathStrokeType(3.3f,
                                           juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));

        // Ombra sottile per far staccare il ring dal fondo
        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.48f));
        g.strokePath(baseRing,
                     juce::PathStrokeType(0.9f,
                                           juce::PathStrokeType::curved,
                                           juce::PathStrokeType::rounded));

        //==========================================================================
        // OMBRA SOTTO IL KNOB
        //==========================================================================
        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.66f));
        g.fillEllipse(cx - r * 0.78f + 2.3f,
                      cy - r * 1.02f + 3.5f,
                      r * 1.56f,
                      r * 2.04f);

        //==========================================================================
        // CORPO DEL CHICCO
        //==========================================================================
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(angle, cx, cy));

            // Knob normali più chiari/caldi per staccare dal pannello.
            // Pitch resta crema.
            const juce::Colour beanCol = isPitchKnob
                ? juce::Colour(C_MASCARPONE)
                : juce::Colour(C_ROAST);

            const juce::Colour beanDark = juce::Colour(C_CHOC_DARK);

            const juce::Colour beanHighlight = isPitchKnob
                ? juce::Colour(C_CREAM)
                : juce::Colour(C_MASCARPONE);

            const float bW = r * 1.34f;
            const float bH = r * 1.92f;
            const float bX = cx - bW * 0.5f;
            const float bY = cy - bH * 0.5f;

            // Corpo principale
            g.setColour(beanCol);
            g.fillEllipse(bX, bY, bW, bH);

            // Ombra interna bassa
            g.setColour(beanDark.withAlpha(isPitchKnob ? 0.10f : 0.18f));
            g.fillEllipse(bX + bW * 0.08f,
                          bY + bH * 0.48f,
                          bW * 0.84f,
                          bH * 0.42f);

            // Highlight alto-sinistra
            g.setColour(beanHighlight.withAlpha(isPitchKnob ? 0.44f : 0.34f));
            g.fillEllipse(cx - bW * 0.32f,
                          bY + bH * 0.11f,
                          bW * 0.30f,
                          bH * 0.18f);

            // Solco centrale del chicco
            juce::Path groove;
            groove.startNewSubPath(cx, bY + bH * 0.06f);
            groove.cubicTo(cx + bW * 0.28f, cy - bH * 0.10f,
                           cx + bW * 0.28f, cy + bH * 0.10f,
                           cx, bY + bH * 0.94f);
            groove.cubicTo(cx - bW * 0.28f, cy + bH * 0.10f,
                           cx - bW * 0.28f, cy - bH * 0.10f,
                           cx, bY + bH * 0.06f);

            g.setColour(beanDark.withAlpha(isPitchKnob ? 0.84f : 0.88f));
            g.strokePath(groove,
                         juce::PathStrokeType(r * 0.15f,
                                               juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

            // Doppio bordo: chiaro + scuro, per contrasto
            g.setColour(juce::Colour(C_CREAM).withAlpha(isPitchKnob ? 0.38f : 0.22f));
            g.drawEllipse(bX, bY, bW, bH, 1.0f);

            g.setColour(beanDark.withAlpha(0.74f));
            g.drawEllipse(bX, bY, bW, bH, 1.6f);
        }

        //==========================================================================
        // PUNTINO INDICATORE
        //==========================================================================
        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(angle, cx, cy));

            const float dotY = cy - r * 1.12f;

            // Pitch: puntino scuro; altri knob: puntino crema
            const juce::Colour dotCol = isPitchKnob
                ? juce::Colour(C_CHOC_DARK)
                : juce::Colour(C_CREAM);

            g.setColour(dotCol.withAlpha(0.25f));
            g.fillEllipse(cx - 5.7f, dotY - 5.7f, 11.4f, 11.4f);

            g.setColour(dotCol.withAlpha(0.98f));
            g.fillEllipse(cx - 3.2f, dotY - 3.2f, 6.4f, 6.4f);
        }
    }

    //==========================================================================
    // TOGGLE BUTTON — pallino pieno colorato
    //==========================================================================
    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        auto bounds = button.getLocalBounds().toFloat();

        const float tickSize = 18.0f;
        const float tickX = 4.0f;
        const float tickY = (bounds.getHeight() - tickSize) * 0.5f;

        const bool on = button.getToggleState();

        const juce::Colour dot = on
            ? button.findColour(juce::ToggleButton::tickColourId)
            : button.findColour(juce::ToggleButton::tickDisabledColourId);

        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.86f));
        g.fillEllipse(tickX, tickY, tickSize, tickSize);

        g.setColour(dot);
        const float inset = on ? 2.5f : 5.0f;
        g.fillEllipse(tickX + inset, tickY + inset,
                      tickSize - inset * 2.0f,
                      tickSize - inset * 2.0f);

        g.setColour(dot.withAlpha(0.65f));
        g.drawEllipse(tickX, tickY, tickSize, tickSize, 1.2f);

        g.setColour(button.findColour(juce::ToggleButton::textColourId));
        g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
        g.drawText(button.getButtonText(),
                   (int)(tickX + tickSize + 6.0f),
                   0,
                   (int)(bounds.getWidth() - tickSize - 12.0f),
                   (int)bounds.getHeight(),
                   juce::Justification::centredLeft);
    }

    //==========================================================================
    // LINEAR SLIDER (morph) — gradiente espresso -> crema
    //==========================================================================
    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int w, int h,
                          float sliderPos, float, float,
                          juce::Slider::SliderStyle style,
                          juce::Slider& slider) override
    {
        if (style != juce::Slider::LinearHorizontal)
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, w, h,
                sliderPos, 0, 0, style, slider);
            return;
        }

        juce::ignoreUnused(slider);

        const float trackH = 10.0f;
        const float trackY = y + (h - trackH) * 0.5f;
        const auto track = juce::Rectangle<float>((float)x, trackY, (float)w, trackH);

        // Base track
        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.95f));
        g.fillRoundedRectangle(track, trackH * 0.5f);

        // Gradiente completo
        juce::ColourGradient fullGradient(
            juce::Colour(C_COFFEE), (float)x, trackY,
            juce::Colour(C_CREAM),  (float)(x + w), trackY,
            false
        );

        g.setGradientFill(fullGradient);
        g.fillRoundedRectangle(track, trackH * 0.5f);

        // Overlay scuro sulla parte non raggiunta
        const float filledW = juce::jlimit(0.0f, (float)w, sliderPos - (float)x);

        if (filledW < (float)w)
        {
            g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.56f));
            g.fillRoundedRectangle((float)x + filledW,
                                   trackY,
                                   (float)w - filledW,
                                   trackH,
                                   trackH * 0.5f);
        }

        // Bordo track
        g.setColour(juce::Colour(C_CREAM).withAlpha(0.44f));
        g.drawRoundedRectangle(track, trackH * 0.5f, 1.1f);

        // Thumb — chicco piccolo
        const float tw = 20.0f;
        const float th = 28.0f;
        const float tx = sliderPos - tw * 0.5f;
        const float ty = y + (h - th) * 0.5f;

        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.48f));
        g.fillEllipse(tx + 1.5f,
                      ty + th * 0.15f + 2.0f,
                      tw,
                      th * 0.7f);

        g.setColour(juce::Colour(C_CREAM));
        g.fillEllipse(tx,
                      ty + th * 0.15f,
                      tw,
                      th * 0.7f);

        juce::Path groove;
        const float mx = tx + tw * 0.5f;
        groove.startNewSubPath(mx, ty + th * 0.20f);
        groove.cubicTo(mx + tw * 0.25f, ty + th * 0.38f,
                       mx + tw * 0.25f, ty + th * 0.55f,
                       mx, ty + th * 0.73f);
        groove.cubicTo(mx - tw * 0.25f, ty + th * 0.55f,
                       mx - tw * 0.25f, ty + th * 0.38f,
                       mx, ty + th * 0.20f);

        g.setColour(juce::Colour(C_COFFEE).withAlpha(0.78f));
        g.strokePath(groove, juce::PathStrokeType(1.8f));

        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.38f));
        g.drawEllipse(tx, ty + th * 0.15f, tw, th * 0.7f, 1.0f);
    }
};

// Versione per il pitch knob (chicco crema)
class PitchBeanLookAndFeel : public TiramisuLookAndFeel
{
public:
    PitchBeanLookAndFeel()
    {
        isPitchKnob = true;
    }
};
