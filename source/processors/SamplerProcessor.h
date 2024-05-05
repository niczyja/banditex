#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "ProcessorBase.h"


class SamplerProcessor : public ProcessorBase, juce::AudioProcessorValueTreeState::Listener
{
public:
    SamplerProcessor();
    ~SamplerProcessor() override;
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer) override;
    void reset() override;
    
    juce::AudioProcessorEditor* createEditor() override;
    juce::AudioProcessorParameter* getBypassParameter() const override;
    const juce::String getName() const override;
    
    void parameterChanged (const juce::String& parameterID, float newValue) override;

    int getCurrentSampleIndex();
    void readFiles(juce::Array<juce::File>& files);
    const std::vector<juce::AudioBuffer<float>>& getWaveformPeaks() const { return waveformPeaks; }
    
private:
    struct SampleSpec
    {
        int ordinal;
        int start;
        int end;
        float gain = 1.0;
        bool bypass = false;
        bool operator < (const SampleSpec& rhs) const { return start < rhs.start; }
    };
    
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioFormatManager formatManager;
    juce::AudioSampleBuffer samplesBuffer;
    std::vector<SampleSpec> samplesSpecs;

    int currentPosition = 0;
    int currentSampleIndex = -1;
    void advanceToNextSample();
    void setIsShuffling(bool shouldShuffle);
    
    std::vector<juce::AudioBuffer<float>> waveformPeaks;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerProcessor)
};
