#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include "processors/SamplerProcessor.h"


class SamplerEditor : public juce::AudioProcessorEditor,
                      public juce::ChangeListener,
                      public juce::ListBoxModel
{
public:
    explicit SamplerEditor (SamplerProcessor&, juce::AudioProcessorValueTreeState&);
    ~SamplerEditor() override;

    void changeListenerCallback (juce::ChangeBroadcaster*) override;
    void paint (juce::Graphics&) override;
    void resized() override;
    
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

private:
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    SamplerProcessor& samplerProcessor;
    juce::AudioProcessorValueTreeState& params;
    
    juce::TextButton playStopButton;
    juce::TextButton openButton;
    juce::TextButton clearButton;
    juce::ListBox filesList;

    juce::ToggleButton bypassToggle;
    std::unique_ptr<ButtonAttachment> bypassAttachment;
    
    juce::TextButton shuffleButton;
    std::unique_ptr<ButtonAttachment> shuffleAttachment;
    
    juce::TextButton loopButton;
    std::unique_ptr<ButtonAttachment> loopAttachment;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::Array<juce::File> loadedFiles;
    
    juce::Slider pitchSlider;
    std::unique_ptr<SliderAttachment> pitchAttachment;
    juce::Label pitchLabel;
    
    juce::Slider levelSlider;
    std::unique_ptr<SliderAttachment> levelAttachment;
    juce::Label levelLabel;

    void playStopButtonClicked();
    void openButtonClicked();
    void clearButtonClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SamplerEditor)
};
