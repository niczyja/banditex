#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class ProcessorBase : public juce::AudioProcessor, public juce::ChangeBroadcaster
{
public:
    ProcessorBase()
        : AudioProcessor
            (BusesProperties()
                #if ! JucePlugin_IsMidiEffect
                #if ! JucePlugin_IsSynth
                .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                #endif
                .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                #endif
            )
    {}
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override { juce::ignoreUnused (sampleRate, samplesPerBlock); }
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer) override { juce::ignoreUnused (audioBuffer, midiBuffer); }

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    
    const juce::String getName() const override { return {}; }
    
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override { juce::ignoreUnused (index); }
    const juce::String getProgramName (int index) override { juce::ignoreUnused (index); return {}; }
    void changeProgramName (int index, const juce::String& newName) override { juce::ignoreUnused (index, newName); }
    
    void getStateInformation (juce::MemoryBlock& destData) override { juce::ignoreUnused (destData); }
    void setStateInformation (const void* data, int sizeInBytes) override { juce::ignoreUnused (data, sizeInBytes); }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorBase)
};
