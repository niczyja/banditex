
#include "SamplerEditor.h"


SamplerEditor::SamplerEditor(SamplerProcessor& p, juce::AudioProcessorValueTreeState& vst)
    : AudioProcessorEditor(&p), samplerProcessor(p), params(vst)
{
    samplerProcessor.addChangeListener(this);
    
    addAndMakeVisible(bypassToggle);
    bypassToggle.setButtonText("Bypass");
    bypassAttachment.reset(new ButtonAttachment(params, "bypass", bypassToggle));
    
    addAndMakeVisible(playStopButton);
    playStopButton.setButtonText("Play");
    playStopButton.setClickingTogglesState(true);
    playStopButton.onClick = [this] { playStopButtonClicked(); };
    
    addAndMakeVisible(shuffleButton);
    shuffleButton.setButtonText("Shuffle");
    shuffleButton.setClickingTogglesState(true);
    shuffleAttachment.reset(new ButtonAttachment(params, "shuffle", shuffleButton));
    
    addAndMakeVisible(loopButton);
    loopButton.setButtonText("Loop");
    loopButton.setClickingTogglesState(true);
    loopAttachment.reset(new ButtonAttachment(params, "loop", loopButton));
    
    addAndMakeVisible(openButton);
    openButton.setButtonText("Choose files...");
    openButton.onClick = [this] { openButtonClicked(); };
    
    addAndMakeVisible(clearButton);
    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] { clearButtonClicked(); };
    
    addAndMakeVisible(filesList);
    filesList.setModel(this);
    filesList.setClickingTogglesRowSelection(false);
    
    //addAndMakeVisible(waveformDisplay);
    
    addAndMakeVisible(pitchSlider);
    pitchSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    pitchSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    pitchAttachment.reset(new SliderAttachment(params, "pitch", pitchSlider));
    
    addAndMakeVisible(pitchLabel);
    pitchLabel.setFont({ 11.0f });
    pitchLabel.setJustificationType(juce::Justification::centred);
    pitchLabel.setText("PITCH", juce::NotificationType::dontSendNotification);

    addAndMakeVisible(levelSlider);
    levelSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    levelSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    levelAttachment.reset(new SliderAttachment(params, "level", levelSlider));
    
    addAndMakeVisible(levelLabel);
    levelLabel.setFont({ 11.0f });
    levelLabel.setJustificationType(juce::Justification::centred);
    levelLabel.setText("LEVEL", juce::NotificationType::dontSendNotification);
    
    setSize(300, 200);
}

SamplerEditor::~SamplerEditor()
{
    samplerProcessor.removeChangeListener(this);
    loadedFiles.clear();
}

#pragma mark -

void SamplerEditor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    playStopButton.setToggleState(!samplerProcessor.isSuspended(), juce::NotificationType::dontSendNotification);
    
    if (!samplerProcessor.isSuspended() && samplerProcessor.getCurrentSampleIndex() > -1)
        filesList.selectRow(samplerProcessor.getCurrentSampleIndex());
    else
        filesList.deselectAllRows();
}

void SamplerEditor::resized()
{
    auto area = getLocalBounds();
    auto buttons = area.removeFromTop(40);
    auto buttonWidth = area.getWidth() / 6;
    
    bypassToggle.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    playStopButton.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    shuffleButton.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    loopButton.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    openButton.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    clearButton.setBounds(buttons.reduced(10));
    
    auto level = area.removeFromRight(40);
    levelLabel.setBounds(level.removeFromBottom(20));
    levelSlider.setBounds(level);
    
    auto pitch = area.removeFromRight(40);
    pitchLabel.setBounds(pitch.removeFromBottom(20));
    pitchSlider.setBounds(pitch);
    
    filesList.setBounds(area);
    
    //waveformDisplay.setBounds(area.reduced(10));

}

void SamplerEditor::paint(juce::Graphics &g)
{
    juce::ignoreUnused(g);
}

#pragma mark -

int SamplerEditor::getNumRows()
{
    return loadedFiles.size();
}

void SamplerEditor::paintListBoxItem (int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);

    g.setColour(juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::Label::textColourId));
    g.setFont((float) height * 0.7f);
    g.drawText(loadedFiles[rowNumber].getFileName(), 5, 0, width, height, juce::Justification::centredLeft, true);
}

#pragma mark -

void SamplerEditor::playStopButtonClicked()
{
    if (loadedFiles.isEmpty())
    {
        playStopButton.setToggleState(false, juce::NotificationType::dontSendNotification);
        return;
    }
    
    samplerProcessor.suspendProcessing(!playStopButton.getToggleState());
}

void SamplerEditor::openButtonClicked()
{
    fileChooser = std::make_unique<juce::FileChooser>("Choose your samples...");
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems;
    
    fileChooser->launchAsync(chooserFlags, [this] (const juce::FileChooser &fc)
    {
        auto files = fc.getResults();
        
        if (files.isEmpty())
            return;
        
        loadedFiles = files;
        samplerProcessor.readFiles(loadedFiles);
        filesList.updateContent();
        //waveformDisplay.setWaveformPeaks(samplerProcessor.getWaveformPeaks());
    });
}

void SamplerEditor::clearButtonClicked()
{
    samplerProcessor.reset();
    loadedFiles.clear();
    filesList.updateContent();
}

//void WaveformDisplayComponent::paint(juce::Graphics& g)
//{
//    g.fillAll(juce::Colours::black);  // Background color
//    g.setColour(juce::Colours::white);  // Waveform color
//
//    auto area = getLocalBounds();
//    float x = area.getX();
//    float width = area.getWidth() / static_cast<float>(waveformPeaks.size() / 2);
//
//    for (size_t i = 0; i < waveformPeaks.size(); i += 2)
//    {
//        float top = juce::jmap(waveformPeaks[i], -1.0f, 1.0f, area.getBottom(), area.getY());
//        float bottom = juce::jmap(waveformPeaks[i+1], -1.0f, 1.0f, area.getBottom(), area.getY());
//        g.drawVerticalLine(static_cast<int>(x), top, bottom);
//        x += width;
//    }
//}
//
//void WaveformDisplayComponent::setWaveformPeaks(const std::vector<std::vector<float>>& peaks)
//{
//    // For simplicity, assuming you just want to visualize one file's peaks
//    if (!peaks.empty())
//        waveformPeaks = peaks[0];
//    repaint();
//}
