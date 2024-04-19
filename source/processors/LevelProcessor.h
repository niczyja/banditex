#pragma once

#include "ProcessorBase.h"


class LevelProcessor : public ProcessorBase
{
public:
    LevelProcessor();
    
    void processBlock (juce::AudioBuffer<float>& audioBuffer, juce::MidiBuffer&) override;
    void reset() override;
    
    const juce::String getName() const override;
    
private:
    juce::AudioParameterFloat* level;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelProcessor)
};
