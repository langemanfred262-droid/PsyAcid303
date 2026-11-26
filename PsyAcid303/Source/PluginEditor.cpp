#include "PluginEditor.h"
#include "Presets.h"

using namespace PsyCol;

//==============================================================================
PsyLook::PsyLook()
{
    setColour (juce::ComboBox::backgroundColourId, dark);
    setColour (juce::ComboBox::textColourId, text);
    setColour (juce::ComboBox::outlineColourId, violet);
    setColour (juce::ComboBox::arrowColourId, neonGreen);
    setColour (juce::PopupMenu::backgroundColourId, dark);
    setColour (juce::PopupMenu::textColourId, text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, magenta.darker (0.4f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::Label::textColourId, text);
    setColour (juce::TextButton::buttonColourId, panel);
    setColour (juce::TextButton::textColourOffId, neonGreen);
    setColour (juce::Slider::textBoxTextColourId, text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void PsyLook::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                float startAngle, float endAngle, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (5.0f);
    const float dia = juce::jmin (b.getWidth(), b.getHeight());
    const auto c = b.getCentre();
    const float r = dia * 0.5f;
    const auto col = s.findColour (juce::Slider::rotarySliderFillColourId);
    const float ang = startAngle + pos * (endAngle - startAngle);

    // glow + track
    g.setColour (col.withAlpha (0.12f));
    g.fillEllipse (c.x - r, c.y - r, r * 2.0f, r * 2.0f);

    juce::Path track, val;
    const float ar = r - 2.0f;
    track.addCentredArc (c.x, c.y, ar, ar, 0.0f, startAngle, endAngle, true);
    val.addCentredArc (c.x, c.y, ar, ar, 0.0f, startAngle, ang, true);
    g.setColour (juce::Colour (0xff2a1850));
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (col);
    g.strokePath (val, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // knob body
    const float br = r * 0.68f;
    juce::ColourGradient grad (juce::Colour (0xff3b2468), c.x - br * 0.5f, c.y - br * 0.6f,
                               juce::Colour (0xff0c0618), c.x + br * 0.6f, c.y + br * 0.8f, true);
    g.setGradientFill (grad);
    g.fillEllipse (c.x - br, c.y - br, br * 2.0f, br * 2.0f);
    g.setColour (col.withAlpha (0.6f));
    g.drawEllipse (c.x - br, c.y - br, br * 2.0f, br * 2.0f, 1.2f);

    // pointer
    const float px = c.x + std::sin (ang) * br * 0.92f, py = c.y - std::cos (ang) * br * 0.92f;
    const float qx = c.x + std::sin (ang) * br * 0.3f, qy = c.y - std::cos (ang) * br * 0.3f;
    g.setColour (juce::Colours::white);
    g.drawLine (qx, qy, px, py, 2.2f);
}

void PsyLook::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float sliderPos,
                                float, float, juce::Slider::SliderStyle, juce::Slider& s)
{
    const auto col = s.findColour (juce::Slider::trackColourId);
    auto r = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced ((float) w * 0.28f, 2.0f);

    g.setColour (dark);
    g.fillRoundedRectangle (r, 3.0f);
    g.setColour (col.withAlpha (0.3f));
    g.drawRoundedRectangle (r, 3.0f, 1.0f);

    const float zeroProp = (float) s.valueToProportionOfLength (juce::jlimit (s.getMinimum(), s.getMaximum(), 0.0));
    const float zeroY = r.getY() + r.getHeight() * (1.0f - zeroProp);
    const float vy = juce::jlimit (r.getY(), r.getBottom(), sliderPos);

    g.setColour (col);
    g.fillRoundedRectangle (r.getX() + 1.0f, juce::jmin (zeroY, vy), r.getWidth() - 2.0f,
                            std::abs (zeroY - vy), 2.0f);

    if (zeroProp > 0.02f && zeroProp < 0.98f)
    {
        g.setColour (col.withAlpha (0.6f));
        g.drawHorizontalLine ((int) zeroY, r.getX(), r.getRight());
    }
    g.setColour (juce::Colours::white);
    g.fillRect (r.getX() - 1.0f, vy - 1.5f, r.getWidth() + 2.0f, 3.0f);
}

void PsyLook::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.5f);
    const auto col = b.findColour (juce::ToggleButton::tickColourId);
    const bool on = b.getToggleState();

    g.setColour (on ? col.withAlpha (0.9f) : dark);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (on ? col.brighter (0.4f) : col.withAlpha (highlighted ? 0.9f : 0.45f));
    g.drawRoundedRectangle (r, 4.0f, 1.3f);
    g.setColour (on ? juce::Colours::black : text);
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred);
}

