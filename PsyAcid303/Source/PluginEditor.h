#pragma once
#include "PluginProcessor.h"

namespace PsyCol
{
inline const juce::Colour neonGreen { 0xff39ff14 }, magenta { 0xffff2bd6 }, cyan { 0xff00e5ff },
    orange { 0xffff9100 }, yellow { 0xffffe600 }, violet { 0xff9d4dff },
    panel { 0xff170a2b }, dark { 0xff0c0618 }, text { 0xffe8dcff };
}

//==============================================================================
class PsyLook : public juce::LookAndFeel_V4
{
public:
    PsyLook();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float sliderPos,
                           float minPos, float maxPos, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

//==============================================================================
class PsyAcidEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit PsyAcidEditor (PsyAcidProcessor&);
    ~PsyAcidEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using APVTS = juce::AudioProcessorValueTreeState;
    using SA = APVTS::SliderAttachment;
    using BA = APVTS::ButtonAttachment;
    using CA = APVTS::ComboBoxAttachment;

    struct Knob : juce::Component
    {
        Knob (APVTS& a, const juce::String& id, const juce::String& name, juce::Colour c)
        {
            s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 62, 14);
            s.setColour (juce::Slider::rotarySliderFillColourId, c);
            s.setColour (juce::Slider::thumbColourId, c);
            l.setText (name, juce::dontSendNotification);
            l.setJustificationType (juce::Justification::centred);
            l.setFont (juce::FontOptions (11.0f));
            l.setColour (juce::Label::textColourId, PsyCol::text);
            addAndMakeVisible (s);
            addAndMakeVisible (l);
            att = std::make_unique<SA> (a, id, s);
        }
        void resized() override
        {
            auto r = getLocalBounds();
            l.setBounds (r.removeFromTop (14));
            s.setBounds (r);
        }
        juce::Slider s;
        juce::Label l;
        std::unique_ptr<SA> att;
    };

    struct Combo : juce::Component
    {
        Combo (APVTS& a, const juce::String& id, const juce::String& name)
        {
            l.setText (name, juce::dontSendNotification);
            l.setJustificationType (juce::Justification::centred);
            l.setFont (juce::FontOptions (11.0f));
            l.setColour (juce::Label::textColourId, PsyCol::text);
            if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (a.getParameter (id)))
                box.addItemList (p->choices, 1);
            addAndMakeVisible (box);
            addAndMakeVisible (l);
            att = std::make_unique<CA> (a, id, box);
        }
        void resized() override
        {
            auto r = getLocalBounds().reduced (3, 0);
            l.setBounds (r.removeFromTop (14));
            box.setBounds (r.getX(), r.getY() + (r.getHeight() - 24) / 2, r.getWidth(), 24);
        }
        juce::ComboBox box;
        juce::Label l;
        std::unique_ptr<CA> att;
    };

    struct Tog : juce::Component
    {
        Tog (APVTS& a, const juce::String& id, const juce::String& name, juce::Colour c)
        {
            b.setButtonText (name);
            b.setColour (juce::ToggleButton::tickColourId, c);
            addAndMakeVisible (b);
            att = std::make_unique<BA> (a, id, b);
        }
        void resized() override
        {
            auto r = getLocalBounds();
            b.setBounds (r.withSizeKeepingCentre (r.getWidth() - 6, 28));
        }
        juce::ToggleButton b;
        std::unique_ptr<BA> att;
    };

    void timerCallback() override;
    void changePreset (int delta);

    PsyAcidProcessor& proc;
    PsyLook look; // declared before components -> destroyed after them

    Knob kTune, kCut, kReso, kEnv, kDec, kAcc, kGlide, kVol, kDist, kLfoDepth,
        kFb, kMix, kWidth, kTone, kLen, kGate, kTrans, kBpm;
    Combo cWave, cLfoDiv, cLfoShape, cLfoDest, cDlyTime, cSeqDiv;
    Tog tSeqOn, tFree;

    juce::ComboBox presetBox;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" };

    juce::ToggleButton gateB[PA::numSteps], accB[PA::numSteps], sldB[PA::numSteps];
    juce::Slider noteS[PA::numSteps], distS[PA::numSteps], filtS[PA::numSteps], lfoS[PA::numSteps];

    std::vector<std::unique_ptr<BA>> btnAtt;
    std::vector<std::unique_ptr<SA>> sldAtt;

    struct Panel { juce::Rectangle<int> r; juce::String title; juce::Colour c; };
    std::vector<Panel> panels;
    std::vector<std::pair<juce::Rectangle<int>, juce::String>> rowLabels;
    juce::Rectangle<int> ledRects[PA::numSteps];
    juce::Rectangle<int> ledStrip;
    int lastStep = -2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PsyAcidEditor)
};
