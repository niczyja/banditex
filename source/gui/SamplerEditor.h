#pragma once

#include "processors/SamplerProcessor.h"


class SamplerEditor : public juce::AudioProcessorEditor
{
public:
    explicit SamplerEditor (SamplerEditor&);
    ~SamplerEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SamplerProcessor& samplerProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerEditor)
};