//==============================================================================
PsyAcidEditor::PsyAcidEditor (PsyAcidProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p),
      kTune (p.apvts, "tune", "TUNE", violet),
      kCut (p.apvts, "cutoff", "CUTOFF", neonGreen),
      kReso (p.apvts, "reso", "RESO", neonGreen),
      kEnv (p.apvts, "envmod", "ENV MOD", neonGreen),
      kDec (p.apvts, "decay", "DECAY", neonGreen),
      kAcc (p.apvts, "accent", "ACCENT", orange),
      kGlide (p.apvts, "glide", "SLIDE", cyan),
      kVol (p.apvts, "volume", "VOLUME", yellow),
      kDist (p.apvts, "dist", "DRIVE", magenta),
      kLfoDepth (p.apvts, "lfo_depth", "DEPTH", cyan),
      kFb (p.apvts, "dly_fb", "FEEDBACK", magenta),
      kMix (p.apvts, "dly_mix", "MIX", magenta),
      kWidth (p.apvts, "dly_width", "PAN WIDTH", cyan),
      kTone (p.apvts, "dly_tone", "TONE", violet),
      kLen (p.apvts, "seq_len", "LENGTH", neonGreen),
      kGate (p.apvts, "gate_len", "GATE", orange),
      kTrans (p.apvts, "transpose", "TRANSPOSE", violet),
      kBpm (p.apvts, "int_bpm", "INT BPM", yellow),
      cWave (p.apvts, "wave", "WAVE"),
      cLfoDiv (p.apvts, "lfo_div", "RATE"),
      cLfoShape (p.apvts, "lfo_shape", "SHAPE"),
      cLfoDest (p.apvts, "lfo_dest", "DEST"),
      cDlyTime (p.apvts, "dly_time", "TIME"),
      cSeqDiv (p.apvts, "seq_div", "RATE"),
      tSeqOn (p.apvts, "seq_on", "SEQ ON", neonGreen),
      tFree (p.apvts, "free_run", "FREE RUN", cyan)
{
    setLookAndFeel (&look);

    for (auto* c : std::initializer_list<juce::Component*> {
             &kTune, &kCut, &kReso, &kEnv, &kDec, &kAcc, &kGlide, &kVol, &kDist, &kLfoDepth,
             &kFb, &kMix, &kWidth, &kTone, &kLen, &kGate, &kTrans, &kBpm,
             &cWave, &cLfoDiv, &cLfoShape, &cLfoDest, &cDlyTime, &cSeqDiv, &tSeqOn, &tFree })
        addAndMakeVisible (c);

    // ---- step grid
    for (int i = 0; i < PA::numSteps; ++i)
    {
        gateB[i].setButtonText ("GATE");
        gateB[i].setColour (juce::ToggleButton::tickColourId, neonGreen);
        accB[i].setButtonText ("ACC");
        accB[i].setColour (juce::ToggleButton::tickColourId, orange);
        sldB[i].setButtonText ("SLIDE");
        sldB[i].setColour (juce::ToggleButton::tickColourId, cyan);

        auto setup = [&] (juce::Slider& s, juce::Colour c)
        {
            s.setSliderStyle (juce::Slider::LinearVertical);
            s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            s.setColour (juce::Slider::trackColourId, c);
            s.setPopupDisplayEnabled (true, true, this);
        };
        setup (noteS[i], yellow);
        setup (distS[i], magenta);
        setup (filtS[i], neonGreen);
        setup (lfoS[i], cyan);

        for (juce::Component* c : std::initializer_list<juce::Component*> {
                 &gateB[i], &accB[i], &sldB[i], &noteS[i], &distS[i], &filtS[i], &lfoS[i] })
            addAndMakeVisible (c);

        btnAtt.push_back (std::make_unique<BA> (p.apvts, PA::sid (i, "gate"), gateB[i]));
        btnAtt.push_back (std::make_unique<BA> (p.apvts, PA::sid (i, "acc"), accB[i]));
        btnAtt.push_back (std::make_unique<BA> (p.apvts, PA::sid (i, "sld"), sldB[i]));
        sldAtt.push_back (std::make_unique<SA> (p.apvts, PA::sid (i, "note"), noteS[i]));
        sldAtt.push_back (std::make_unique<SA> (p.apvts, PA::sid (i, "dist"), distS[i]));
        sldAtt.push_back (std::make_unique<SA> (p.apvts, PA::sid (i, "filt"), filtS[i]));
        sldAtt.push_back (std::make_unique<SA> (p.apvts, PA::sid (i, "lfo"), lfoS[i]));
    }

    // ---- presets
    for (int i = 0; i < 100; ++i)
        presetBox.addItem (Presets::name (i), i + 1);
    presetBox.setSelectedId (proc.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0)
            proc.setCurrentProgram (id - 1);
    };
    prevBtn.onClick = [this] { changePreset (-1); };
    nextBtn.onClick = [this] { changePreset (1); };
    addAndMakeVisible (presetBox);
    addAndMakeVisible (prevBtn);
    addAndMakeVisible (nextBtn);

    setResizable (true, true);
    setResizeLimits (760, 562, 1500, 1110);
    getConstrainer()->setFixedAspectRatio (1000.0 / 740.0);
    setSize (1000, 740);
    startTimerHz (30);
}

