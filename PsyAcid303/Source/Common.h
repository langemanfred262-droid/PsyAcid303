#pragma once
#include <juce_audio_utils/juce_audio_utils.h>

namespace PA
{
constexpr int numSteps = 16;
constexpr int numDivs = 13;

// Note divisions (BPM synced): names and length in quarter-note beats
inline constexpr const char* divNames[numDivs] = { "1/1", "1/2", "1/4", "1/8", "1/16", "1/32", "1/2.",
                                                   "1/4.", "1/8.", "1/16.", "1/4T", "1/8T", "1/16T" };
inline constexpr float divBeats[numDivs] = { 4.f, 2.f, 1.f, 0.5f, 0.25f, 0.125f, 3.f,
                                             1.5f, 0.75f, 0.375f, 2.f / 3.f, 1.f / 3.f, 1.f / 6.f };

enum PIdx
{
    Cutoff, Reso, EnvMod, Decay, Accent, Glide, Wave, Tune, Volume, Dist,
    LfoDiv, LfoShape, LfoDest, LfoDepth,
    DlyTime, DlyFb, DlyMix, DlyWidth, DlyTone,
    SeqOn, SeqLen, SeqDiv, FreeRun, GateLen, Transpose, Bpm,
    Count
};

inline constexpr const char* ids[Count] = {
    "cutoff", "reso", "envmod", "decay", "accent", "glide", "wave", "tune", "volume", "dist",
    "lfo_div", "lfo_shape", "lfo_dest", "lfo_depth",
    "dly_time", "dly_fb", "dly_mix", "dly_width", "dly_tone",
    "seq_on", "seq_len", "seq_div", "free_run", "gate_len", "transpose", "int_bpm"
};

// per-step parameter keys: note, gate, accent, slide, distortion, filter, lfo
inline constexpr int numStepKeys = 7;
inline constexpr const char* stepKeys[numStepKeys] = { "note", "gate", "acc", "sld", "dist", "filt", "lfo" };

inline juce::String sid (int step, const char* key)
{
    return "s" + juce::String (step + 1) + "_" + key;
}
} // namespace PA
