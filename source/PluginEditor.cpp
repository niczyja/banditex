#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    addAndMakeVisible (inspectButton);
    addAndMakeVisible (mLoadButton);

    // Initialize file label
    addAndMakeVisible(fileLabel);
    fileLabel.setText("No file loaded", juce::NotificationType::dontSendNotification);
    fileLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // File loading button behaviour
    mLoadButton.onClick = [&]() {
        p.loadFiles();
    };
    addAndMakeVisible(mLoadButton);
    
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

void PluginEditor::resized()
{
    // Layout the positions of your child components here
    auto area = getLocalBounds();
    area.removeFromBottom(50);
    inspectButton.setBounds (getLocalBounds().withSizeKeepingCentre(100, 50));
    mLoadButton.setBounds(getLocalBounds().withSize(100, 50));

    // Set bounds for file label
    fileLabel.setBounds(area.removeFromTop(20));
}


PluginEditor::~PluginEditor()
{
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
