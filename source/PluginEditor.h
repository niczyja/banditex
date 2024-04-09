#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"



//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor,
                     public juce::ChangeListener
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    // Add a member function to update the list of loaded file names and highlight the currently played file
    void updateLoadedFilesList();
    // adding a change listener callback to listen to the broadcaster notifys
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    // Adding button for file loading
    juce::TextButton mLoadButton {"Load Files"};
    // Label to display loaded files
    juce::Label loadedFilesLabel;
    
    

    // Down JUCE stuff generated previously
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
