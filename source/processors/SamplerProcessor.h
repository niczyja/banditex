#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "ProcessorBase.h"


class SamplerProcessor : public ProcessorBase
{
public:
    SamplerProcessor();
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer) override;
    void reset() override;
    
    juce::AudioProcessorEditor* createEditor() override;
    const juce::String getName() const override;

    bool isPlaying = false;
    bool isLooping = false;

    bool getIsShuffling();
    void setIsShuffling(bool shouldShuffle);
    
    int getCurrentSampleIndex();
    void readFiles(juce::Array<juce::File>& files);
    
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
    
    juce::AudioFormatManager formatManager;
    juce::AudioSampleBuffer samplesBuffer;
    std::vector<SampleSpec> samplesSpecs;

    int currentPosition = 0;
    int currentSampleIndex = -1;
    void advanceToNextSample();
    
    bool isShuffling = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerProcessor)
};
