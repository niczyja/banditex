#pragma once

#include "ProcessorBase.h"


class GainProcessor : public ProcessorBase
{
public:
    GainProcessor();
    
    void processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer&) override;
    void reset() override;
    
    const juce::String getName() const override;

private:
    juce::AudioParameterFloat* gain;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainProcessor)
};
