#pragma once
#include "Common.h"

// 100 internal Psy / Goa presets: 10 categories x 10 variations.
// Each preset sets the sound, LFO, pan delay AND a full 16-step pattern
// (notes, gate, accent, slide, step distortion, step filter, step LFO).
namespace Presets
{
struct Cat
{
    const char* name;
    float cutoff, reso, envmod, decay, dist;
    int wave, lfoDiv, lfoShape, lfoDest;
    float lfoDepth;
    int dlyDiv;
    float fb, mix, width;
    int pat;
    float acc, glide;
};

inline const Cat cats[10] = {
    { "Goa Classic",      700.f, .55f, .60f, .35f, .15f, 0, 1, 0, 0, .30f,  8, .40f, .25f, .80f, 0, .60f, .06f },
    { "Full-On Acid",     900.f, .75f, .75f, .25f, .30f, 0, 2, 1, 0, .35f,  8, .45f, .30f, 1.0f, 1, .80f, .05f },
    { "Dark Forest",      400.f, .60f, .50f, .45f, .45f, 1, 0, 1, 1, .40f,  7, .55f, .35f, .90f, 0, .50f, .08f },
    { "Hi-Tech Squelch", 1200.f, .90f, .85f, .15f, .25f, 0, 3, 3, 0, .50f, 12, .35f, .30f, .70f, 2, .90f, .03f },
    { "Progressive Psy",  600.f, .45f, .50f, .50f, .10f, 0, 1, 0, 0, .25f,  8, .50f, .30f, .80f, 3, .40f, .10f },
    { "Trippy Delay",    1000.f, .65f, .55f, .30f, .20f, 0, 3, 4, 0, .45f, 11, .70f, .50f, 1.0f, 1, .60f, .12f },
    { "Raw Distortion",   800.f, .50f, .70f, .30f, .85f, 1, 4, 2, 2, .50f,  3, .30f, .20f, .60f, 2, .70f, .04f },
    { "Rolling Bass",     350.f, .40f, .45f, .40f, .20f, 0, 2, 0, 0, .20f,  9, .25f, .15f, .50f, 0, .50f, .05f },
    { "Sunrise Goa",     1500.f, .55f, .50f, .50f, .10f, 0, 0, 0, 3, .15f,  7, .50f, .35f, 1.0f, 4, .50f, .15f },
    { "Twilight Zone",    500.f, .80f, .90f, .70f, .50f, 1, 6, 0, 1, .50f,  6, .65f, .45f, 1.0f, 4, .80f, .20f }
};

inline constexpr const char* suffixes[10] = { "Spiral", "Vortex", "Mantra", "Shiva", "Lotus",
                                              "Cosmic", "Chakra", "Nebula", "Orbit", "Mandala" };

inline juce::String name (int i)
{
    i = juce::jlimit (0, 99, i);
    return juce::String (i + 1).paddedLeft ('0', 3) + "  " + cats[i / 10].name + " - " + suffixes[i % 10];
}

inline void setReal (juce::AudioProcessorValueTreeState& a, const juce::String& id, float v)
{
    if (auto* p = a.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

inline void apply (int index, juce::AudioProcessorValueTreeState& a)
{
    using juce::jlimit;
    index = jlimit (0, 99, index);
    const Cat& c = cats[index / 10];
    const int v = index % 10;
    juce::Random r (7919 + index * 104729);
    auto jit = [&] (float amt) { return (r.nextFloat() - 0.5f) * 2.0f * amt; };

    // ---- sound
    setReal (a, "cutoff", jlimit (60.f, 9000.f, c.cutoff * std::exp2 (jit (1.0f) + ((float) v - 4.5f) * 0.06f)));
    setReal (a, "reso", jlimit (0.15f, 0.95f, c.reso + jit (0.12f)));
    setReal (a, "envmod", jlimit (0.1f, 1.0f, c.envmod + jit (0.15f)));
    setReal (a, "decay", jlimit (0.06f, 1.5f, c.decay * std::exp2 (jit (0.7f))));
    setReal (a, "accent", jlimit (0.1f, 1.0f, c.acc + jit (0.15f)));
    setReal (a, "glide", jlimit (0.01f, 0.35f, c.glide * std::exp2 (jit (0.6f))));
    setReal (a, "dist", jlimit (0.0f, 1.0f, c.dist + jit (0.1f)));
    setReal (a, "wave", (float) ((c.wave + (v == 7 ? 1 : 0)) % 2));
    setReal (a, "tune", 0.0f);

    // ---- LFO (BPM synced)
    setReal (a, "lfo_div", (float) jlimit (0, 12, c.lfoDiv + (v % 3) - 1));
    setReal (a, "lfo_shape", (float) ((c.lfoShape + (v >= 5 ? 1 : 0)) % 5));
    setReal (a, "lfo_dest", (float) c.lfoDest);
    setReal (a, "lfo_depth", jlimit (0.0f, 1.0f, c.lfoDepth + jit (0.15f)));

    // ---- pan delay (BPM synced)
    static const int altDly[4] = { 7, 8, 11, 3 };
    setReal (a, "dly_time", (float) (v < 6 ? c.dlyDiv : altDly[v - 6]));
    setReal (a, "dly_fb", jlimit (0.1f, 0.9f, c.fb + jit (0.1f)));
    setReal (a, "dly_mix", jlimit (0.05f, 0.7f, c.mix + jit (0.1f)));
    setReal (a, "dly_width", jlimit (0.2f, 1.0f, c.width + jit (0.15f)));
    setReal (a, "dly_tone", 3000.0f + r.nextFloat() * 8000.0f);

    // ---- sequencer
    static const int trans[6] = { 0, -2, 2, -5, 3, 0 };
    setReal (a, "seq_on", 1.0f);
    setReal (a, "seq_div", 4.0f); // 1/16
    setReal (a, "seq_len", (float) (v % 5 == 3 ? 12 : 16));
    setReal (a, "gate_len", 0.5f + r.nextFloat() * 0.2f);
    setReal (a, "transpose", (float) trans[v % 6]);

    // ---- 16-step pattern
    static const int sc[5] = { 0, 3, 5, 7, 10 }; // minor pentatonic
    for (int i = 0; i < 16; ++i)
    {
        int note = 0;
        bool gate = true, acc = false, sld = false;
        switch (c.pat)
        {
            case 0: // rolling off-beat bass
                gate = (i % 4) != 0;
                if (i % 4 == 3 && r.nextFloat() < 0.45f)
                    note = r.nextBool() ? 12 : 7;
                acc = (i % 8 == 6);
                break;
            case 1: // random acid line
                gate = r.nextFloat() < 0.82f;
                note = sc[r.nextInt (5)] + (r.nextFloat() < 0.3f ? 12 : 0);
                acc = r.nextFloat() < 0.25f;
                sld = r.nextFloat() < 0.2f;
                break;
            case 2: // 16th jumps
            {
                const int q = i % 4;
                note = q == 0 ? 0 : q == 1 ? 12 : q == 2 ? 0 : sc[1 + r.nextInt (4)];
                acc = (q == 1);
                sld = (i % 8 == 7);
                break;
            }
            case 3: // sparse off-beat
                gate = (i % 2) == 1 || (i % 8 == 4);
                note = (i % 8 == 7 && r.nextBool()) ? 12 : 0;
                acc = (i % 4 == 3);
                break;
            default: // melodic
                gate = r.nextFloat() < 0.72f;
                note = sc[r.nextInt (5)] + 12 * r.nextInt (2) - (r.nextFloat() < 0.3f ? 12 : 0);
                sld = r.nextFloat() < 0.3f;
                acc = r.nextFloat() < 0.2f;
                break;
        }
        note = jlimit (-12, 24, note);

        // step distortion
        float d;
        if (v % 3 == 0)      d = (float) i / 15.0f * 0.5f;
        else if (v % 3 == 1) d = acc ? 0.5f : 0.0f;
        else                 d = r.nextFloat() * 0.35f;

        // step filter
        float f;
        if (v % 4 == 0)      f = -0.3f + (float) i / 15.0f * 1.1f;
        else if (v % 4 == 1) f = (float) (i % 4) / 3.0f - 0.3f;
        else if (v % 4 == 2) f = jit (0.6f);
        else                 f = std::sin (juce::MathConstants<float>::twoPi * (float) i / 16.0f) * 0.7f;

        // step LFO depth
        const float l = (v % 2 == 0) ? 1.0f : 0.3f + r.nextFloat() * 0.7f;

        setReal (a, PA::sid (i, "note"), (float) note);
        setReal (a, PA::sid (i, "gate"), gate ? 1.0f : 0.0f);
        setReal (a, PA::sid (i, "acc"), acc ? 1.0f : 0.0f);
        setReal (a, PA::sid (i, "sld"), sld ? 1.0f : 0.0f);
        setReal (a, PA::sid (i, "dist"), jlimit (0.0f, 1.0f, d));
        setReal (a, PA::sid (i, "filt"), jlimit (-1.0f, 1.0f, f));
        setReal (a, PA::sid (i, "lfo"), l);
    }
}
} // namespace Presets
