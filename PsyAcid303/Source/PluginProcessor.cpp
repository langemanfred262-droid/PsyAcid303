#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"

using namespace PA;

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout PsyAcidProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout L;

    auto F = [&] (const String& id, const String& nm, float lo, float hi, float def, float skew = 1.0f)
    {
        L.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, nm,
                                                      NormalisableRange<float> (lo, hi, 0.0f, skew), def));
    };
    auto I = [&] (const String& id, const String& nm, int lo, int hi, int def)
    {
        L.add (std::make_unique<AudioParameterInt> (ParameterID { id, 1 }, nm, lo, hi, def));
    };
    auto B = [&] (const String& id, const String& nm, bool def)
    {
        L.add (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, nm, def));
    };
    auto C = [&] (const String& id, const String& nm, const StringArray& items, int def)
    {
        L.add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, nm, items, def));
    };

    StringArray divs;
    for (int i = 0; i < numDivs; ++i)
        divs.add (divNames[i]);

    F ("cutoff", "Cutoff", 20.0f, 12000.0f, 800.0f, 0.3f);
    F ("reso", "Resonance", 0.0f, 1.0f, 0.55f);
    F ("envmod", "Env Mod", 0.0f, 1.0f, 0.6f);
    F ("decay", "Decay", 0.05f, 2.0f, 0.35f, 0.5f);
    F ("accent", "Accent", 0.0f, 1.0f, 0.6f);
    F ("glide", "Slide", 0.005f, 0.4f, 0.06f, 0.5f);
    C ("wave", "Waveform", StringArray { "Saw", "Square" }, 0);
    F ("tune", "Tune", -12.0f, 12.0f, 0.0f);
    F ("volume", "Volume", 0.0f, 1.0f, 0.7f);
    F ("dist", "Distortion", 0.0f, 1.0f, 0.2f);

    C ("lfo_div", "LFO Rate", divs, 1);
    C ("lfo_shape", "LFO Shape", StringArray { "Sine", "Triangle", "Saw", "Square", "S&H" }, 0);
    C ("lfo_dest", "LFO Dest", StringArray { "Cutoff", "Resonance", "Distortion", "Pitch" }, 0);
    F ("lfo_depth", "LFO Depth", 0.0f, 1.0f, 0.3f);

    C ("dly_time", "Delay Time", divs, 8);
    F ("dly_fb", "Delay Feedback", 0.0f, 0.95f, 0.4f);
    F ("dly_mix", "Delay Mix", 0.0f, 1.0f, 0.25f);
    F ("dly_width", "Delay Pan Width", 0.0f, 1.0f, 0.8f);
    F ("dly_tone", "Delay Tone", 500.0f, 16000.0f, 6000.0f, 0.4f);

    B ("seq_on", "Sequencer On", true);
    I ("seq_len", "Seq Length", 1, 16, 16);
    C ("seq_div", "Seq Rate", divs, 4);
    B ("free_run", "Free Run", true);
    F ("gate_len", "Gate Length", 0.1f, 1.0f, 0.55f);
    I ("transpose", "Transpose", -12, 12, 0);
    I ("int_bpm", "Internal BPM", 60, 200, 145);

    for (int i = 0; i < numSteps; ++i)
    {
        const String n = "S" + String (i + 1) + " ";
        I (sid (i, "note"), n + "Note", -12, 24, 0);
        B (sid (i, "gate"), n + "Gate", (i % 4) != 0);
        B (sid (i, "acc"), n + "Accent", false);
        B (sid (i, "sld"), n + "Slide", false);
        F (sid (i, "dist"), n + "Distortion", 0.0f, 1.0f, 0.0f);
        F (sid (i, "filt"), n + "Filter", -1.0f, 1.0f, 0.0f);
        F (sid (i, "lfo"), n + "LFO", 0.0f, 1.0f, 1.0f);
    }
    return L;
}

//==============================================================================
PsyAcidProcessor::PsyAcidProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    for (int i = 0; i < Count; ++i)
        P[i] = apvts.getRawParameterValue (ids[i]);

    for (int s = 0; s < numSteps; ++s)
        for (int k = 0; k < numStepKeys; ++k)
            stepP[s][k] = apvts.getRawParameterValue (sid (s, stepKeys[k]));

    held.reserve (128);
}

