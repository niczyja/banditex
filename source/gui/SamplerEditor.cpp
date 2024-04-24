
#include "SamplerEditor.h"


SamplerEditor::SamplerEditor(SamplerProcessor& p)
    : AudioProcessorEditor(&p), samplerProcessor(p)
{
    samplerProcessor.addChangeListener(this);
    
    addAndMakeVisible(playStopButton);
    playStopButton.setButtonText("Play");
    playStopButton.setClickingTogglesState(true);
    playStopButton.onClick = [this] { playStopButtonClicked(); };
    
    addAndMakeVisible(shuffleButton);
    shuffleButton.setButtonText("Shuffle");
    shuffleButton.setClickingTogglesState(true);
    shuffleButton.onClick = [this] { shuffleButtonClicked(); };
    
    addAndMakeVisible(loopButton);
    loopButton.setButtonText("Loop");
    loopButton.setClickingTogglesState(true);
    loopButton.onClick = [this] { loopButtonClicked(); };
    
    addAndMakeVisible(openButton);
    openButton.setButtonText("Choose files...");
    openButton.onClick = [this] { openButtonClicked(); };
    
    addAndMakeVisible(clearButton);
    clearButton.setButtonText("Clear");
    clearButton.onClick = [this] { clearButtonClicked(); };
    
    addAndMakeVisible(filesList);
    filesList.setModel(this);
    filesList.setClickingTogglesRowSelection(false);
    
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
    shuffleButton.setToggleState(samplerProcessor.getIsShuffling(), juce::NotificationType::dontSendNotification);
    loopButton.setToggleState(samplerProcessor.isLooping, juce::NotificationType::dontSendNotification);
    
    if (!samplerProcessor.isSuspended() && samplerProcessor.getCurrentSampleIndex() > -1)
        filesList.selectRow(samplerProcessor.getCurrentSampleIndex());
    else
        filesList.deselectAllRows();
}

void SamplerEditor::resized()
{
    auto area = getLocalBounds();
    auto buttons = area.removeFromTop(40);
    auto buttonWidth = area.getWidth() / 5;
    
    playStopButton.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    shuffleButton.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    loopButton.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    openButton.setBounds(buttons.removeFromLeft(buttonWidth).reduced(10));
    clearButton.setBounds(buttons.reduced(10));
    
    filesList.setBounds(area);
}

void SamplerEditor::paint(juce::Graphics &g)
{
    g.fillAll(juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::ListBox::backgroundColourId));
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

void SamplerEditor::shuffleButtonClicked()
{
    samplerProcessor.setIsShuffling(shuffleButton.getToggleState());
}

void SamplerEditor::loopButtonClicked()
{
    samplerProcessor.isLooping = loopButton.getToggleState();
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
    });
}

void SamplerEditor::clearButtonClicked()
{
    samplerProcessor.reset();
    loadedFiles.clear();
    filesList.updateContent();
}