PsyAcidEditor::~PsyAcidEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void PsyAcidEditor::changePreset (int delta)
{
    int id = presetBox.getSelectedId();
    if (id <= 0)
        id = proc.getCurrentProgram() + 1;
    id = ((id - 1 + delta + 100) % 100) + 1;
    presetBox.setSelectedId (id, juce::sendNotificationSync);
}

void PsyAcidEditor::timerCallback()
{
    const int s = proc.curStep.load();
    if (s != lastStep)
    {
        lastStep = s;
        repaint (ledStrip);
    }
}

//==============================================================================
void PsyAcidEditor::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff0a0414), 0.0f, 0.0f,
                                             juce::Colour (0xff1d0838), b.getWidth(), b.getHeight(), false));
    g.fillAll();

    // psychedelic mandala in the background
    const auto ctr = b.getCentre();
    const juce::Colour cols[4] = { magenta, violet, cyan, neonGreen };
    for (int i = 0; i < 14; ++i)
    {
        const float rr = b.getHeight() * (0.1f + 0.07f * (float) i);
        g.setColour (cols[i % 4].withAlpha (0.045f));
        g.drawEllipse (ctr.x - rr, ctr.y - rr, rr * 2.0f, rr * 2.0f, 2.0f);
    }

    // title
    g.setColour (neonGreen);
    g.setFont (juce::FontOptions (b.getHeight() * 0.042f, juce::Font::bold));
    g.drawText ("PSY ACID 303", 16, 6, (int) (b.getWidth() * 0.4f), (int) (b.getHeight() * 0.05f),
                juce::Justification::centredLeft);
    g.setColour (magenta);
    g.setFont (juce::FontOptions (b.getHeight() * 0.0175f));
    g.drawText ("TB-303 STYLE  *  GOA / PSYTRANCE  *  ALL BPM SYNCED", 18, (int) (b.getHeight() * 0.055f),
                (int) (b.getWidth() * 0.5f), (int) (b.getHeight() * 0.025f), juce::Justification::centredLeft);

    for (auto& p : panels)
    {
        auto r = p.r.toFloat();
        g.setColour (panel.withAlpha (0.82f));
        g.fillRoundedRectangle (r, 9.0f);
        g.setColour (p.c.withAlpha (0.75f));
        g.drawRoundedRectangle (r.reduced (0.5f), 9.0f, 1.4f);
        g.setColour (p.c);
        g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
        g.drawText (p.title, p.r.getX() + 12, p.r.getY() + 3, p.r.getWidth() - 20, 16, juce::Justification::centredLeft);
    }

    g.setColour (text.withAlpha (0.85f));
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    for (auto& rl : rowLabels)
        g.drawText (rl.second, rl.first, juce::Justification::centredLeft);

    const int cur = proc.curStep.load();
    for (int i = 0; i < PA::numSteps; ++i)
    {
        auto r = ledRects[i].toFloat();
        const bool on = (i == cur);
        if (on)
        {
            g.setColour (neonGreen.withAlpha (0.35f));
            g.fillEllipse (r.expanded (3.0f));
        }
        g.setColour (on ? neonGreen : juce::Colour (0xff2a1850));
        g.fillEllipse (r);
        g.setColour ((i % 4 == 0) ? magenta.withAlpha (0.8f) : violet.withAlpha (0.5f));
        g.drawEllipse (r, 1.0f);
    }
}