bool PsyAcidProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PsyAcidProcessor::prepareToPlay (double sr, int)
{
    fs = sr;
    voice.prepare (sr);
    dlyL.prepare ((int) (6.0 * sr));
    dlyR.prepare ((int) (6.0 * sr));
    smoothCoef = 1.0f - std::exp (-1.0f / (0.004f * (float) sr));
    lastIdx = std::numeric_limits<long long>::min();
    wasActive = false;
    lpL = lpR = 0.0f;
    held.clear();
    midiTranspose = 0;
}

//==============================================================================
void PsyAcidProcessor::handleMidi (const juce::MidiMessage& m, bool seqActive)
{
    if (m.isNoteOn())
    {
        const int n = m.getNoteNumber();
        const bool legato = ! held.empty();
        held.push_back (n);
        if (! seqActive)
            voice.noteOn ((float) n, m.getVelocity() > 100, legato);
    }
    else if (m.isNoteOff())
    {
        const int n = m.getNoteNumber();
        for (size_t i = 0; i < held.size(); ++i)
            if (held[i] == n)
            {
                held.erase (held.begin() + (long) i);
                break;
            }
        if (! seqActive)
        {
            if (held.empty())
                voice.noteOff();
            else
                voice.noteOn ((float) held.back(), false, true);
        }
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff())
    {
        held.clear();
        if (! seqActive)
            voice.noteOff();
    }
    // held key transposes the pattern relative to middle C (MIDI 60)
    midiTranspose = held.empty() ? 0 : juce::jlimit (-24, 24, held.back() - 60);
}

void PsyAcidProcessor::triggerStep (long long idx, int len, int transpose)
{
    const int si = (int) (((idx % len) + len) % len);
    curStep.store (si);

    auto gv = [&] (int k) { return stepP[si][k]->load(); };
    stepDistT = gv (4);
    stepFiltT = gv (5);
    stepLfoT = gv (6);

    const bool gate = gv (1) > 0.5f, acc = gv (2) > 0.5f, sld = gv (3) > 0.5f;

    if (gate)
    {
        const float note = 36.0f + (float) transpose + gv (0) + (float) midiTranspose;
        const bool legato = lastSlide && voice.isGated();
        voice.noteOn (note, acc, legato);
    }
    else
    {
        voice.noteOff();
    }
    lastSlide = gate && sld;
    curSlide = lastSlide;
}

static float lfoShapeValue (int shape, float ph, float sh)
{
    switch (shape)
    {
        case 0: return std::sin (juce::MathConstants<float>::twoPi * ph);
        case 1: return 1.0f - 4.0f * std::abs (ph - 0.5f);
        case 2: return 2.0f * ph - 1.0f;
        case 3: return ph < 0.5f ? 1.0f : -1.0f;
        default: return sh;
    }
}

