#pragma once

#include "melatonin_inspector/melatonin_inspector.h"
#include "PluginProcessor.h"
#include "BinaryData.h"


class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& pluginProcessor;
    
    std::unique_ptr<juce::GenericAudioProcessorEditor> globalParams;
    std::unique_ptr<melatonin::Inspector> inspector;
    std::unique_ptr<juce::TextButton> inspectButton;
    
    juce::Component headerComp { "Global" };
    juce::Component procComp { "Processors" };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
