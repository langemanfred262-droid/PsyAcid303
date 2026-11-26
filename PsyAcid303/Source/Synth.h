#pragma once
#include "Common.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace PA
{
inline float polyBlep (float t, float dt)
{
    if (t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.0f;
    }
    if (t > 1.0f - dt)
    {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return 0.0f;
}

//==============================================================================
struct DelayLine
{
    std::vector<float> buf;
    int w = 0;

    void prepare (int size)
    {
        buf.assign ((size_t) std::max (size, 16), 0.0f);
        w = 0;
    }

    int size() const { return (int) buf.size(); }

    void write (float x)
    {
        buf[(size_t) w] = x;
        if (++w >= size())
            w = 0;
    }

    float read (float d) const
    {
        const int sz = size();
        float rp = (float) w - std::max (d, 1.0f);
        while (rp < 0.0f)
            rp += (float) sz;
        int i0 = (int) rp;
        if (i0 >= sz)
            i0 -= sz;
        int i1 = i0 + 1;
        if (i1 >= sz)
            i1 = 0;
        const float f = rp - (float) std::floor (rp);
        return buf[(size_t) i0] + (buf[(size_t) i1] - buf[(size_t) i0]) * f;
    }
};

//==============================================================================
// Monophonic TB-303 style voice: saw/square, resonant 4-pole ladder filter,
// decay envelope, accent, slide (legato glide) and a post-filter distortion.
struct AcidVoice
{
    double fs = 44100.0;
    float phase = 0.0f, freq = 110.0f, target = 110.0f;
    bool gate = false, gliding = false, accent = false;
    float amp = 0.0f, env = 0.0f;
    float s[4] = { 0, 0, 0, 0 };
    float y4 = 0.0f;
    float envCoef = 0.999f, envCoefAcc = 0.998f, glideCoef = 0.01f, atk = 0.01f, rel = 0.01f;

    void prepare (double sr)
    {
        fs = sr;
        atk = 1.0f - std::exp (-1.0f / (0.0015f * (float) fs));
        rel = 1.0f - std::exp (-1.0f / (0.006f * (float) fs));
        reset();
    }

    void reset()
    {
        phase = 0.0f;
        freq = target = 110.0f;
        gate = gliding = accent = false;
        amp = env = y4 = 0.0f;
        for (auto& x : s)
            x = 0.0f;
    }

    void setCoefs (float decaySec, float glideSec)
    {
        const float f = (float) fs;
        envCoef = std::exp (-1.0f / (std::max (0.01f, decaySec) * f));
        envCoefAcc = std::exp (-1.0f / (std::max (0.01f, decaySec * 0.35f) * f));
        glideCoef = 1.0f - std::exp (-1.0f / (std::max (0.002f, glideSec) * f));
    }

    bool isGated() const { return gate; }

    void noteOn (float midi, bool acc, bool legato)
    {
        target = 440.0f * std::exp2 ((midi - 69.0f) / 12.0f);
        accent = acc;
        if (legato && amp > 0.001f)
        {
            gliding = true; // slide: no retrigger, glide to new pitch
        }
        else
        {
            gliding = false;
            freq = target;
            env = 1.0f;
        }
        gate = true;
    }

    void noteOff() { gate = false; }

    float process (float cutoff, float reso, float envmod, float accAmt, int wave,
                   float semis, float cutOct, float dist)
    {
        if (gliding)
            freq += (target - freq) * glideCoef;
        else
            freq = target;

        const float f = freq * std::exp2 (semis / 12.0f);
        const float inc = std::min (f / (float) fs, 0.45f);

        float osc;
        if (wave == 0)
        {
            osc = -(2.0f * phase - 1.0f - polyBlep (phase, inc)); // falling saw
        }
        else
        {
            const float t2 = phase < 0.5f ? phase + 0.5f : phase - 0.5f;
            osc = (phase < 0.5f ? 1.0f : -1.0f) + polyBlep (phase, inc) - polyBlep (t2, inc);
        }
        phase += inc;
        if (phase >= 1.0f)
            phase -= 1.0f;

        amp += ((gate ? 1.0f : 0.0f) - amp) * (gate ? atk : rel);

        const float e = env;
        env *= accent ? envCoefAcc : envCoef;

        const float eAmt = envmod * e * (accent ? 1.0f + accAmt * 0.7f : 1.0f);
        float fc = cutoff * std::exp2 (eAmt * 3.5f + cutOct);
        fc = juce::jlimit (20.0f, std::min (18000.0f, 0.45f * (float) fs), fc);

        const float rz = juce::jlimit (0.0f, 1.0f, reso + (accent ? accAmt * 0.15f : 0.0f));
        const float k = rz * 3.9f;

        const float g = std::tan (juce::MathConstants<float>::pi * fc / (float) fs);
        const float G = g / (1.0f + g);

        float u = std::tanh (osc * 0.8f - k * y4);
        for (int i = 0; i < 4; ++i)
        {
            const float v = (u - s[i]) * G;
            const float y = v + s[i];
            s[i] = y + v;
            u = y;
        }
        y4 = u;

        float out = u * (1.0f + k * 0.3f) * (accent ? 1.0f + accAmt * 0.6f : 1.0f);

        if (dist > 0.001f)
        {
            const float drive = 1.0f + dist * 25.0f;
            const float bias = 0.2f * dist;
            out = std::tanh (drive * out + bias) - std::tanh (bias);
            out *= 0.9f / (1.0f + dist * 0.5f);
        }
        return out * amp * 0.6f;
    }
};
} // namespace PA