//==============================================================================
void PsyAcidProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    buffer.clear();
    if (buffer.getNumChannels() < 2)
        return;

    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getWritePointer (1);

    auto ld = [this] (int i) { return P[i]->load(); };

    // ---- host transport / tempo (everything runs on the DAW's BPM grid)
    double bpm = ld (Bpm);
    double ppq0 = internalPpq;
    bool hostPlaying = false;
    if (auto* ph = getPlayHead())
        if (auto info = ph->getPosition())
        {
            if (auto b = info->getBpm())
                bpm = *b;
            if (info->getIsPlaying())
                if (auto pp = info->getPpqPosition())
                {
                    ppq0 = *pp;
                    hostPlaying = true;
                }
        }
    bpm = juce::jlimit (20.0, 400.0, bpm);
    const double ppqInc = bpm / 60.0 / fs;

    // ---- parameters
    const float cutoff = ld (Cutoff), reso = ld (Reso), envmod = ld (EnvMod), accAmt = ld (Accent);
    const int wave = (int) ld (Wave);
    const float tune = ld (Tune), vol = ld (Volume), distBase = ld (Dist);
    const int lfoDivIdx = juce::jlimit (0, numDivs - 1, (int) ld (LfoDiv));
    const int lfoShape = (int) ld (LfoShape), lfoDest = (int) ld (LfoDest);
    const float lfoDepth = ld (LfoDepth);
    const int dlyIdx = juce::jlimit (0, numDivs - 1, (int) ld (DlyTime));
    const float dFb = ld (DlyFb), dMix = ld (DlyMix), dWidth = ld (DlyWidth);
    const float toneA = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * ld (DlyTone) / (float) fs);
    const int len = juce::jlimit (1, numSteps, (int) ld (SeqLen));
    const double stepBeats = divBeats[juce::jlimit (0, numDivs - 1, (int) ld (SeqDiv))];
    const int transposeSeq = (int) ld (Transpose);
    const double gateLen = ld (GateLen);
    const bool seqActive = ld (SeqOn) > 0.5f && (hostPlaying || ld (FreeRun) > 0.5f);

    if (seqActive && ! wasActive)
        lastIdx = std::numeric_limits<long long>::min();
    if (! seqActive && wasActive)
    {
        voice.noteOff();
        curStep.store (-1);
        lastSlide = curSlide = false;
    }
    wasActive = seqActive;

    voice.setCoefs (ld (Decay), ld (Glide));

    const float dTarget = juce::jlimit (8.0f, (float) dlyL.size() - 4.0f,
                                        (float) (divBeats[dlyIdx] * 60.0 / bpm * fs));
    const double lfoBeats = divBeats[lfoDivIdx];

    auto midiIt = midi.begin();

    for (int i = 0; i < n; ++i)
    {
        while (midiIt != midi.end() && (*midiIt).samplePosition <= i)
        {
            handleMidi ((*midiIt).getMessage(), seqActive);
            ++midiIt;
        }

        const double ppq = ppq0 + (double) i * ppqInc;

        if (seqActive)
        {
            const double pos = ppq / stepBeats;
            const long long idx = (long long) std::floor (pos);
            if (idx != lastIdx)
            {
                lastIdx = idx;
                gateOffDone = false;
                triggerStep (idx, len, transposeSeq);
            }
            if (! gateOffDone && ! curSlide && (pos - (double) idx) >= gateLen)
            {
                voice.noteOff();
                gateOffDone = true;
            }
        }

        // smoothed step modulation (step distortion / filter / LFO)
        stepDist += (stepDistT - stepDist) * smoothCoef;
        stepFilt += (stepFiltT - stepFilt) * smoothCoef;
        stepLfo += (stepLfoT - stepLfo) * smoothCoef;

        // BPM-synced LFO
        const double lp = ppq / lfoBeats;
        const long long cyc = (long long) std::floor (lp);
        if (cyc != lastCycle)
        {
            lastCycle = cyc;
            shVal = rng.nextFloat() * 2.0f - 1.0f;
        }
        const float phase = (float) (lp - (double) cyc);
        const float lv = lfoShapeValue (lfoShape, phase, shVal) * lfoDepth * stepLfo;

        float cutMod = stepFilt * 2.5f, resoAdd = 0.0f, distAdd = 0.0f, pitch = 0.0f;
        switch (lfoDest)
        {
            case 0: cutMod += lv * 2.5f; break;
            case 1: resoAdd = lv * 0.4f; break;
            case 2: distAdd = lv * 0.5f; break;
            default: pitch = lv * 1.5f; break;
        }
        const float dist = juce::jlimit (0.0f, 1.0f, distBase + stepDist + distAdd);

        const float y = voice.process (cutoff, reso + resoAdd, envmod, accAmt, wave, tune + pitch, cutMod, dist);

        // ---- BPM-synced ping-pong pan delay
        dlySmooth += (dTarget - dlySmooth) * 0.0008f;
        const float dl = dlyL.read (dlySmooth);
        const float dr = dlyR.read (dlySmooth);
        lpL += toneA * (dr - lpL);
        dlyL.write (y + std::tanh (lpL * dFb));
        dlyR.write (dl);
        const float a = 0.5f + 0.5f * dWidth, b = 0.5f - 0.5f * dWidth;
        const float wetL = dl * a + dr * b;
        const float wetR = dr * a + dl * b;

        outL[i] = std::tanh ((y + wetL * dMix) * vol * 1.4f);
        outR[i] = std::tanh ((y + wetR * dMix) * vol * 1.4f);
    }

    internalPpq = ppq0 + (double) n * ppqInc;
}

//==============================================================================
juce::AudioProcessorEditor* PsyAcidProcessor::createEditor()
{
    return new PsyAcidEditor (*this);
}

void PsyAcidProcessor::setCurrentProgram (int index)
{
    currentProgram = juce::jlimit (0, 99, index);
    Presets::apply (currentProgram, apvts);
}

const juce::String PsyAcidProcessor::getProgramName (int index)
{
    return Presets::name (index);
}

void PsyAcidProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.setProperty ("program", currentProgram, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, dest);
}

void PsyAcidProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto vt = juce::ValueTree::fromXml (*xml);
            currentProgram = juce::jlimit (0, 99, (int) vt.getProperty ("program", 0));
            apvts.replaceState (vt);
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PsyAcidProcessor();
}
