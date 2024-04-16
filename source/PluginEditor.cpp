#include "PluginEditor.h"

PluginEditor::PluginEditor (TestPlaygroundProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    
    processorRef.addChangeListener(this); // Register as listener

    addAndMakeVisible (inspectButton);
    addAndMakeVisible (mLoadButton);

    // File loading button behaviour
    mLoadButton.onClick = [&]() {
        p.loadFiles();
    };
    addAndMakeVisible(mLoadButton);
    
    // Initialize loadedFilesLabel
    addAndMakeVisible(loadedFilesLabel);
    loadedFilesLabel.setText("Loaded Files:", juce::dontSendNotification);
    
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}


void PluginEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &processorRef)
    {
        // When the processor notifies a change, update the list to highlight the new index
        updateLoadedFilesList();
    }
}

void PluginEditor::updateLoadedFilesList()
{
    // Get loaded file names from the processor
    auto loadedFilesNames = processorRef.getLoadedFilesNames();
    
    // Get the index of the currently playing file
    const auto& indexToHighlight = processorRef.getCurrentlyPlayingFileIndex();
    
    int loadedFilesSize = loadedFilesNames.size();

    // Construct a string to display all loaded file names
    juce::String loadedFilesText;
    
    for (int i = 0; i < loadedFilesSize; ++i)
    {
        if (i == indexToHighlight) {
            // Highlight the currently playing file by prepending "> "
            loadedFilesText += "> ";
        } else {
            // Add some spacing for alignment if not the currently playing file
            loadedFilesText += "   ";
        }
        const auto& fileName = loadedFilesNames[i];
        loadedFilesText += fileName + "\n";
    }

    // Update the label text to display loaded files
    loadedFilesLabel.setText(loadedFilesText, juce::dontSendNotification);
}





void PluginEditor::resized()
{
    // Layout the positions of your child components here
    auto area = getLocalBounds();
    auto buttonHeight = 40;
    auto margin = 10;
    
    //inspectButton.setBounds (getLocalBounds().withSizeKeepingCentre(100, 50));
    mLoadButton.setBounds(area.removeFromTop(buttonHeight).reduced(margin));
    loadedFilesLabel.setBounds(area.reduced(margin).withTrimmedTop(margin));
    
    // Update the loaded files list
    updateLoadedFilesList();
    
    

}


PluginEditor::~PluginEditor()
{
    processorRef.removeChangeListener(this); // Unregister as listener when destroyed
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}