void PsyAcidEditor::resized()
{
    panels.clear();
    rowLabels.clear();

    auto area = getLocalBounds().reduced (12);
    auto header = area.removeFromTop ((int) (area.getHeight() * 0.075f));

    auto pr = header.removeFromRight (juce::jmin (400, header.getWidth() / 2)).reduced (0, 8);
    prevBtn.setBounds (pr.removeFromLeft (34));
    nextBtn.setBounds (pr.removeFromRight (34));
    presetBox.setBounds (pr.reduced (6, 0));

    area.removeFromTop (6);
    const int rowAh = (int) (area.getHeight() * 0.245f);
    const int rowBh = (int) (area.getHeight() * 0.215f);
    auto rowA = area.removeFromTop (rowAh);
    area.removeFromTop (8);
    auto rowB = area.removeFromTop (rowBh);
    area.removeFromTop (8);
    auto rowC = area;

    auto addPanel = [this] (juce::Rectangle<int> r, const juce::String& t, juce::Colour c)
    {
        panels.push_back ({ r, t, c });
        return r.reduced (8).withTrimmedTop (18);
    };
    auto cells = [] (juce::Rectangle<int> r, std::initializer_list<juce::Component*> list)
    {
        const int w = r.getWidth() / (int) list.size();
        int x = r.getX();
        for (auto* c : list)
        {
            c->setBounds (x, r.getY(), w, r.getHeight());
            x += w;
        }
    };

    // ---- row A: synth / distortion / lfo
    const int synthW = (int) (rowA.getWidth() * 0.54f), distW = (int) (rowA.getWidth() * 0.11f);
    auto synthR = rowA.removeFromLeft (synthW);
    rowA.removeFromLeft (8);
    auto distR = rowA.removeFromLeft (distW);
    rowA.removeFromLeft (8);
    auto lfoR = rowA;

    cells (addPanel (synthR, "303 SYNTH", neonGreen),
           { &cWave, &kTune, &kCut, &kReso, &kEnv, &kDec, &kAcc, &kGlide, &kVol });
    cells (addPanel (distR, "STEP DIST", magenta), { &kDist });
    cells (addPanel (lfoR, "STEP LFO  (BPM)", cyan), { &cLfoDiv, &cLfoShape, &cLfoDest, &kLfoDepth });

    // ---- row B: pan delay / sequencer
    const int dlyW = (int) (rowB.getWidth() * 0.42f);
    auto dlyR = rowB.removeFromLeft (dlyW);
    rowB.removeFromLeft (8);
    cells (addPanel (dlyR, "PAN DELAY  (BPM)", magenta), { &cDlyTime, &kFb, &kMix, &kWidth, &kTone });
    cells (addPanel (rowB, "SEQUENCER  (DAW SYNC)", yellow),
           { &tSeqOn, &tFree, &kLen, &cSeqDiv, &kGate, &kTrans, &kBpm });

    // ---- row C: step grid
    auto stepR = addPanel (rowC, "STEP SEQUENCER  -  step distortion / step filter / step LFO", violet);
    auto labelCol = stepR.removeFromLeft (66);
    const int cw = stepR.getWidth() / PA::numSteps;
    const int ledH = 14;
    const int btnH = juce::jmax (18, (int) (stepR.getHeight() * 0.085f));
    const int rest = stepR.getHeight() - ledH - 3 * btnH;
    const int hNote = (int) (rest * 0.28f), hOther = (rest - hNote) / 3;

    const int heights[8] = { ledH, btnH, btnH, btnH, hNote, hOther, hOther, hOther };
    const char* names[8] = { "STEP", "GATE", "ACCENT", "SLIDE", "NOTE", "DIST", "FILTER", "LFO" };
    int rowY[8], y = stepR.getY();
    for (int k = 0; k < 8; ++k)
    {
        rowY[k] = y;
        rowLabels.push_back ({ juce::Rectangle<int> (labelCol.getX(), y, labelCol.getWidth(), heights[k]), names[k] });
        y += heights[k];
    }

    for (int i = 0; i < PA::numSteps; ++i)
    {
        const int x = stepR.getX() + i * cw;
        ledRects[i] = juce::Rectangle<int> (x + cw / 2 - 5, rowY[0] + 2, 10, 10);
        gateB[i].setBounds (x + 2, rowY[1] + 1, cw - 4, heights[1] - 2);
        accB[i].setBounds (x + 2, rowY[2] + 1, cw - 4, heights[2] - 2);
        sldB[i].setBounds (x + 2, rowY[3] + 1, cw - 4, heights[3] - 2);
        noteS[i].setBounds (x + 2, rowY[4], cw - 4, heights[4]);
        distS[i].setBounds (x + 2, rowY[5], cw - 4, heights[5]);
        filtS[i].setBounds (x + 2, rowY[6], cw - 4, heights[6]);
        lfoS[i].setBounds (x + 2, rowY[7], cw - 4, heights[7]);
    }
    ledStrip = juce::Rectangle<int> (stepR.getX(), rowY[0] - 2, stepR.getWidth(), ledH + 4);
}
