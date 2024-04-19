#pragma once

#include "ProcessorBase.h"


class SamplerProcessor : public ProcessorBase
{
public:
    SamplerProcessor();
    
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer& midiBuffer) override;

    const juce::String getName() const override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerProcessor)
};
