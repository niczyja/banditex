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
    
    int getCurrentFileIndex();
    void readFiles(juce::Array<juce::File>& files);
    
private:
    juce::AudioFormatManager formatManager;
    std::vector<juce::AudioSampleBuffer> fileBuffers;
    int position;
    
    int currentBufferIndex = -1;
    void advanceBufferIndex();
    
    bool isShuffling = false;
    std::vector<int> bufferOrder;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerProcessor)
};
