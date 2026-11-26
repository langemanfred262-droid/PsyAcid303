#pragma once
#include "Common.h"
#include "Synth.h"

class PsyAcidProcessor : public juce::AudioProcessor
{
public:
    PsyAcidProcessor();
    ~PsyAcidProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Psy Acid 303"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 100; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<int> curStep { -1 };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void triggerStep (long long idx, int len, int transpose);
    void handleMidi (const juce::MidiMessage&, bool seqActive);

    PA::AcidVoice voice;
    PA::DelayLine dlyL, dlyR;
    double fs = 44100.0;

    std::atomic<float>* P[PA::Count] {};
    std::atomic<float>* stepP[PA::numSteps][PA::numStepKeys] {};

    int currentProgram = 0;
    double internalPpq = 0.0;
    long long lastIdx = std::numeric_limits<long long>::min();
    bool wasActive = false, gateOffDone = false, lastSlide = false, curSlide = false;
    int midiTranspose = 0;
    std::vector<int> held;

    // smoothed per-step modulation
    float stepDist = 0, stepFilt = 0, stepLfo = 1;
    float stepDistT = 0, stepFiltT = 0, stepLfoT = 1;
    float smoothCoef = 0.01f;

    // lfo
    juce::Random rng;
    long long lastCycle = std::numeric_limits<long long>::min();
    float shVal = 0.0f;

    // delay
    float dlySmooth = 10000.0f, lpL = 0.0f, lpR = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PsyAcidProcessor)
};
