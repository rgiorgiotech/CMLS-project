#pragma once
#include <JuceHeader.h>

class TiramisuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static constexpr uint32_t C_ESPRESSO   = 0xff1c0e06;
    static constexpr uint32_t C_COFFEE     = 0xff3d2010;
    static constexpr uint32_t C_ROAST      = 0xff5c3317;
    static constexpr uint32_t C_CREAM      = 0xfffff8e7;
    static constexpr uint32_t C_MASCARPONE = 0xfff0ddb0;
    static constexpr uint32_t C_CHOC_DARK  = 0xff140800;

    bool isPitchKnob = false;

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int w, int h,
                          float sliderPos,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
                          juce::Slider&) override
    {
        float cx = x + w * 0.5f;
        float cy = y + h * 0.5f;
        float r  = juce::jmin(w, h) * 0.38f;

        float angle = rotaryStartAngle
                    + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.45f));
        g.fillEllipse(cx - r * 0.72f + 2, cy - r * 0.98f + 3,
                      r * 1.44f, r * 1.96f);

        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(angle, cx, cy));

            juce::Colour beanCol = isPitchKnob
                ? juce::Colour(C_MASCARPONE)
                : juce::Colour(C_COFFEE);
            juce::Colour beanDark = isPitchKnob
                ? juce::Colour(C_COFFEE)
                : juce::Colour(C_CHOC_DARK);
            juce::Colour beanHighlight = isPitchKnob
                ? juce::Colour(C_CREAM)
                : juce::Colour(C_ROAST);

            float bW = r * 1.32f;
            float bH = r * 1.90f;
            float bX = cx - bW * 0.5f;
            float bY = cy - bH * 0.5f;

            g.setColour(beanCol);
            g.fillEllipse(bX, bY, bW, bH);

            juce::Path groove;
            groove.startNewSubPath(cx, bY + bH * 0.06f);
            groove.cubicTo(cx + bW * 0.28f, cy - bH * 0.10f,
                           cx + bW * 0.28f, cy + bH * 0.10f,
                           cx, bY + bH * 0.94f);
            groove.cubicTo(cx - bW * 0.28f, cy + bH * 0.10f,
                           cx - bW * 0.28f, cy - bH * 0.10f,
                           cx, bY + bH * 0.06f);
            g.setColour(beanDark.withAlpha(0.75f));
            g.strokePath(groove, juce::PathStrokeType(r * 0.14f,
                juce::PathStrokeType::curved,
                juce::PathStrokeType::rounded));

            g.setColour(beanHighlight.withAlpha(0.30f));
            g.fillEllipse(cx - bW * 0.30f, bY + bH * 0.10f,
                          bW * 0.28f, bH * 0.18f);

            g.setColour(beanDark.withAlpha(0.55f));
            g.drawEllipse(bX, bY, bW, bH, 1.2f);
        }

        {
            juce::Graphics::ScopedSaveState ss(g);
            g.addTransform(juce::AffineTransform::rotation(angle, cx, cy));
            float dotY = cy - r * 1.12f;
            juce::Colour dotCol = isPitchKnob
                ? juce::Colour(C_CREAM)
                : juce::Colour(C_CREAM);
            g.setColour(dotCol.withAlpha(0.95f));
            g.fillEllipse(cx - 3.0f, dotY - 3.0f, 6.0f, 6.0f);
        }

        {
            float arcR = r * 1.28f;
            juce::Path arcPath;
            arcPath.addArc(cx - arcR, cy - arcR,
                           arcR * 2, arcR * 2,
                           rotaryStartAngle, angle, true);
            juce::Colour arcCol = isPitchKnob
                ? juce::Colour(C_MASCARPONE).withAlpha(0.85f)
                : juce::Colour(C_MASCARPONE).withAlpha(0.55f);
            g.setColour(arcCol);
            g.strokePath(arcPath, juce::PathStrokeType(2.4f,
                juce::PathStrokeType::curved,
                juce::PathStrokeType::rounded));
        }
    }

    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override
    {
        juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        auto bounds = button.getLocalBounds().toFloat();
        float tickSize = 18.0f;
        float tickX = 4.0f;
        float tickY = (bounds.getHeight() - tickSize) * 0.5f;

        bool on = button.getToggleState();
        juce::Colour dot = on
            ? button.findColour(juce::ToggleButton::tickColourId)
            : button.findColour(juce::ToggleButton::tickDisabledColourId);

        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.8f));
        g.fillEllipse(tickX, tickY, tickSize, tickSize);

        g.setColour(dot);
        float inset = on ? 2.5f : 5.0f;
        g.fillEllipse(tickX + inset, tickY + inset,
                      tickSize - inset * 2, tickSize - inset * 2);

        g.setColour(dot.withAlpha(0.5f));
        g.drawEllipse(tickX, tickY, tickSize, tickSize, 1.0f);

        g.setColour(button.findColour(juce::ToggleButton::textColourId));
        g.setFont(juce::FontOptions(12.0f).withStyle("Bold"));
        g.drawText(button.getButtonText(),
                   (int)(tickX + tickSize + 6), 0,
                   (int)(bounds.getWidth() - tickSize - 12),
                   (int)bounds.getHeight(),
                   juce::Justification::centredLeft);
    }

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

        float trackH = 8.0f;
        float trackY = y + (h - trackH) * 0.5f;

        g.setColour(juce::Colour(C_CHOC_DARK).withAlpha(0.9f));
        g.fillRoundedRectangle((float)x, trackY, (float)w, trackH, 4.0f);

        juce::ColourGradient morphGrad(
            juce::Colour(C_ROAST).withAlpha(0.9f), (float)x,       trackY,
            juce::Colour(C_CREAM).withAlpha(0.9f), (float)(x + w), trackY,
            false);
        g.setGradientFill(morphGrad);
        g.fillRoundedRectangle((float)x, trackY,
                               sliderPos - x, trackH, 4.0f);

        float tw = 18.0f, th = 26.0f;
        float tx = sliderPos - tw * 0.5f;
        float ty = y + (h - th) * 0.5f;

        g.setColour(juce::Colour(C_CREAM));
        g.fillEllipse(tx, ty + th * 0.15f, tw, th * 0.7f);

        juce::Path groove;
        float mx = tx + tw * 0.5f;
        groove.startNewSubPath(mx, ty + th * 0.20f);
        groove.cubicTo(mx + tw * 0.25f, ty + th * 0.38f,
                       mx + tw * 0.25f, ty + th * 0.55f,
                       mx, ty + th * 0.73f);
        groove.cubicTo(mx - tw * 0.25f, ty + th * 0.55f,
                       mx - tw * 0.25f, ty + th * 0.38f,
                       mx, ty + th * 0.20f);
        g.setColour(juce::Colour(C_COFFEE).withAlpha(0.65f));
        g.strokePath(groove, juce::PathStrokeType(1.8f));
    }
};

class PitchBeanLookAndFeel : public TiramisuLookAndFeel
{
public:
    PitchBeanLookAndFeel() { isPitchKnob = true; }
};