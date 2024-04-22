#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include "processors/SamplerProcessor.h"


class SamplerEditor : public juce::AudioProcessorEditor,
                      public juce::ChangeListener,
                      public juce::ListBoxModel
{
public:
    explicit SamplerEditor (SamplerProcessor&);
    ~SamplerEditor() override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void paint (juce::Graphics&) override;
    void resized() override;
    
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

private:
    SamplerProcessor& samplerProcessor;
    
    juce::TextButton playStopButton;
    juce::TextButton shuffleButton;
    juce::TextButton loopButton;
    juce::TextButton openButton;
    juce::TextButton clearButton;
    juce::ListBox filesList;

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Array<juce::File> loadedFiles;
    
    void playStopButtonClicked();
    void shuffleButtonClicked();
    void loopButtonClicked();
    void openButtonClicked();
    void clearButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerEditor)
};
